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
  getCodeViewTypeAsync,
  parseCodeViewTypesAsync,
  parseCodeViewTypesWithoutHeaderAsync,
} from "../src/codeview.mjs";
import {
  parsePDBHeaderAsync,
  parsePDBStreamDirectoryAsync,
  parsePDBTPIStreamHeaderAsync,
} from "../src/pdb.mjs";
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
    let expectedLocalTypes = {
      c: { byteSize: 1, name: "char" },
      sc: { byteSize: 1, name: "signed char" },
      uc: { byteSize: 1, name: "unsigned char" },
      ss: { byteSize: 2, name: "short" },
      us: { byteSize: 2, name: "unsigned short" },
      f: { byteSize: 4, name: "float" },
      si: { byteSize: 4, name: "int" },
      sl: { byteSize: 4, name: "long" },
      ui: { byteSize: 4, name: "unsigned" },
      ul: { byteSize: 4, name: "unsigned long" },
      wc: { byteSize: 4, name: "wchar_t" },
      d: { byteSize: 8, name: "double" },
      // NOTE(strager): On Microsoft x64, long double is the same as double.
      ld: { byteSize: 8, name: "double" },
      sll: { byteSize: 8, name: "long long" },
      ull: { byteSize: 8, name: "unsigned long long" },
    };
    for (let localName in expectedLocalTypes) {
      let expectedType = expectedLocalTypes[localName];
      let actualType = await localsByName
        .get(localName)
        .getTypeAsync(typeTable);
      assert.strictEqual(
        actualType.byteSize,
        expectedType.byteSize,
        `actual byte size = ${actualType.byteSize}\nexpected byte size = ${expectedType.byteSize}\nlocalName = '${localName}'`
      );
      assert.strictEqual(
        actualType.name,
        expectedType.name,
        `actual name = ${actualType.name}\nexpected name = ${expectedType.name}\nlocalName = '${localName}'`
      );
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
  it("fails to load type info from COFF", async () => {
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

  it("COFF can load type info from PDB TPI+IPI", async () => {
    let objFile = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.obj"))
    );
    let pdbFile = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.pdb"))
    );

    let pdbStreams = await parsePDBStreamDirectoryAsync(
      pdbFile,
      await parsePDBHeaderAsync(pdbFile)
    );
    let pdbTPIHeader = await parsePDBTPIStreamHeaderAsync(pdbStreams[2]);
    let pdbTypeTable = await parseCodeViewTypesWithoutHeaderAsync(
      pdbTPIHeader.typeReader
    );
    let pdbIPIHeader = await parsePDBTPIStreamHeaderAsync(pdbStreams[4]);
    let pdbTypeIndexTable = await parseCodeViewTypesWithoutHeaderAsync(
      pdbIPIHeader.typeReader
    );

    let coffFuncs = await findAllCodeViewFunctionsAsync(
      (
        await findCOFFSectionsByNameAsync(objFile, ".debug$S")
      )[0]
    );

    assert.strictEqual(
      await coffFuncs[0].getCallerStackSizeAsync(
        pdbTypeTable,
        pdbTypeIndexTable
      ),
      40
    );
  });
});

test("CodeView special types", async () => {
  let testTypes = [
    // Normal non-pointer types:
    { typeID: 0x10, byteSize: 1, name: "signed char" },
    { typeID: 0x11, byteSize: 2, name: "short" },
    { typeID: 0x12, byteSize: 4, name: "long" },
    { typeID: 0x13, byteSize: 8, name: "long long" },
    { typeID: 0x20, byteSize: 1, name: "unsigned char" },
    { typeID: 0x21, byteSize: 2, name: "unsigned short" },
    { typeID: 0x22, byteSize: 4, name: "unsigned long" },
    { typeID: 0x23, byteSize: 8, name: "unsigned long long" },
    { typeID: 0x40, byteSize: 4, name: "float" },
    { typeID: 0x41, byteSize: 8, name: "double" },
    { typeID: 0x70, byteSize: 1, name: "char" },
    { typeID: 0x71, byteSize: 4, name: "wchar_t" },
    { typeID: 0x74, byteSize: 4, name: "int" },
    { typeID: 0x75, byteSize: 4, name: "unsigned" },

    // 64-bit pointer types:
    { typeID: 0x603, byteSize: 8, name: "void *" },
    { typeID: 0x610, byteSize: 8, name: "signed char *" },
    { typeID: 0x611, byteSize: 8, name: "short *" },
    { typeID: 0x612, byteSize: 8, name: "long *" },
    { typeID: 0x613, byteSize: 8, name: "long long *" },
    { typeID: 0x620, byteSize: 8, name: "unsigned char *" },
    { typeID: 0x621, byteSize: 8, name: "unsigned short *" },
    { typeID: 0x622, byteSize: 8, name: "unsigned long *" },
    { typeID: 0x623, byteSize: 8, name: "unsigned long long *" },
    { typeID: 0x640, byteSize: 8, name: "float *" },
    { typeID: 0x641, byteSize: 8, name: "double *" },
    { typeID: 0x670, byteSize: 8, name: "char *" },
    { typeID: 0x671, byteSize: 8, name: "wchar_t *" },
    { typeID: 0x674, byteSize: 8, name: "int *" },
    { typeID: 0x675, byteSize: 8, name: "unsigned *" },
  ];
  let typeTable = null; // Primitive types do not need the type table.
  for (let testType of testTypes) {
    let actualType = await getCodeViewTypeAsync(testType.typeID, typeTable);
    assert.strictEqual(
      actualType.byteSize,
      testType.byteSize,
      `actual byte size = ${actualType.byteSize}\nexpected byte size = ${
        testType.byteSize
      }\ntypeID = 0x${testType.typeID.toString(16)}`
    );
    assert.strictEqual(
      actualType.name,
      testType.name,
      `actual name = ${actualType.name}\nexpected name = ${
        testType.name
      }\ntypeID = 0x${testType.typeID.toString(16)}`
    );
  }
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
