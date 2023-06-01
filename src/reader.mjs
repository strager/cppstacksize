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

  utf8String(offset, length) {
    let decoder = new TextDecoder("utf-8");
    let result = "";
    this.enumerateBytes(offset, length, (bytes) => {
      result += decoder.decode(bytes, { stream: true });
    });
    result += decoder.decode(new Uint8Array(0), { stream: false });
    return result;
  }

  copyBytesInto(out, offset, size, outOffset = 0) {
    this.enumerateBytes(offset, size, (chunk) => {
      out.set(chunk, outOffset);
      outOffset += chunk.byteLength ?? chunk.length;
    });
  }

  copyBytes(offset, size) {
    let buffer = new Uint8Array(size);
    this.copyBytesInto(buffer, offset, size);
    return buffer;
  }

  _raiseOutOfBoundsError(offset, size) {
    throw new RangeError(
      `${this.locate(
        offset
      )}: cannot read out of bounds; size=0x${size.toString(16)}`
    );
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

  u8(offset) {
    return this.#buffer.readUInt8(offset);
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
      this._raiseOutOfBoundsError(offset, length);
    }
    return this.#buffer.toString("utf-8", offset, offset + length);
  }

  enumerateBytes(offset, size, callback) {
    if (offset + size > this.#buffer.length) {
      this._raiseOutOfBoundsError(offset, size);
    }
    callback(this.#buffer.subarray(offset, offset + size));
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

  get size() {
    return this.#uint8Array.byteLength;
  }

  locate(offset) {
    return new Location(offset, null, null);
  }

  u8(offset) {
    return this.#dataView.getUint8(offset);
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

  enumerateBytes(offset, size, callback) {
    callback(
      new Uint8Array(
        this.#dataView.buffer,
        this.#dataView.byteOffset + offset,
        size
      )
    );
  }
}

export class SubFileReader extends ReaderBase {
  baseReader;
  subFileOffset;
  subFileSize;

  constructor(baseReader, offset, size = null) {
    super();
    if (size === null) {
      size = baseReader.size - offset;
    }
    if (baseReader instanceof SubFileReader) {
      // Optimization: Combine nested SubFileReader-s.
      offset += baseReader.subFileOffset;
      baseReader = baseReader.baseReader;
    }
    this.baseReader = baseReader;
    // TODO(strager): Ensure offset does not exceed baseReader.size.
    this.subFileOffset = offset;
    if (offset + size >= this.baseReader.size) {
      this.subFileSize = this.baseReader.size - offset;
    } else {
      this.subFileSize = size;
    }
  }

  get size() {
    return this.subFileSize;
  }

  locate(offset) {
    return this.baseReader.locate(offset + this.subFileOffset);
  }

  u8(offset) {
    this.#checkBounds(offset, 1);
    return this.baseReader.u8(offset + this.subFileOffset);
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

  enumerateBytes(offset, size, callback) {
    this.#checkBounds(offset, size);
    this.baseReader.enumerateBytes(offset + this.subFileOffset, size, callback);
  }

  #checkBounds(offset, size) {
    if (offset + size > this.subFileSize) {
      this._raiseOutOfBoundsError(offset, size);
    }
  }
}

export class CStringNullTerminatorNotFoundError extends Error {
  constructor() {
    super("could not find null terminator in C string");
  }
}
