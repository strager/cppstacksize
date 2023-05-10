import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

class NodeBufferReader {
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

class SubFileReader {
  baseFile;
  subFileOffset;
  subFileSize;

  constructor(baseFile, offset, size) {
    this.baseFile = baseFile;
    this.subFileOffset = offset;
    this.subFileSize = size;
  }
}

function parseCOFFSection(file, offset) {
  return {
    name: file.fixedWidthString(offset, 8),
    dataSize: file.u32(offset + 16),
    dataFileOffset: file.u32(offset + 20),
  };
}

function findCOFFSectionsByName(file, sectionName) {
  let magic = file.u16(0);
  if (magic != 0x8664) {
    throw new COFFParseError(`unexpected magic: 0x${magic.toString(16)}`);
  }
  let sectionCount = file.u16(2);
  let optionalHeaderSize = file.u16(16);
  if (optionalHeaderSize != 0) {
    throw new COFFParseError(
      `unexpected optional header size: 0x${optionalHeaderSize.toString(16)}`
    );
  }

  let foundSections = [];
  for (let sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
    let section = parseCOFFSection(file, 20 + sectionIndex * 40);
    if (section.name === sectionName) {
      foundSections.push(
        new SubFileReader(file, section.dataFileOffset, section.dataSize)
      );
    }
  }
  return foundSections;
}

describe("debug symbols section", (t) => {
  it("small.obj", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/small.obj"))
    );
    let sectionReaders = findCOFFSectionsByName(file, ".debug$S");

    // Data according to: dumpbin.exe /HEADERS
    assert.strictEqual(sectionReaders.length, 1);
    assert.ok(sectionReaders[0] instanceof SubFileReader);
    assert.strictEqual(sectionReaders[0].baseFile, file);
    assert.strictEqual(sectionReaders[0].subFileOffset, 0x10b);
    assert.strictEqual(sectionReaders[0].subFileSize, 0x1e8);
  });
});
