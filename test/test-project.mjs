import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import {
  CodeViewTypesInSeparatePDBFileError,
  findAllCodeViewFunctionsAsync,
  findAllCodeViewFunctions2Async,
  getCodeViewFunctionLocalsAsync,
  parseCodeViewTypesAsync,
  parseCodeViewTypesWithoutHeaderAsync,
} from "../src/codeview.mjs";
import {
  PDBMagicMismatchError,
  parsePDBDBIStreamAsync,
  parsePDBHeaderAsync,
  parsePDBStreamDirectoryAsync,
  parsePDBTPIStreamHeaderAsync,
} from "../src/pdb.mjs";
import { fallbackLogger } from "../src/logger.mjs";
import { assertRejectsAsync } from "./assert-util.mjs";
import { findCOFFSectionsByNameAsync } from "../src/coff.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

class ProjectFile {
  name;
  reader;

  pdbSuperBlock = null;
  pdbStreams = null;
  pdbDBI = null;
  pdbTPIHeader = null;

  constructor(name, reader) {
    this.name = name;
    this.reader = reader;
  }

  async tryLoadPDBGenericHeadersAsync(logger) {
    try {
      if (this.pdbSuperBlock === null) {
        this.pdbSuperBlock = await parsePDBHeaderAsync(this.reader, logger);
      }
      if (this.pdbStreams === null) {
        this.pdbStreams = await parsePDBStreamDirectoryAsync(
          this.reader,
          this.pdbSuperBlock,
          logger
        );
      }
    } catch (e) {
      if (e instanceof PDBMagicMismatchError) {
        return;
      }
      throw e;
    }
  }
}

class Project {
  #files = [];

  addFile(name, reader) {
    this.#files.push(new ProjectFile(name, reader));
  }

  async getTypeTableAsync(logger = fallbackLogger) {
    for (let file of this.#files) {
      await file.tryLoadPDBGenericHeadersAsync(logger);
      if (file.pdbStreams !== null) {
        if (file.pdbTPIHeader === null) {
          file.pdbTPIHeader = await parsePDBTPIStreamHeaderAsync(
            file.pdbStreams[2],
            logger
          );
        }
        // TODO[start-type-id]
        return await parseCodeViewTypesWithoutHeaderAsync(
          file.pdbTPIHeader.typeReader,
          logger
        );
      }

      for (let sectionReader of await findCOFFSectionsByNameAsync(
        file.reader,
        ".debug$T"
      )) {
        return await parseCodeViewTypesAsync(sectionReader);
      }
    }
    return null;
  }

  async getAllFunctionsAsync(logger = fallbackLogger) {
    let funcs = [];
    for (let file of this.#files) {
      await file.tryLoadPDBGenericHeadersAsync(logger);
      if (file.pdbStreams !== null) {
        if (file.pdbDBI === null) {
          file.pdbDBI = await parsePDBDBIStreamAsync(
            file.pdbStreams[3],
            logger
          );
        }
        for (let module of file.pdbDBI.modules) {
          let codeViewStream = file.pdbStreams[module.debugInfoStreamIndex];
          funcs.push(
            ...(await findAllCodeViewFunctions2Async(codeViewStream, logger))
          );
        }
      } else {
        for (let sectionReader of await findCOFFSectionsByNameAsync(
          file.reader,
          ".debug$S"
        )) {
          funcs.push(...(await findAllCodeViewFunctionsAsync(sectionReader)));
        }
      }
    }
    return funcs;
  }
}

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
    assert.strictEqual(await funcs[0].getCallerStackSizeAsync(typeTable), 32);
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
    // Type table should load.
    let typeTable = await project.getTypeTableAsync();
    assert.strictEqual(await funcs[0].getCallerStackSizeAsync(typeTable), 40);
  });
});
