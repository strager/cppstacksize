import { withLoadScopeAsync } from "./loader.mjs";

export class PDBParser {
  /// Call parseStreamDirectoryAsync to populate streams.
  streams = null;

  #reader;

  /// Call parseHeaderAsync to populate the following variables:
  #blockSize = null;
  #blockCount = null;
  #directorySize = null; // In bytes.
  #directoryMapBlock = null; // Block index.

  constructor(reader) {
    this.#reader = reader;
  }

  /// Parses the superblock.
  async parseHeaderAsync() {
    return await withLoadScopeAsync(() => {
      // TODO(strager): Validate the PDB signature.
      this.#blockSize = this.#reader.u32(0x20);
      this.#blockCount = this.#reader.u32(0x28);
      this.#directorySize = this.#reader.u32(0x2c);
      this.#directoryMapBlock = this.#reader.u32(0x34);
    });
  }

  async parseStreamDirectoryAsync() {
    return await withLoadScopeAsync(() => {
      let directoryBlockCount = Math.ceil(
        this.#directorySize / this.#blockSize
      );
      let directoryBlocks = [];
      for (let i = 0; i < directoryBlockCount; ++i) {
        directoryBlocks.push(
          this.#reader.u32(this.#directoryMapBlock * this.#blockSize + i * 4)
        );
      }
      let directoryReader = new PDBBlocksReader(
        this.#reader,
        directoryBlocks,
        this.#blockSize,
        this.#directorySize
      );

      let streams = [];
      let offset = 0;
      let streamCount = directoryReader.u32(offset);
      offset += 4;
      let streamSizes = [];
      for (let streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
        streamSizes.push(directoryReader.u32(offset));
        offset += 4;
      }
      for (let streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
        let streamSize = streamSizes[streamIndex];
        let blockCount = Math.ceil(streamSize / this.#blockSize);
        if (streamSize === 2 ** 32 - 1) {
          // HACK(strager): Sometimes we see a stream with size 4294967295. This
          // might be legit, but I suspect not. Pretend the size is 0 instead.
          blockCount = 0;
        }

        let streamBlocks = [];
        for (let i = 0; i < blockCount; ++i) {
          streamBlocks.push(directoryReader.u32(offset));
          offset += 4;
        }
        streams.push(
          new PDBBlocksReader(
            this.#reader,
            streamBlocks,
            this.#blockSize,
            streamSize
          )
        );
      }
      this.streams = streams;
    });
  }
}

class PDBBlocksReader {
  #baseReader;
  #blockIndexes;
  #blockSize;
  #byteSize;

  constructor(baseReader, blockIndexes, blockSize, byteSize) {
    this.#baseReader = baseReader;
    this.#blockIndexes = blockIndexes;
    this.#blockSize = blockSize;
    this.#byteSize = byteSize;
  }

  get blockIndexes() {
    return this.#blockIndexes;
  }

  get size() {
    return this.#byteSize;
  }

  u16(offset) {
    let size = 2;
    if (offset + size >= this.#byteSize) {
      // TODO(strager): Report out of range error.
    }

    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    let blockIndexIndex = Math.floor(offset / this.#blockSize);
    let blockIndex = this.#blockIndexes[blockIndexIndex];
    let endBlockIndexIndex = Math.floor((offset + size - 1) / this.#blockSize);
    let data;
    if (blockIndexIndex === endBlockIndexIndex) {
      let relativeOffset = offset % this.#blockSize;
      return this.#baseReader.u16(
        blockIndex * this.#blockSize + relativeOffset
      );
    } else {
      throw new Error(
        `not yet implemented: reading u16 straddling multiple blocks`
      );
      //let data = this.#readCopySlow(offset, size);
      //return data.getUint16(0, /*littleEndian=*/ true);
    }
  }

  u32(offset) {
    let size = 4;
    if (offset + size >= this.#byteSize) {
      // TODO(strager): Report out of range error.
    }

    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    let blockIndexIndex = Math.floor(offset / this.#blockSize);
    let blockIndex = this.#blockIndexes[blockIndexIndex];
    let endBlockIndexIndex = Math.floor((offset + size - 1) / this.#blockSize);
    let data;
    if (blockIndexIndex === endBlockIndexIndex) {
      let relativeOffset = offset % this.#blockSize;
      return this.#baseReader.u32(
        blockIndex * this.#blockSize + relativeOffset
      );
    } else {
      throw new Error(
        `not yet implemented: reading u32 straddling multiple blocks`
      );
      //let data = this.#readCopySlow(offset, size);
      //return data.getUint32(0, /*littleEndian=*/ true);
    }
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset, endOffset = null) {
    if (endOffset === null || endOffset > this.#byteSize) {
      endOffset = this.#byteSize;
    }
    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    let beginBlockIndexIndex = Math.floor(offset / this.#blockSize);
    let endBlockIndexIndex = Math.floor((endOffset - 1) / this.#blockSize);
    let relativeOffset = offset % this.#blockSize;

    for (
      let blockIndexIndex = beginBlockIndexIndex;
      blockIndexIndex <= endBlockIndexIndex;
      ++blockIndexIndex
    ) {
      // TODO(strager): Bounds check.
      let blockIndex = this.#blockIndexes[blockIndexIndex];

      // TODO(strager): Properly bounds check.
      let i = this.#baseReader.findU8(
        b,
        blockIndex * this.#blockSize + relativeOffset,
        (blockIndex + 1) * this.#blockSize
      );
      if (i !== null) {
        return (i % this.#blockSize) + blockIndexIndex * this.#blockSize;
      }
      relativeOffset = 0;
    }
    return null;
  }

  utf8CString(offset) {
    let endOffset = this.findU8(0, offset);

    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    let beginBlockIndexIndex = Math.floor(offset / this.#blockSize);
    let endBlockIndexIndex = Math.floor(endOffset / this.#blockSize);
    let relativeOffset = offset % this.#blockSize;
    let decoder = new TextDecoder("utf-8");
    let result = "";

    let buffer = new Uint8Array(this.#blockSize);
    for (
      let blockIndexIndex = beginBlockIndexIndex;
      blockIndexIndex <= endBlockIndexIndex;
      ++blockIndexIndex
    ) {
      let blockIndex = this.#blockIndexes[blockIndexIndex];

      let isLastBlock = blockIndexIndex === endBlockIndexIndex;
      let sizeNeededInBlock =
        (isLastBlock ? endOffset & (this.#blockSize - 1) : this.#blockSize) -
        relativeOffset;

      for (
        let i = relativeOffset;
        i < relativeOffset + sizeNeededInBlock;
        ++i
      ) {
        // FIXME(strager): This is inefficient and silly. At the time of writing,
        // our Reader abstractions are too weak to make this elegant.
        let word = this.#baseReader.u16(blockIndex * this.#blockSize + i);
        buffer[i] = word & 0xff;
      }

      let data = new Uint8Array(
        buffer.buffer,
        relativeOffset,
        sizeNeededInBlock
      );
      if (isLastBlock) {
        result += decoder.decode(data /*options={stream: false}*/);
        return result;
      } else {
        result += decoder.decode(data, { stream: true });
        relativeOffset = 0;
      }
    }
  }
}
