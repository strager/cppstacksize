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

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset) {
    let buffer = this.#buffer;
    let size = buffer.length;
    let i = offset;
    for (;;) {
      if (i >= size) {
        return null;
      }
      if (buffer.readUInt8(i) === b) {
        return i;
      }
      i += 1;
    }
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
  #uint8Array;

  constructor(arrayBuffer) {
    this.#dataView = new DataView(arrayBuffer);
    this.#uint8Array = new Uint8Array(arrayBuffer);
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

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset) {
    let i = this.#uint8Array.indexOf(b, offset);
    if (i === -1) {
      return null;
    }
    return i;
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

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset) {
    // TODO(strager): Bounds check.
    let i = this.baseReader.findU8(b, offset + this.subFileOffset);
    if (i === null) {
      return null;
    }
    return i - this.subFileOffset;
  }
}
