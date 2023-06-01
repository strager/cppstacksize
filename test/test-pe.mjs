import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import url from "node:url";
import { NodeBufferReader, SubFileReader } from "../src/reader.mjs";
import { describe, it } from "node:test";
import { getPEDebugDirectoryAsync, getPESectionsAsync } from "../src/pe.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

describe("PE file sections", (t) => {
  it("pdb/example.dll", async () => {
    let file = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "pdb/example.dll"))
    );
    let sections = await getPESectionsAsync(file);
    sections = sections.map((section) => ({
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
    let debugEntries = await getPEDebugDirectoryAsync(file);
    debugEntries = debugEntries.map((entry) => ({
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
