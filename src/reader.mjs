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
    let buffer = this.#buffer;
    let endOffset = offset + size;
    while (endOffset > offset && buffer.readUInt8(endOffset - 1) === 0) {
      endOffset -= 1;
    }
    return buffer.toString("latin1", offset, endOffset);
  }

  utf8CString(offset) {
    let buffer = this.#buffer;
    let endOffset = offset;
    while (buffer.readUInt8(endOffset) !== 0) {
      endOffset += 1;
    }
    return this.#buffer.toString("utf-8", offset, endOffset);
  }
}

export class ArrayBufferReader {
  #dataView;

  constructor(arrayBuffer) {
    this.#dataView = new DataView(arrayBuffer);
  }

  u16(offset) {
    return this.#dataView.getUint16(offset, /*littleEndian=*/ true);
  }

  u32(offset) {
    return this.#dataView.getUint32(offset, /*littleEndian=*/ true);
  }

  fixedWidthString(offset, size) {
    let dataView = this.#dataView;
    let endOffset = offset + size;
    while (endOffset > offset && dataView.getUint8(endOffset - 1) === 0) {
      endOffset -= 1;
    }
    return new TextDecoder("latin1").decode(
      new Uint8Array(this.#dataView.buffer, offset, endOffset - offset)
    );
  }

  utf8CString(offset) {
    let buffer = this.#dataView;
    let endOffset = offset;
    while (buffer.getUint8(endOffset) !== 0) {
      endOffset += 1;
    }
    return new TextDecoder("utf-8").decode(
      new Uint8Array(this.#dataView.buffer, offset, endOffset - offset)
    );
  }
}

export class SubFileReader {
  baseReader;
  subFileOffset;
  subFileSize;

  constructor(baseReader, offset, size) {
    this.baseReader = baseReader;
    this.subFileOffset = offset;
    this.subFileSize = size;
  }

  get size() {
    return this.subFileSize;
  }

  u16(offset) {
    // TODO(strager): Bounds check.
    return this.baseReader.u16(offset + this.subFileOffset);
  }

  u32(offset) {
    // TODO(strager): Bounds check.
    return this.baseReader.u32(offset + this.subFileOffset);
  }

  utf8CString(offset) {
    // TODO(strager): Bounds check.
    return this.baseReader.utf8CString(offset + this.subFileOffset);
  }

  fixedWidthString(offset, size) {
    // TODO(strager): Bounds check.
    return this.baseReader.fixedWidthString(offset + this.subFileOffset, size);
  }
}
