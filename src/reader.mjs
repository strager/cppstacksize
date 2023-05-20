export class Location {
  fileOffset;
  streamIndex;
  streamOffset;

  constructor(fileOffset, streamIndex, streamOffset) {
    this.fileOffset = fileOffset;
    this.streamIndex = streamIndex;
    this.streamOffset = streamOffset;
  }

  toString() {
    let result = `file offset 0x${this.fileOffset.toString(16)}`;
    if (this.streamIndex !== null && this.streamOffset !== null) {
      if (this.streamIndex === -1) {
        result = `stream directory offset 0x${this.streamOffset.toString(
          16
        )} (${result})`;
      } else {
        result = `stream #${
          this.streamIndex
        } offset 0x${this.streamOffset.toString(16)} (${result})`;
      }
    }
    return result;
  }
}

export class ReaderBase {
  fixedWidthString(offset, size) {
    let length = size;
    let stringEndOffset = this.findU8(0, offset, offset + size);
    if (stringEndOffset !== null) {
      length = stringEndOffset - offset;
    }
    return this.utf8String(offset, length);
  }

  utf8CString(offset) {
    let endOffset = this.findU8(0, offset);
    if (endOffset === null) {
      throw new CStringNullTerminatorNotFoundError();
    }
    return this.utf8String(offset, endOffset - offset);
  }
}

export class NodeBufferReader extends ReaderBase {
  #buffer;

  constructor(buffer) {
    super();
    this.#buffer = buffer;
  }

  get size() {
    return this.#buffer.length;
  }

  locate(offset) {
    return new Location(offset, null, null);
  }

  u16(offset) {
    return this.#buffer.readUInt16LE(offset);
  }

  u32(offset) {
    return this.#buffer.readUInt32LE(offset);
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset, endOffset = null) {
    let buffer = this.#buffer;
    if (endOffset === null || endOffset > buffer.length) {
      endOffset = buffer.length;
    }
    let i = offset;
    for (;;) {
      if (i >= endOffset) {
        return null;
      }
      if (buffer.readUInt8(i) === b) {
        return i;
      }
      i += 1;
    }
  }

  utf8String(offset, length) {
    if (offset + length > this.#buffer.length) {
      throw new RangeError(
        `cannot read out of bounds; offset=0x${offset.toString(
          16
        )} length=0x${length.toString(16)}`
      );
    }
    return this.#buffer.toString("utf-8", offset, offset + length);
  }

  copyBytesInto(out, offset, size, outOffset = 0) {
    if (offset + size > this.#buffer.length) {
      throw new RangeError(
        `cannot read out of bounds; offset=0x${offset.toString(
          16
        )} size=0x${size.toString(16)}`
      );
    }
    console.assert(out instanceof Uint8Array);
    let data = this.#buffer.subarray(offset, offset + size);
    out.set(data, outOffset);
  }
}

export class ArrayBufferReader extends ReaderBase {
  #dataView;
  #uint8Array;

  constructor(arrayBuffer) {
    super();
    this.#dataView = new DataView(arrayBuffer);
    this.#uint8Array = new Uint8Array(arrayBuffer);
  }

  locate(offset) {
    return new Location(offset, null, null);
  }

  u16(offset) {
    return this.#dataView.getUint16(offset, /*littleEndian=*/ true);
  }

  u32(offset) {
    return this.#dataView.getUint32(offset, /*littleEndian=*/ true);
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset, endOffset = null) {
    // TODO(strager): Don't read past endOffset if not null.
    let i = this.#uint8Array.indexOf(b, offset);
    if (i === -1) {
      return null;
    }
    if (endOffset !== null) {
      if (i >= endOffset) {
        return null;
      }
    }
    return i;
  }

  utf8String(offset, length) {
    return new TextDecoder("utf-8").decode(
      new Uint8Array(this.#dataView.buffer, offset, length)
    );
  }

  copyBytesInto(out, offset, size, outOffset = 0) {
    console.assert(out instanceof Uint8Array);
    let data = new Uint8Array(
      this.#dataView.buffer,
      this.#dataView.byteOffset + offset,
      size
    );
    out.set(data, outOffset);
  }
}

export class SubFileReader extends ReaderBase {
  baseReader;
  subFileOffset;
  subFileSize;

  constructor(baseReader, offset, size) {
    super();
    this.baseReader = baseReader;
    this.subFileOffset = offset;
    this.subFileSize = size;
  }

  get size() {
    return this.subFileSize;
  }

  locate(offset) {
    return this.baseReader.locate(offset + this.subFileOffset);
  }

  u16(offset) {
    this.#checkBounds(offset, 2);
    return this.baseReader.u16(offset + this.subFileOffset);
  }

  u32(offset) {
    this.#checkBounds(offset, 4);
    return this.baseReader.u32(offset + this.subFileOffset);
  }

  utf8String(offset, length) {
    this.#checkBounds(offset, length);
    return this.baseReader.utf8String(offset + this.subFileOffset, length);
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  findU8(b, offset, endOffset = null) {
    if (offset >= this.subFileSize) {
      return null;
    }
    let subFileEndOffset = this.subFileOffset + this.subFileSize;
    if (endOffset === null) {
      endOffset = subFileEndOffset;
    } else {
      endOffset = endOffset + this.subFileOffset;
      if (endOffset > subFileEndOffset) {
        endOffset = subFileEndOffset;
      }
    }

    let i = this.baseReader.findU8(b, offset + this.subFileOffset, endOffset);
    if (i === null) {
      return null;
    }
    return i - this.subFileOffset;
  }

  copyBytesInto(out, offset, size, outOffset = 0) {
    this.#checkBounds(offset, size);
    console.assert(out instanceof Uint8Array);
    this.baseReader.copyBytesInto(
      out,
      offset + this.subFileOffset,
      size,
      outOffset
    );
  }

  #checkBounds(offset, size) {
    if (offset + size > this.subFileSize) {
      throw new RangeError(
        `cannot read out of bounds; offset=0x${offset.toString(
          16
        )} size=0x${size.toString(16)}`
      );
    }
  }
}

export class CStringNullTerminatorNotFoundError extends Error {
  constructor() {
    super("could not find null terminator in C string");
  }
}
