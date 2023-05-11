import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import { findCOFFSectionsByNameAsync } from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("debug symbols section", (t) => {
  it("small.obj", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/small.obj"))
    );
    let sectionReaders = await findCOFFSectionsByNameAsync(file, ".debug$S");

    // Data according to: dumpbin.exe /HEADERS
    assert.strictEqual(sectionReaders.length, 1);
    assert.ok(sectionReaders[0] instanceof SubFileReader);
    assert.strictEqual(sectionReaders[0].baseReader, file);
    assert.strictEqual(sectionReaders[0].subFileOffset, 0x10b);
    assert.strictEqual(sectionReaders[0].subFileSize, 0x1e8);
  });
});
