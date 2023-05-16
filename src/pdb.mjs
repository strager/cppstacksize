import { ReaderBase, SubFileReader } from "./reader.mjs";
import { alignUp } from "./util.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

/// The header of a PDB file.
export class PDBSuperBlock {
  blockSize;
  blockCount;
  directorySize; // In bytes.
  directoryMapBlock; // Block index.
}

export class PDBParser {
  /// Call parseStreamDirectoryAsync to populate streams.
  streams = null;

  #reader;

  /// Call parseHeaderAsync to populate the following variables:
  #superBlock = null;

  constructor(reader) {
    this.#reader = reader;
  }

  /// Parses the superblock.
  async parseHeaderAsync() {
    return await withLoadScopeAsync(() => {
      // TODO(strager): Validate the PDB signature.
      let superBlock = new PDBSuperBlock();
      superBlock.blockSize = this.#reader.u32(0x20);
      superBlock.blockCount = this.#reader.u32(0x28);
      superBlock.directorySize = this.#reader.u32(0x2c);
      superBlock.directoryMapBlock = this.#reader.u32(0x34);
      this.#superBlock = superBlock;
    });
  }

  async parseStreamDirectoryAsync() {
    return await withLoadScopeAsync(() => {
      let directoryBlockCount = Math.ceil(
        this.#superBlock.directorySize / this.#superBlock.blockSize
      );
      let directoryBlocks = [];
      for (let i = 0; i < directoryBlockCount; ++i) {
        directoryBlocks.push(
          this.#reader.u32(
            this.#superBlock.directoryMapBlock * this.#superBlock.blockSize +
              i * 4
          )
        );
      }
      let directoryReader = new PDBBlocksReader(
        this.#reader,
        directoryBlocks,
        this.#superBlock.blockSize,
        this.#superBlock.directorySize
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
        let blockCount = Math.ceil(streamSize / this.#superBlock.blockSize);
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
            this.#superBlock.blockSize,
            streamSize
          )
        );
      }
      this.streams = streams;
    });
  }
}

export async function parsePDBDBIStreamAsync(reader) {
  return withLoadScopeAsync(() => {
    let moduleInfoSize = reader.u32(0x18);

    let modules = [];
    let moduleInfosBegin = 0x40;
    let moduleInfosReader = new SubFileReader(
      reader,
      moduleInfosBegin,
      moduleInfoSize
    );
    let offset = 0;
    while (offset < moduleInfosReader.size) {
      offset = alignUp(offset, 4);
      let moduleSymStream = moduleInfosReader.u16(offset + 0x22);
      let moduleNameNullTerminatorOffset = moduleInfosReader.findU8(
        0,
        offset + 0x40
      );
      if (moduleNameNullTerminatorOffset === null) {
        console.error("incomplete module info entry");
        break;
      }
      let moduleName = moduleInfosReader.utf8String(
        offset + 0x40,
        moduleNameNullTerminatorOffset - (offset + 0x40)
      );
      offset = moduleNameNullTerminatorOffset + 1;
      let objNameNullTerminatorOffset = moduleInfosReader.findU8(0, offset);
      if (objNameNullTerminatorOffset === null) {
        console.error("incomplete module info entry");
        break;
      }
      let objName = moduleInfosReader.utf8String(
        offset,
        objNameNullTerminatorOffset - offset
      );
      offset = objNameNullTerminatorOffset + 1;
      modules.push({
        linkedObjectPath: objName,
        sourceObjectPath: moduleName,
        debugInfoStreamIndex: moduleSymStream,
      });
    }
    return {
      modules: modules,
    };
  });
}

export class PDBBlocksReader extends ReaderBase {
  #baseReader;
  #blockIndexes;
  #blockSize;
  #byteSize;

  constructor(baseReader, blockIndexes, blockSize, byteSize) {
    super();
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

  utf8String(offset, length) {
    let endOffset = offset + length;
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
