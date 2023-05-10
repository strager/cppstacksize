export class NodeBufferReader {
  #buffer;

  constructor(buffer) {
    this.#buffer = buffer;
  }

  u16(offset) {
    return this.#buffer.readUInt16LE(offset);
  }

  u32(offset) {
    return this.#buffer.readUInt32LE(offset);
  }

  fixedWidthString(offset, size) {
    let bytes = this.#buffer.subarray(offset, offset + size);
    let length = bytes.length;
    while (length > 0) {
      if (bytes.readUInt8(length - 1) !== 0) {
        break;
      }
      length -= 1;
    }
    return bytes.toString("latin1", 0, length);
  }
}

export class SubFileReader {
  baseFile;
  subFileOffset;
  subFileSize;

  constructor(baseFile, offset, size) {
    this.baseFile = baseFile;
    this.subFileOffset = offset;
    this.subFileSize = size;
  }
}
