import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import {
  CodeViewTypesInSeparatePDBFileError,
  findAllCodeViewFunctionsAsync,
  getCodeViewFunctionLocalsAsync,
  parseCodeViewTypesAsync,
} from "../src/codeview.mjs";
import { assertRejectsAsync } from "./assert-util.mjs";
import { findCOFFSectionsByNameAsync } from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("primitives.obj", (t) => {
  it("has one function", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/primitives.obj"))
    );
    let sectionReader = (
      await findCOFFSectionsByNameAsync(file, ".debug$S")
    )[0];

    let functions = await findAllCodeViewFunctionsAsync(sectionReader);
    assert.strictEqual(functions.length, 1);
    assert.strictEqual(functions[0].name, "primitives");
    assert.strictEqual(
      rebaseReaderOffset(functions[0].reader, functions[0].byteOffset, file),
      0x237
    );
    assert.strictEqual(functions[0].selfStackSize, 88);
  });

  it("function has local variables", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/primitives.obj"))
    );
    let sectionReader = (
      await findCOFFSectionsByNameAsync(file, ".debug$S")
    )[0];

    let func = (await findAllCodeViewFunctionsAsync(sectionReader))[0];
    let locals = await getCodeViewFunctionLocalsAsync(
      func.reader,
      func.byteOffset
    );

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

    let typeTable = null; // Primitive types do not need the type table.
    // prettier-ignore
    {
      assert.strictEqual(await localsByName.get("c").getByteSizeAsync(typeTable), 1);
      assert.strictEqual(await localsByName.get("sc").getByteSizeAsync(typeTable), 1);
      assert.strictEqual(await localsByName.get("uc").getByteSizeAsync(typeTable), 1);
      assert.strictEqual(await localsByName.get("ss").getByteSizeAsync(typeTable), 2);
      assert.strictEqual(await localsByName.get("us").getByteSizeAsync(typeTable), 2);
      assert.strictEqual(await localsByName.get("f").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("si").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("sl").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("ui").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("ul").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("ul").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("wc").getByteSizeAsync(typeTable), 4);
      assert.strictEqual(await localsByName.get("d").getByteSizeAsync(typeTable), 8);
      assert.strictEqual(await localsByName.get("ld").getByteSizeAsync(typeTable), 8);
      assert.strictEqual(await localsByName.get("sll").getByteSizeAsync(typeTable), 8);
      assert.strictEqual(await localsByName.get("ull").getByteSizeAsync(typeTable), 8);
    }
  });
});

describe("int parameters", (t) => {
  async function loadAsync(name) {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, name))
    );

    let typeTable = await parseCodeViewTypesAsync(
      (
        await findCOFFSectionsByNameAsync(file, ".debug$T")
      )[0]
    );

    let func = (
      await findAllCodeViewFunctionsAsync(
        (
          await findCOFFSectionsByNameAsync(file, ".debug$S")
        )[1]
      )
    )[0];

    return { func, typeTable };
  }

  it("void callee(void) has shadow space for four registers", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-0-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 32);
  });

  it("void callee(int) has shadow space for four registers", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-1-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 32);
  });

  it("void callee(int, int) has shadow space for four registers", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-2-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 32);
  });

  it("void callee(int, int, int, int) has shadow space for four registers", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-4-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 32);
  });

  it("void callee(int, int, int, int, int) has shadow space for four registers plus space for int on stack", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-5-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 40);
  });

  it("void callee(int, int, int, int, int, int) has shadow space for four registers plus space for two ints on stack", async () => {
    let { func, typeTable } = await loadAsync(
      "coff/parameters-int-6-callee.obj"
    );
    assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 48);
  });
});

describe("split COFF + PDB", (t) => {
  it("fails to load type info", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.obj"))
    );

    let sectionReader = (
      await findCOFFSectionsByNameAsync(file, ".debug$T")
    )[0];
    let error = await assertRejectsAsync(async () => {
      await parseCodeViewTypesAsync(sectionReader);
    }, CodeViewTypesInSeparatePDBFileError);
    assert.strictEqual(
      error.pdbPath,
      "C:\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\coff-pdb\\example.pdb"
    );
    assert.strictEqual(
      error.pdbGUID.toString(),
      "015182d6-09fa-4590-89e2-5abf55ea3c33"
    );
  });
});

function rebaseReaderOffset(reader, offset, desiredReader) {
  while (reader !== desiredReader) {
    if (reader instanceof SubFileReader) {
      offset += reader.subFileOffset;
      reader = reader.baseReader;
    } else {
      throw new Error("failed to rebase file offset");
    }
  }
  return offset;
}
