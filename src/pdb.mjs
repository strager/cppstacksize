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
}
