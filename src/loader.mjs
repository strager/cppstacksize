import assert from "node:assert";

/// A simple Loader implementation which reads data from an array. Used for
/// testing.
export class ArrayLoader {
  #data;

  constructor(bytes) {
    this.#data = new Uint8Array(bytes).buffer;
  }

  async readAsync(offset, size) {
    size = Math.min(size, this.#data.byteLength - offset);
    return new Uint8Array(this.#data, offset, size);
  }
}

/// A Reader which caches data from a Loader.
///
/// TODO(strager): Implement a cache eviction strategy.
export class LoaderReader {
  /// Array<Promise<DataView> | DataView | undefined>
  ///
  /// If the slot is a Promise, then either the data failed to load or the data
  /// is being loaded.
  #chunks = [];

  #loader;
  #chunkSize;
  #chunkShift;

  constructor(loader, { chunkSize } = { chunkSize: 1 << 16 }) {
    // TODO(strager): Assert that chunkSize is a power of two.
    this.#loader = loader;
    this.#chunkSize = chunkSize;
    this.#chunkShift = 32 - Math.clz32(chunkSize) - 1;
  }

  async fetchAsync(offset, size) {
    assert.ok(size > 0);
    let beginChunkIndex = offset >> this.#chunkShift;
    let endChunkIndex = (offset + size - 1) >> this.#chunkShift;
    if (this.#allChunksAreLoaded(beginChunkIndex, endChunkIndex)) {
      return;
    }
    let chunkCount = endChunkIndex - beginChunkIndex + 1;

    // TODO(strager): If a chunk in the middle was already loaded, don't re-load
    // it.
    let chunksLoadPromise = this.#loader.readAsync(
      beginChunkIndex << this.#chunkShift,
      chunkCount << this.#chunkShift
    );
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
      this.#chunks[chunkIndex] = chunksLoadPromise.then((chunksData) => {
        assert.ok(chunksData instanceof Uint8Array);
        let chunkOffset =
          chunksData.byteOffset +
          ((chunkIndex - beginChunkIndex) << this.#chunkShift);
        let chunk = new DataView(
          chunksData.buffer,
          chunkOffset,
          Math.min(
            this.#chunkSize,
            chunksData.byteLength + chunksData.byteOffset - chunkOffset
          )
        );
        this.#chunks[chunkIndex] = chunk;
        return chunk;
      });
    }
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
      await this.#chunks[chunkIndex];
    }
  }

  #allChunksAreLoaded(beginChunkIndex, endChunkIndex) {
    for (let i = beginChunkIndex; i <= endChunkIndex; ++i) {
      if (this.#chunks[i] === undefined) {
        return false;
      }
    }
    return true;
  }

  u16(offset) {
    let size = 2;
    let relativeOffset = offset & (this.#chunkSize - 1);
    let chunkIndex = offset >> this.#chunkShift;
    let data = this.#chunks[chunkIndex];
    let endChunkIndex = (offset + size - 1) >> this.#chunkShift;
    this.#requireChunkLoaded(data, offset, size);
    if (chunkIndex !== endChunkIndex) {
      data = this.#readCopySlow(offset, size);
      relativeOffset = 0;
    }
    return data.getUint16(relativeOffset, /*littleEndian=*/ true);
  }

  u32(offset) {
    let size = 4;
    let relativeOffset = offset & (this.#chunkSize - 1);
    let chunkIndex = offset >> this.#chunkShift;
    let data = this.#chunks[chunkIndex];
    let endChunkIndex = (offset + size - 1) >> this.#chunkShift;
    this.#requireChunkLoaded(data, offset, size);
    if (chunkIndex !== endChunkIndex) {
      data = this.#readCopySlow(offset, size);
      relativeOffset = 0;
    }
    return data.getUint32(relativeOffset, /*littleEndian=*/ true);
  }

  fixedWidthString(offset, size) {
    // TODO(strager): Make this more efficient.
    let data = this.#readCopySlow(offset, size);
    let length = size;
    while (length > offset && data.getUint8(length - 1) === 0) {
      length -= 1;
    }
    return new TextDecoder("latin1").decode(
      new Uint8Array(data.buffer, data.byteOffset, length)
    );
  }

  utf8CString(offset) {
    let beginChunkIndex = offset >> this.#chunkShift;
    let relativeOffset = offset & (this.#chunkSize - 1);
    let decoder = new TextDecoder("utf-8");
    let result = "";
    for (let chunkIndex = beginChunkIndex; ; ++chunkIndex) {
      let chunk = this.#chunks[chunkIndex];
      this.#requireChunkLoaded(
        chunk,
        (chunkIndex << this.#chunkShift) | relativeOffset,
        1
      );

      let data = new Uint8Array(
        chunk.buffer,
        chunk.byteOffset + relativeOffset,
        chunk.byteLength - relativeOffset
      );

      let nullTerminatorIndex = data.indexOf(0);
      if (nullTerminatorIndex === -1) {
        result += decoder.decode(data, { stream: true });
      } else {
        result += decoder.decode(
          new Uint8Array(data.buffer, data.byteOffset, nullTerminatorIndex)
          /*options={stream: false}*/
        );
        return result;
      }
      relativeOffset = 0;
    }
  }

  #requireChunkLoaded(chunk, offset, size) {
    if (chunk === undefined || isPromise(chunk)) {
      throw new DataNotLoadedError(this.#loader, offset, size);
    }
  }

  /// Return a copy of data from offset to offset+size. Supports cross-chunk
  /// reads.
  ///
  /// @return DataView
  #readCopySlow(offset, size) {
    let beginChunkIndex = offset >> this.#chunkShift;
    let endChunkIndex = (offset + size - 1) >> this.#chunkShift;
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
      let chunk = this.#chunks[chunkIndex];
      if (chunk === undefined || isPromise(chunk)) {
        throw new DataNotLoadedError(this.#loader, offset, size);
      }
    }
    let endOffset = offset + size;
    let data = new Uint8Array(size);
    let outOffset = 0;
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
      let chunkBeginOffset = chunkIndex << this.#chunkShift;
      let inBeginOffset =
        chunkIndex === beginChunkIndex ? offset - chunkBeginOffset : 0;
      let inEndOffset =
        chunkIndex === endChunkIndex
          ? endOffset - chunkBeginOffset
          : this.#chunkSize;
      let inLength = inEndOffset - inBeginOffset;
      let chunk = this.#chunks[chunkIndex];
      data.set(
        new Uint8Array(
          chunk.buffer,
          chunk.byteOffset + inBeginOffset,
          inLength
        ),
        outOffset
      );
      outOffset += inLength;
    }
    return new DataView(data.buffer, data.byteOffset, data.byteLength);
  }

  debugDump() {
    let table = [];
    for (let chunkIndex in this.#chunks) {
      let chunk = this.#chunks[chunkIndex];
      if (chunk === undefined) {
        continue;
      }
      let data = isPromise(chunk)
        ? "(pending)"
        : new Uint8Array(chunk.buffer, chunk.byteOffset, chunk.byteLength);
      table.push({ chunk: +chunkIndex, data: data });
    }
    console.log("LoaderReader:");
    console.table(table, ["chunk", "data"]);
  }
}

export class DataNotLoadedError extends Error {
  loader;
  offset;
  size;

  constructor(loader, offset, size) {
    super("data not loaded");
    this.loader = loader;
    this.offset = offset;
    this.size = size;
  }
}

function isPromise(v) {
  return v instanceof Promise;
}
