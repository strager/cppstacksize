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

  it("function has local variables", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/primitives.obj"))
    );
    let sectionReader = findCOFFSectionsByName(file, ".debug$S")[0];

    let func = findAllCodeViewFunctions(sectionReader)[0];
    let locals = getCodeViewFunctionLocals(func.reader, func.byteOffset);

    let localsByName = new Map();
    for (let local of locals) {
      // Duplicate locals are not allowed.
      assert.strictEqual(localsByName.get(local.name), undefined);

      localsByName.set(local.name, local);
    }
    let localNames = [...localsByName.keys()].sort();
    assert.deepStrictEqual(localNames, [
      "c",
      "d",
      "f",
      "ld",
      "sc",
      "si",
      "sl",
      "sll",
      "ss",
      "uc",
      "ui",
      "ul",
      "ull",
      "us",
      "wc",
    ]);

    assert.strictEqual(localsByName.get("uc").spOffset, 0);
    assert.strictEqual(localsByName.get("sc").spOffset, 1);
    assert.strictEqual(localsByName.get("c").spOffset, 2);
    assert.strictEqual(localsByName.get("us").spOffset, 4);
    assert.strictEqual(localsByName.get("ss").spOffset, 8);
    assert.strictEqual(localsByName.get("wc").spOffset, 12);
    assert.strictEqual(localsByName.get("ui").spOffset, 16);
    assert.strictEqual(localsByName.get("ul").spOffset, 20);
    assert.strictEqual(localsByName.get("si").spOffset, 24);
    assert.strictEqual(localsByName.get("sl").spOffset, 28);
    assert.strictEqual(localsByName.get("f").spOffset, 32);
    assert.strictEqual(localsByName.get("ull").spOffset, 40);
    assert.strictEqual(localsByName.get("sll").spOffset, 48);
    assert.strictEqual(localsByName.get("d").spOffset, 56);
    assert.strictEqual(localsByName.get("ld").spOffset, 64);

    assert.strictEqual(localsByName.get("c").byteSize, 1);
    assert.strictEqual(localsByName.get("sc").byteSize, 1);
    assert.strictEqual(localsByName.get("uc").byteSize, 1);
    assert.strictEqual(localsByName.get("ss").byteSize, 2);
    assert.strictEqual(localsByName.get("us").byteSize, 2);
    assert.strictEqual(localsByName.get("f").byteSize, 4);
    assert.strictEqual(localsByName.get("si").byteSize, 4);
    assert.strictEqual(localsByName.get("sl").byteSize, 4);
    assert.strictEqual(localsByName.get("ui").byteSize, 4);
    assert.strictEqual(localsByName.get("ul").byteSize, 4);
    assert.strictEqual(localsByName.get("ul").byteSize, 4);
    assert.strictEqual(localsByName.get("wc").byteSize, 4);
    assert.strictEqual(localsByName.get("d").byteSize, 8);
    assert.strictEqual(localsByName.get("ld").byteSize, 8);
    assert.strictEqual(localsByName.get("sll").byteSize, 8);
    assert.strictEqual(localsByName.get("ull").byteSize, 8);
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
