import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import {
  findCOFFSectionsByNameAsync,
  getCOFFSectionsAsync,
} from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("sections", (t) => {
  it("small.obj findCOFFSectionsByNameAsync", async () => {
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

  it("small.obj", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/small.obj"))
    );
    let sections = await getCOFFSectionsAsync(file);
    sections = sections.map((section) => ({
      name: section.name,
      dataSize: section.dataSize,
      dataFileOffset: section.dataFileOffset,
    }));

    // Data according to: dumpbin.exe /HEADERS
    assert.deepStrictEqual(sections, [
      {
        name: ".drectve",
        dataSize: 0x2f,
        dataFileOffset: 0xdc,
      },
      {
        name: ".debug$S",
        dataSize: 0x1e8,
        dataFileOffset: 0x10b,
      },
      {
        name: ".text$mn",
        dataSize: 0x3,
        dataFileOffset: 0x31b,
      },
      {
        name: ".debug$T",
        dataSize: 0x68,
        dataFileOffset: 0x31e,
      },
      {
        name: ".chks64",
        dataSize: 0x28,
        dataFileOffset: 0x386,
      },
    ]);
  });
});
