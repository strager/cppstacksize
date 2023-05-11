import assert from "node:assert";
import { describe, it } from "node:test";
import {
  ArrayLoader,
  LoaderReader,
  DataNotLoadedError,
} from "../src/loader.mjs";

describe("LoaderReader", (t) => {
  it("reading fails if data was not fetched", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
    let reader = new LoaderReader(loader, { chunkSize: 8 });
    for (let offset of [0, 1, 4]) {
      let error = assertThrows(() => {
        reader.u32(offset);
      }, DataNotLoadedError);
      assert.strictEqual(error.loader, loader);
      assert.strictEqual(error.offset, offset);
      assert.strictEqual(error.size, 4);
    }
  });

  it("reading succeeds if data was fetched", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(0, 4);
    assert.strictEqual(reader.u32(0), 0x04030201);
  });

  it("reading between chunks fails if data was not fetched", async () => {
    for (let { fetchFirstChunk, fetchLastChunk } of [
      { fetchFirstChunk: false, fetchLastChunk: false },
      { fetchFirstChunk: true, fetchLastChunk: false },
      { fetchFirstChunk: false, fetchLastChunk: true },
    ]) {
      let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
      let reader = new LoaderReader(loader, { chunkSize: 8 });
      if (fetchFirstChunk) {
        await reader.fetchAsync(0, 8);
      }
      if (fetchLastChunk) {
        await reader.fetchAsync(8, 8);
      }
      let error = assertThrows(() => {
        // 5..(5+4) straddles between two chunks.
        reader.u32(5);
      }, DataNotLoadedError);
      assert.strictEqual(error.loader, loader);
      assert.strictEqual(error.offset, 5);
      assert.strictEqual(error.size, 4);
    }
  });

  it("reading C string across multiple chunks succeeds if data was fetched", async () => {
    let data = "xhello beautiful world\0";
    //          ^   ^   ^   ^   ^   ^  -- start of each chunk
    let loader = new TestLoader(new TextEncoder().encode(data));
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(0, data.length);
    assert.strictEqual(reader.utf8CString(1), "hello beautiful world");
  });

  it("reading between chunks succeeds if both chunks were fetched separately", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
    let reader = new LoaderReader(loader, { chunkSize: 8 });
    await reader.fetchAsync(0, 8);
    await reader.fetchAsync(8, 8);
    // 5..(5+4) straddles between two chunks.
    assert.strictEqual(reader.u32(5), 0x09080706);
  });

  it("reading between chunks succeeds if both chunks were fetched together", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]);
    let reader = new LoaderReader(loader, { chunkSize: 8 });
    await reader.fetchAsync(5, 4);
    // 5..(5+4) straddles between two chunks.
    assert.strictEqual(reader.u32(5), 0x09080706);
  });

  it("reading from not-first chunk succeeds if data was fetched in not-first chunk", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(4, 4);
    assert.strictEqual(reader.u32(4), 0x08070605);
  });

  it("one-chunk fetch loads one chunk", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(0, 4);
    assert.deepStrictEqual(loader.readCalls, [{ offset: 0, size: 4 }]);
  });

  it("half-chunk fetch loads one chunk", async () => {
    for (let offset of [0, 2]) {
      let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
      let reader = new LoaderReader(loader, { chunkSize: 4 });
      await reader.fetchAsync(offset, 2);
      assert.deepStrictEqual(loader.readCalls, [{ offset: 0, size: 4 }]);
    }
  });

  it("multi-chunk fetch loads two chunks at once", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(1, 6);
    assert.deepStrictEqual(loader.readCalls, [{ offset: 0, size: 8 }]);
  });

  it("fetch does not load already-loaded chunk", async () => {
    let loader = new TestLoader([1, 2, 3, 4, 5, 6, 7, 8]);
    let reader = new LoaderReader(loader, { chunkSize: 4 });
    await reader.fetchAsync(1, 2);
    await reader.fetchAsync(0, 1);
    assert.deepStrictEqual(loader.readCalls, [{ offset: 0, size: 4 }]);
  });
});

class TestLoader {
  readCalls = [];
  #loader;

  constructor(bytes) {
    this.#loader = new ArrayLoader(bytes);
  }

  async readAsync(offset, size) {
    this.readCalls.push({ offset: offset, size: size });
    return await this.#loader.readAsync(offset, size);
  }
}

// Like assert.throws, but returns the thrown error.
function assertThrows(callback, ...args) {
  let error;
  assert.throws(() => {
    try {
      return callback();
    } catch (e) {
      error = e;
      throw e;
    }
  }, ...args);
  return error;
}
