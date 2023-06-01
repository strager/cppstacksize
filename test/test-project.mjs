import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import { Project } from "../src/project.mjs";
import { assertRejectsAsync } from "./assert-util.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("Project", (t) => {
  it("loads complete .obj file", async () => {
    let project = new Project();
    project.addFile(
      "parameters-int-0-callee.obj",
      new NodeBufferReader(
        await fs.promises.readFile(
          path.join(__dirname, "coff/parameters-int-0-callee.obj")
        )
      )
    );

    // Function table should load.
    let funcs = await project.getAllFunctionsAsync();
    assert.strictEqual(funcs[0].name, "callee");
    // Type table should load.
    let typeTable = await project.getTypeTableAsync();
    let typeIndexTable = await project.getTypeIndexTableAsync();
    assert.strictEqual(
      await funcs[0].getCallerStackSizeAsync(typeTable, typeIndexTable),
      32
    );
  });

  it("loads complete .pdb file", async () => {
    let project = new Project();
    project.addFile(
      "parameters-int-0-callee.obj",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "pdb/example.pdb"))
      )
    );

    // Function table should load.
    let funcs = await project.getAllFunctionsAsync();
    assert.strictEqual(funcs[0].name, "callee");
    // Type tables should load.
    let typeTable = await project.getTypeTableAsync();
    let typeIndexTable = await project.getTypeIndexTableAsync();
    assert.strictEqual(
      await funcs[0].getCallerStackSizeAsync(typeTable, typeIndexTable),
      40
    );
  });

  it("loads unlinked .pdb and .obj", async () => {
    let project = new Project();
    project.addFile(
      "example.pdb",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.pdb"))
      )
    );
    project.addFile(
      "example.obj",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.obj"))
      )
    );

    // Function table should load from .obj.
    let funcs = await project.getAllFunctionsAsync();
    assert.strictEqual(funcs[0].name, "callee");
    // Type tables should load from .pdb.
    let typeTable = await project.getTypeTableAsync();
    let typeIndexTable = await project.getTypeIndexTableAsync();
    assert.strictEqual(
      await funcs[0].getCallerStackSizeAsync(typeTable, typeIndexTable),
      40
    );
  });

  it("loads unlinked .obj and .pdb", async () => {
    let project = new Project();
    project.addFile(
      "example.obj",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.obj"))
      )
    );
    project.addFile(
      "example.pdb",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "coff-pdb/example.pdb"))
      )
    );

    // Function table should load from .obj.
    let funcs = await project.getAllFunctionsAsync();
    assert.strictEqual(funcs[0].name, "callee");
    // Type tables should load from .pdb.
    let typeTable = await project.getTypeTableAsync();
    let typeIndexTable = await project.getTypeIndexTableAsync();
    assert.strictEqual(
      await funcs[0].getCallerStackSizeAsync(typeTable, typeIndexTable),
      40
    );
  });

  it(".pdb functions link to loaded .dll", async () => {
    let project = new Project();
    project.addFile(
      "temporary.pdb",
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "pdb-pe/temporary.pdb"))
      )
    );
    let dllReader = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "pdb-pe/temporary.dll"))
    );
    project.addFile("temporary.dll", dllReader);

    // Function table should load from .pdb.
    let functions = await project.getAllFunctionsAsync();
    let functionsByName = new Map(functions.map((func) => [func.name, func]));

    let localVariableBytesReader = functionsByName
      .get("local_variable")
      .getInstructionBytesReader();
    assert.ok(localVariableBytesReader instanceof SubFileReader);
    assert.strictEqual(localVariableBytesReader.baseReader, dllReader);
    assert.strictEqual(localVariableBytesReader.subFileOffset, 0x0400);
    assert.strictEqual(localVariableBytesReader.subFileSize, 0x39);

    let temporaryReader = functionsByName
      .get("temporary")
      .getInstructionBytesReader();
    assert.ok(temporaryReader instanceof SubFileReader);
    assert.strictEqual(temporaryReader.baseReader, dllReader);
    assert.strictEqual(temporaryReader.subFileOffset, 0x0440);
    assert.strictEqual(temporaryReader.subFileSize, 0x39);
  });
});
