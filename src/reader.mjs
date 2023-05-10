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

  utf8CString(offset) {
    let buffer = this.#buffer;
    let endOffset = offset;
    for (; buffer.readUInt8(endOffset) !== 0; ++endOffset) {
      // Scan.
    }
    return this.#buffer.toString("utf-8", offset, endOffset);
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

  get size() {
    return this.subFileSize;
  }

  u16(offset) {
    // TODO(strager): Bounds check.
    return this.baseFile.u16(offset + this.subFileOffset);
  }

  u32(offset) {
    // TODO(strager): Bounds check.
    return this.baseFile.u32(offset + this.subFileOffset);
  }

  utf8CString(offset) {
    // TODO(strager): Bounds check.
    return this.baseFile.utf8CString(offset + this.subFileOffset);
  }
}
