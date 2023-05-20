import { Location, ReaderBase } from "./reader.mjs";

/// A simple Loader implementation which reads data from an array. Used for
/// testing.
export class ArrayLoader {
  #data;

  constructor(bytes) {
    this.#data = new Uint8Array(bytes).buffer;
  }

  get size() {
    return this.#data.byteLength;
  }

  async readAsync(offset, size) {
    size = Math.min(size, this.#data.byteLength - offset);
    return new Uint8Array(this.#data, offset, size);
  }
}

/// A Loader implementation which reads data from a web Blob, such as a file
/// input upload.
export class BlobLoader {
  #blob;

  constructor(blob) {
    this.#blob = blob;
  }

  get size() {
    return this.#blob.size;
  }

  async readAsync(offset, size) {
    let arrayBuffer = await this.#blob
      .slice(offset, offset + size)
      .arrayBuffer();
    return new Uint8Array(arrayBuffer);
  }
}

/// A Reader which caches data from a Loader.
///
/// TODO(strager): Implement a cache eviction strategy.
export class LoaderReader extends ReaderBase {
  /// Array<Promise<DataView> | DataView | undefined>
  ///
  /// If the slot is a Promise, then either the data failed to load or the data
  /// is being loaded.
  #chunks = [];

  #loader;
  #chunkSize;
  #chunkShift;
  #chunkCount;

  constructor(loader, { chunkSize } = { chunkSize: 1 << 16 }) {
    super();
    // TODO(strager): Assert that chunkSize is a power of two.
    this.#loader = loader;
    this.#chunkSize = chunkSize;
    this.#chunkShift = 32 - Math.clz32(chunkSize) - 1;
    this.#chunkCount = (loader.size + chunkSize - 1) >> this.#chunkShift;
  }

  async fetchAsync(offset, size) {
    console.assert(size > 0);
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
        console.assert(chunksData instanceof Uint8Array);
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

  locate(offset) {
    return new Location(offset, null, null);
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

  enumerateBytes(offset, size, callback) {
    let endOffset = offset + size;
    let beginChunkIndex = offset >> this.#chunkShift;
    let endChunkIndex = (endOffset - 1) >> this.#chunkShift;
    let relativeOffset = offset & (this.#chunkSize - 1);
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
      let chunk = this.#chunks[chunkIndex];
      let isLastChunk = chunkIndex === endChunkIndex;
      let sizeNeededInChunk;
      if (isLastChunk) {
        sizeNeededInChunk = endOffset & (this.#chunkSize - 1);
        if (sizeNeededInChunk === 0) {
          sizeNeededInChunk = this.#chunkSize;
        }
        sizeNeededInChunk -= relativeOffset;
      } else {
        sizeNeededInChunk = this.#chunkSize - relativeOffset;
      }
      this.#requireChunkLoaded(
        chunk,
        (chunkIndex << this.#chunkShift) | relativeOffset,
        sizeNeededInChunk
      );

      callback(
        new Uint8Array(
          chunk.buffer,
          chunk.byteOffset + relativeOffset,
          sizeNeededInChunk
        )
      );
      relativeOffset = 0;
    }
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset, endOffset = null) {
    if (endOffset === null || endOffset > this.#loader.size) {
      endOffset = this.#loader.size;
    }
    let beginChunkIndex = offset >> this.#chunkShift;
    let endChunkIndex = (endOffset - 1) >> this.#chunkShift;
    let relativeOffset = offset & (this.#chunkSize - 1);
    for (
      let chunkIndex = beginChunkIndex;
      chunkIndex <= endChunkIndex;
      ++chunkIndex
    ) {
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
      let i = data.indexOf(b);
      if (i !== -1) {
        let foundOffset = (chunkIndex << this.#chunkShift) + relativeOffset + i;
        if (foundOffset >= endOffset) {
          return null;
        }
        return foundOffset;
      }
      relativeOffset = 0;
    }
    return null;
  }

  #requireChunkLoaded(chunk, offset, size) {
    if (chunk === undefined || isPromise(chunk)) {
      if (offset + size > this.#loader.size) {
        throw new RangeError(
          `cannot read out of bounds; offset=0x${offset.toString(
            16
          )} size=0x${size.toString(16)}`
        );
      }
      throw new DataNotLoadedError(this, offset, size);
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
      this.#requireChunkLoaded(this.#chunks[chunkIndex], offset, size);
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
  loaderReader;
  offset;
  size;

  constructor(loaderReader, offset, size) {
    super("data not loaded");
    this.loaderReader = loaderReader;
    this.offset = offset;
    this.size = size;
  }
}

function isPromise(v) {
  return v instanceof Promise;
}

export async function withLoadScopeAsync(scopeFunction) {
  for (;;) {
    try {
      return await scopeFunction();
    } catch (e) {
      if (e instanceof DataNotLoadedError) {
        await e.loaderReader.fetchAsync(e.offset, e.size);
        continue;
      } else {
        throw e;
      }
    }
  }
}
