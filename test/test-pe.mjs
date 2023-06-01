import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import { assertRejectsAsync } from "./assert-util.mjs";
import { describe, it } from "node:test";
import {
  PEMagicMismatchError,
  getPEPDBReferenceAsync,
  parsePEFileAsync,
} from "../src/pe.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("PE file sections", (t) => {
  it("pdb/example.dll", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "pdb/example.dll"))
    );
    let pe = await parsePEFileAsync(file);
    let sections = pe.sections.map((section) => ({
      name: section.name,
      dataSize: section.dataSize,
      dataFileOffset: section.dataFileOffset,
    }));

    // Data according to: dumpbin.exe /HEADERS
    assert.deepStrictEqual(sections, [
      {
        name: ".text",
        dataSize: 0xe00,
        dataFileOffset: 0x400,
      },
      {
        name: ".rdata",
        dataSize: 0xe00,
        dataFileOffset: 0x1200,
      },
      {
        name: ".data",
        dataSize: 0x200,
        dataFileOffset: 0x2000,
      },
      {
        name: ".pdata",
        dataSize: 0x200,
        dataFileOffset: 0x2200,
      },
      {
        name: ".reloc",
        dataSize: 0x200,
        dataFileOffset: 0x2400,
      },
    ]);
  });
});

describe("PE debug directory", (t) => {
  it("pdb/example.dll", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "pdb/example.dll"))
    );
    let pe = await parsePEFileAsync(file);
    let debugEntries = pe.debugDirectory.map((entry) => ({
      type: entry.type,
      dataSize: entry.dataSize,
      dataRVA: entry.dataRVA,
      dataFileOffset: entry.dataFileOffset,
    }));

    // Data according to: dumpbin.exe /HEADERS
    assert.deepStrictEqual(debugEntries, [
      {
        type: 2, // IMAGE_DEBUG_TYPE_CODEVIEW ("cv")
        dataSize: 0x5e,
        dataRVA: 0x23d0,
        dataFileOffset: 0x15d0,
      },
      {
        type: 12, // undocumented ("feat")
        dataSize: 0x14,
        dataRVA: 0x2430,
        dataFileOffset: 0x1630,
      },
      {
        type: 13, // undocumented ("coffgrp")
        dataSize: 0x230,
        dataRVA: 0x2444,
        dataFileOffset: 0x1644,
      },
    ]);
  });
});

describe("PE PDB reference", (t) => {
  it("pdb/example.dll", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "pdb/example.dll"))
    );
    let pe = await parsePEFileAsync(file);
    let reference = await getPEPDBReferenceAsync(pe);
    assert.ok(reference !== null);
    assert.strictEqual(
      reference.pdbGUID.toString(),
      "597c058d-affe-4abf-a0ea-76a2e3a3d099"
    );
    assert.strictEqual(
      reference.pdbPath,
      "C:\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\pdb\\example.pdb"
    );
  });
});

describe("corrupted PE", () => {
  it("pdb/example.dll with invalid DOS signature", async () => {
    let originalFileBuffer = await fs.promises.readFile(
      path.join(__dirname, "pdb/example.dll")
    );
    for (let i = 0; i < 2; ++i) {
      let corruptedFileBuffer = Buffer.from(originalFileBuffer);
      corruptedFileBuffer[i] ^= 0xcc;
      let file = new NodeBufferReader(corruptedFileBuffer);
      await assertRejectsAsync(
        async () => await parsePEFileAsync(file),
        PEMagicMismatchError
      );
    }
  });

  it("pdb/example.dll with invalid PE signature", async () => {
    let originalFileBuffer = await fs.promises.readFile(
      path.join(__dirname, "pdb/example.dll")
    );
    for (let i = 0; i < 4; ++i) {
      let corruptedFileBuffer = Buffer.from(originalFileBuffer);
      corruptedFileBuffer[0x0100 + i] ^= 0xcc;
      let file = new NodeBufferReader(corruptedFileBuffer);
      await assertRejectsAsync(
        async () => await parsePEFileAsync(file),
        PEMagicMismatchError
      );
    }
  });
});
