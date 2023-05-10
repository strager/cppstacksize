import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import { findAllCodeViewFunctions } from "../src/codeview.mjs";
import { findCOFFSectionsByName } from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("primitives.obj", (t) => {
  it("has one function", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/primitives.obj"))
    );
    let sectionReader = findCOFFSectionsByName(file, ".debug$S")[0];

    let functions = findAllCodeViewFunctions(sectionReader);
    assert.strictEqual(functions.length, 1);
    assert.strictEqual(functions[0].name, "primitives");
    assert.strictEqual(
      rebaseFileOffset(functions[0].reader, functions[0].byteOffset, file),
      0x237
    );
  });
});

function rebaseFileOffset(reader, offset, desiredReader) {
  while (reader !== desiredReader) {
    if (reader instanceof SubFileReader) {
      offset += reader.subFileOffset;
      reader = reader.baseFile;
    } else {
      throw new Error("failed to rebase file offset");
    }
  }
  return offset;
}
