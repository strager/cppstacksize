import assert from "node:assert";
import { describe, it } from "node:test";
import {
  ArrayBufferReader,
  CStringNullTerminatorNotFoundError,
  Location,
  NodeBufferReader,
  SubFileReader,
} from "../src/reader.mjs";
import { ArrayLoader, LoaderReader } from "../src/loader.mjs";
import { PDBBlocksReader } from "../src/pdb.mjs";

describe("NodeBufferReader", (t) => {
  testReader((bytes) => new NodeBufferReader(Buffer.from(bytes)));
});

describe("ArrayBufferReader", (t) => {
  testReader((bytes) => new ArrayBufferReader(new Uint8Array(bytes).buffer));
});

describe("LoaderReader + ArrayLoader, prefetched", (t) => {
  testReader(async (bytes) => {
    let reader = new LoaderReader(new ArrayLoader(bytes));
    await reader.fetchAsync(0, bytes.length);
    return reader;
  });
});

describe("LoaderReader(chunkSize=1) + ArrayLoader, prefetched", (t) => {
  testReader(async (bytes) => {
    let reader = new LoaderReader(new ArrayLoader(bytes), { chunkSize: 1 });
    await reader.fetchAsync(0, bytes.length);
    return reader;
  });
});

describe("PDBBlocksReader(blockSize=4) + NodeBufferReader", (t) => {
  testReader(async (bytes) => {
    let baseReader = new NodeBufferReader(Buffer.from(bytes));
    let blockSize = 4;
    let blockIndexes = [];
    for (let i = 0; i * blockSize < baseReader.size; ++i) {
      blockIndexes.push(i);
    }
    return new PDBBlocksReader(
      baseReader,
      blockIndexes,
      blockSize,
      baseReader.size,
      /*streamIndex=*/ 0
    );
  });
});

describe("SubFileReader with full NodeBufferReader", (t) => {
  testReader((bytes) => {
    let baseReader = new NodeBufferReader(Buffer.from(bytes));
    return new SubFileReader(baseReader, 0, bytes.length);
  });
});

describe("SubFileReader with partial NodeBufferReader", (t) => {
  testReader((bytes) => {
    let allBytes = [
      0xcc,
      0xdd,
      0xee,
      0xff,
      ...bytes,
      0xcc,
      0xdd,
      0xee,
      0xff,
      0x00,
    ];
    let baseReader = new NodeBufferReader(Buffer.from(allBytes));
    return new SubFileReader(baseReader, 4, bytes.length);
  });
});

describe("Reader location", () => {
  it("NodeBufferReader", async () => {
    let reader = new NodeBufferReader(Buffer.from([1, 2, 3, 4]));

    let location = reader.locate(0);
    assert.strictEqual(location.fileOffset, 0);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);

    location = reader.locate(2);
    assert.strictEqual(location.fileOffset, 2);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);
  });

  it("LoaderReader", async () => {
    let reader = new LoaderReader(new ArrayLoader([1, 2, 3, 4]));

    let location = reader.locate(0);
    assert.strictEqual(location.fileOffset, 0);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);

    location = reader.locate(2);
    assert.strictEqual(location.fileOffset, 2);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);
  });

  it("PDBBlocksReader", async () => {
    let reader = new PDBBlocksReader(
      /*baseReader=*/ new NodeBufferReader(Buffer.from([1, 2, 3, 4])),
      /*blockIndexes=*/ [1, 0],
      /*blockSize=*/ 2,
      /*byteSize=*/ 4,
      /*streamIndex=*/ 42
    );

    let location = reader.locate(0);
    assert.strictEqual(location.fileOffset, 2);
    assert.strictEqual(location.streamIndex, 42);
    assert.strictEqual(location.streamOffset, 0);

    location = reader.locate(1);
    assert.strictEqual(location.fileOffset, 3);
    assert.strictEqual(location.streamIndex, 42);
    assert.strictEqual(location.streamOffset, 1);

    location = reader.locate(2);
    assert.strictEqual(location.fileOffset, 0);
    assert.strictEqual(location.streamIndex, 42);
    assert.strictEqual(location.streamOffset, 2);

    location = reader.locate(3);
    assert.strictEqual(location.fileOffset, 1);
    assert.strictEqual(location.streamIndex, 42);
    assert.strictEqual(location.streamOffset, 3);
  });

  it("SubFileReader with offset", async () => {
    let reader = new SubFileReader(
      new NodeBufferReader(Buffer.from([1, 2, 3, 4])),
      /*offset=*/ 1,
      /*size=*/ 2
    );

    let location = reader.locate(0);
    assert.strictEqual(location.fileOffset, 1);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);

    location = reader.locate(2);
    assert.strictEqual(location.fileOffset, 3);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);
  });

  it("SubFileReader with offset of SubFileReader with offset", async () => {
    let reader = new SubFileReader(
      new SubFileReader(
        new NodeBufferReader(Buffer.from([1, 2, 3, 4, 5, 6, 7])),
        /*offset=*/ 1,
        /*size=*/ 5
      ),
      /*offset=*/ 2,
      /*size=*/ 2
    );

    let location = reader.locate(0);
    assert.strictEqual(location.fileOffset, 0 + 2 + 1);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);

    location = reader.locate(2);
    assert.strictEqual(location.fileOffset, 2 + 2 + 1);
    assert.strictEqual(location.streamIndex, null);
    assert.strictEqual(location.streamOffset, null);
  });

  it("serializes with only file offset", () => {
    assert.strictEqual(
      new Location(42, null, null).toString(),
      "file offset 0x2a"
    );
  });

  it("serializes with stream offset", () => {
    assert.strictEqual(
      new Location(42, 420, 69).toString(),
      "stream #420 offset 0x45 (file offset 0x2a)"
    );
  });

  it("serializes with stream directory offset", () => {
    assert.strictEqual(
      new Location(42, -1, 69).toString(),
      "stream directory offset 0x45 (file offset 0x2a)"
    );
  });
});

function testReader(makeReaderAsync) {
  it("reads u16", async () => {
    let r = await makeReaderAsync([1, 2, 3, 4, 5]);
    assert.strictEqual(r.u16(0), 0x0201);
    assert.strictEqual(r.u16(1), 0x0302);
    assert.strictEqual(r.u16(2), 0x0403);
  });

  it("reads u32", async () => {
    let r = await makeReaderAsync([1, 2, 3, 4, 5]);
    if (r instanceof PDBBlocksReader) {
      // TODO(strager): Reading u32 straddling multiple blocks is not yet
      // implemented by PDBBlocksReader.
      return;
    }
    assert.strictEqual(r.u32(0), 0x04030201);
    assert.strictEqual(r.u32(1), 0x05040302);
  });

  it("reads fixed with string", async () => {
    let r = await makeReaderAsync([
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
    let r = await makeReaderAsync([
      // "hello\0"
      0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00,
      // "wörld\0"
      0x77, 0xc3, 0xb6, 0x72, 0x6c, 0x64, 0x00,
    ]);
    assert.strictEqual(r.utf8CString(0), "hello");
    assert.strictEqual(r.utf8CString(6), "wörld");
  });

  it("finds u8 if present", async () => {
    let r = await makeReaderAsync([10, 20, 30, 40, 50, 60, 70]);
    assert.strictEqual(r.findU8(10, 0), 0);
    assert.strictEqual(r.findU8(20, 0), 1);
    assert.strictEqual(r.findU8(30, 0), 2);
    assert.strictEqual(r.findU8(40, 0), 3);
    assert.strictEqual(r.findU8(50, 0), 4);

    assert.strictEqual(r.findU8(30, 2), 2);
    assert.strictEqual(r.findU8(50, 2), 4);

    assert.strictEqual(r.findU8(10, 0, 6), 0);
    assert.strictEqual(r.findU8(50, 0, 6), 4);
  });

  it("fails to find u8 if missing", async () => {
    let r = await makeReaderAsync([10, 20, 30, 40, 50, 60, 70]);
    assert.strictEqual(r.findU8(0, 0), null);
    assert.strictEqual(r.findU8(15, 0), null);
    assert.strictEqual(r.findU8(15, 0, 7), null);
    assert.strictEqual(r.findU8(15, 0, 100), null);

    assert.strictEqual(r.findU8(10, 2), null);

    assert.strictEqual(r.findU8(70, 0, 2), null);
    assert.strictEqual(r.findU8(50, 0, 4), null);
  });

  it("out of bounds u16 fails", async () => {
    let r = await makeReaderAsync([10, 20]);
    assert.throws(() => {
      r.u16(1);
    }, RangeError);
  });

  it("out of bounds u32 fails", async () => {
    let r = await makeReaderAsync([10, 20, 30, 40]);
    if (r instanceof PDBBlocksReader) {
      // TODO(strager): Reading u32 straddling multiple blocks is not yet
      // implemented by PDBBlocksReader.
      return;
    }
    assert.throws(() => {
      r.u32(2);
    }, RangeError);
  });

  it("out of bounds utf8CString fails", async () => {
    let r = await makeReaderAsync([0x6c, 0x6f, 0x6c]);
    assert.throws(() => {
      r.utf8CString(0);
    }, CStringNullTerminatorNotFoundError);
  });

  it("out of bounds utf8String fails", async () => {
    let r = await makeReaderAsync([0x6c, 0x6f, 0x6c]);
    assert.throws(() => {
      r.utf8String(0, 4);
    }, RangeError);
    assert.throws(() => {
      r.utf8String(0, 100);
    }, RangeError);
  });
}
