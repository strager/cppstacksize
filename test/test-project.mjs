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
    console.error(typeTable.toString());
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
    console.error(typeTable.toString());
    assert.strictEqual(
      await funcs[0].getCallerStackSizeAsync(typeTable, typeIndexTable),
      40
    );
  });
});
