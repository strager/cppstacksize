import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import {
  findAllCodeViewFunctions,
  getCodeViewFunctionLocals,
} from "../src/codeview.mjs";
import { findCOFFSectionsByName } from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("NodeBufferReader", (t) => {
  testReader((bytes) => new NodeBufferReader(Buffer.from(bytes)));
});

describe("SubFileReader with full NodeBufferReader", (t) => {
  testReader((bytes) => {
    let baseReader = new NodeBufferReader(Buffer.from(bytes));
    return new SubFileReader(baseReader, 0, bytes.length);
  });
});

describe("SubFileReader with partial NodeBufferReader", (t) => {
  testReader((bytes) => {
    let allBytes = [0xcc, 0xdd, 0xee, 0xff, ...bytes, 0xcc, 0xdd, 0xee, 0xff];
    let baseReader = new NodeBufferReader(Buffer.from(allBytes));
    return new SubFileReader(baseReader, 4, bytes.length);
  });
});

function testReader(makeReader) {
  it("reads u16", async () => {
    let r = makeReader([1, 2, 3, 4, 5]);
    assert.strictEqual(r.u16(0), 0x0201);
    assert.strictEqual(r.u16(1), 0x0302);
    assert.strictEqual(r.u16(2), 0x0403);
  });

  it("reads u32", async () => {
    let r = makeReader([1, 2, 3, 4, 5]);
    assert.strictEqual(r.u32(0), 0x04030201);
    assert.strictEqual(r.u32(1), 0x05040302);
  });

  it("reads fixed with string", async () => {
    let r = makeReader([
      // "hello\0\0\0x"
      0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00, 0x00, 0x00, 0x78,
    ]);
    assert.strictEqual(r.fixedWidthString(0, 8), "hello");
    assert.strictEqual(r.fixedWidthString(2, 6), "llo");

    // No trailing zeros:
    assert.strictEqual(r.fixedWidthString(0, 5), "hello");
    assert.strictEqual(r.fixedWidthString(0, 4), "hell");
  });

  it("reads UTF-8 C string", async () => {
    let r = makeReader([
      // "hello\0"
      0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00,
      // "wörld\0"
      0x77, 0xc3, 0xb6, 0x72, 0x6c, 0x64, 0x00,
    ]);
    assert.strictEqual(r.utf8CString(0), "hello");
    assert.strictEqual(r.utf8CString(6), "wörld");
  });
}
