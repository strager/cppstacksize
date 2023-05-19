import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import test, { describe, it } from "node:test";
import url from "node:url";
import {
  ArrayBufferReader,
  NodeBufferReader,
  SubFileReader,
} from "../src/reader.mjs";
import {
  PDBBlocksReader,
  PDBMagicMismatchError,
  parsePDBDBIStreamAsync,
  parsePDBHeaderAsync,
  parsePDBStreamDirectoryAsync,
  parsePDBTPIStreamHeaderAsync,
} from "../src/pdb.mjs";
import { assertRejectsAsync } from "./assert-util.mjs";
import {
  findAllCodeViewFunctions2Async,
  getCodeViewFunctionLocalsAsync,
  parseCodeViewTypesWithoutHeaderAsync,
} from "../src/codeview.mjs";

let __filename = url.fileURLToPath(import.meta.url);
let __dirname = path.dirname(__filename);

let pdbFileMagic = [
  ...new TextEncoder("utf-8").encode("Microsoft C/C++ MSF 7.00\r\n"),
  0x1a,
  0x44,
  0x53,
  0x00,
  0x00,
  0x00,
];

describe("PDB file", (t) => {
  it("reads stream directory block indexes", async () => {
    // In this test, we write code to manually construct a PDB file. It has a
    // stream directory which spans only one block and contains only one stream,
    // simulating a small (but incomplete) PDB file.

    let blockSize = 512;
    let blockCount = 1000;
    let freeBlockMapBlockIndex = 1;

    let fileData = new ArrayBuffer(blockSize * blockCount);
    let fileDataView = new DataView(fileData);
    let fileDataBytes = new Uint8Array(fileData);

    // Superblock (block #0):
    fileDataBytes.set(pdbFileMagic, 0);
    fileDataView.setUint32(0x20, blockSize, /*littleEndian=*/ true);
    fileDataView.setUint32(
      0x24,
      freeBlockMapBlockIndex,
      /*littleEndian=*/ true
    );
    fileDataView.setUint32(0x28, blockCount, /*littleEndian=*/ true);
    let directorySize = 4 + 1 * 4 + 1 * 4;
    fileDataView.setUint32(0x2c, directorySize, /*littleEndian=*/ true);
    let directoryMapBlockIndex = 69;
    fileDataView.setUint32(
      0x34,
      directoryMapBlockIndex,
      /*littleEndian=*/ true
    );

    // Directory map (block #69):
    let directoryBlockIndex = 44;
    let directoryMapOffset = directoryMapBlockIndex * blockSize;
    fileDataView.setUint32(
      directoryMapOffset,
      directoryBlockIndex,
      /*littleEndian=*/ true
    );

    // Directory (blocks #44):
    {
      let offset = directoryBlockIndex * blockSize;

      let streamCount = 1;
      fileDataView.setUint32(offset, streamCount, /*littleEndian=*/ true);
      offset += 4;

      // Stream sizes:
      fileDataView.setUint32(offset, 1 * blockSize, /*littleEndian=*/ true);
      offset += 4;

      // Stream #0 blocks:
      fileDataView.setUint32(offset, 200, /*littleEndian=*/ true);
      offset += 4;
    }

    let file = new ArrayBufferReader(fileData);
    let superBlock = await parsePDBHeaderAsync(file);
    let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
    assert.deepStrictEqual(
      streams.map((stream) => ({
        blocks: stream.blockIndexes,
        size: stream.size,
      })),
      [
        // Stream #0 blocks:
        { blocks: [200], size: blockSize },
      ]
    );
  });

  it("reads stream directory block indexes with multi-block stream directory", async () => {
    // In this test, we write code to manually construct a PDB file. It has a
    // stream directory which spans two blocks. (This is hard to replicate with
    // a small sample program.)

    let blockSize = 512;
    let blockCount = 1000;
    let freeBlockMapBlockIndex = 1;
    let streams = [
      // Stream #0 (0.5 blocks):
      { blocks: [200], size: 0.5 * blockSize },
      // Stream #1 (1 block):
      { blocks: [300], size: 1 * blockSize },
      // Stream #2 (3.5 blocks):
      { blocks: [400, 401, 402, 403], size: 3.5 * blockSize },
      // Stream #3 (200 blocks):
      { blocks: [], size: 0 },
    ];
    for (let i = 0; i < 200; ++i) {
      streams[3].blocks.push(500 + i);
    }
    streams[3].size = streams[3].blocks.length * blockSize;

    let fileData = new ArrayBuffer(blockSize * blockCount);
    let fileDataView = new DataView(fileData);
    let fileDataBytes = new Uint8Array(fileData);

    // Superblock (block #0):
    fileDataBytes.set(pdbFileMagic, 0);
    fileDataView.setUint32(0x20, blockSize, /*littleEndian=*/ true);
    fileDataView.setUint32(
      0x24,
      freeBlockMapBlockIndex,
      /*littleEndian=*/ true
    );
    fileDataView.setUint32(0x28, blockCount, /*littleEndian=*/ true);
    let directorySize =
      4 +
      streams.length * 4 +
      streams.reduce((acc, stream) => stream.blocks.length * 4 + acc, 0);
    fileDataView.setUint32(0x2c, directorySize, /*littleEndian=*/ true);
    let directoryMapBlockIndex = 42;
    fileDataView.setUint32(
      0x34,
      directoryMapBlockIndex,
      /*littleEndian=*/ true
    );

    // Directory map (block #42):
    let directoryBlockIndexes = [44, 9];
    let directoryMapOffset = directoryMapBlockIndex * blockSize;
    for (let [i, directoryBlockIndex] of directoryBlockIndexes.entries()) {
      fileDataView.setUint32(
        directoryMapOffset + i * 4,
        directoryBlockIndex,
        /*littleEndian=*/ true
      );
    }

    // Directory (blocks #44 and #9):
    {
      let directoryBlock0Begin = directoryBlockIndexes[0] * blockSize;
      let directoryBlock0End = directoryBlock0Begin + blockSize;
      let directoryBlock1Begin = directoryBlockIndexes[1] * blockSize;
      let directoryBlock1End = directoryBlock1Begin + blockSize;
      let offset = directoryBlock0Begin;

      let streamCount = streams.length;
      fileDataView.setUint32(offset, streamCount, /*littleEndian=*/ true);
      offset += 4;

      // Stream sizes:
      fileDataView.setUint32(offset, streams[0].size, /*littleEndian=*/ true);
      offset += 4;
      fileDataView.setUint32(offset, streams[1].size, /*littleEndian=*/ true);
      offset += 4;
      fileDataView.setUint32(offset, streams[2].size, /*littleEndian=*/ true);
      offset += 4;
      fileDataView.setUint32(offset, streams[3].size, /*littleEndian=*/ true);
      offset += 4;

      // Stream #0 blocks:
      fileDataView.setUint32(
        offset,
        streams[0].blocks[0],
        /*littleEndian=*/ true
      );
      offset += 4;
      // Stream #1 blocks:
      fileDataView.setUint32(
        offset,
        streams[1].blocks[0],
        /*littleEndian=*/ true
      );
      offset += 4;
      // Stream #2 blocks:
      fileDataView.setUint32(
        offset,
        streams[2].blocks[0],
        /*littleEndian=*/ true
      );
      offset += 4;
      fileDataView.setUint32(
        offset,
        streams[2].blocks[1],
        /*littleEndian=*/ true
      );
      offset += 4;
      fileDataView.setUint32(
        offset,
        streams[2].blocks[2],
        /*littleEndian=*/ true
      );
      offset += 4;
      fileDataView.setUint32(
        offset,
        streams[2].blocks[3],
        /*littleEndian=*/ true
      );
      offset += 4;

      // Stream #3 blocks (big stream) (first part):
      let i = 0;
      for (; i < streams[3].blocks.length && offset < directoryBlock0End; ++i) {
        fileDataView.setUint32(
          offset,
          streams[3].blocks[i],
          /*littleEndian=*/ true
        );
        offset += 4;
      }
      assert.strictEqual(offset, directoryBlock0End);
      // Stream #3 blocks (big stream) (second part):
      offset = directoryBlock1Begin;
      for (; i < streams[3].blocks.length; ++i) {
        fileDataView.setUint32(
          offset,
          streams[3].blocks[i],
          /*littleEndian=*/ true
        );
        offset += 4;
      }
      assert.strictEqual(i, streams[3].blocks.length);
      assert.ok(offset < directoryBlock1End);
    }

    let file = new ArrayBufferReader(fileData);
    let superBlock = await parsePDBHeaderAsync(file);
    let parsedStreams = await parsePDBStreamDirectoryAsync(file, superBlock);
    assert.deepStrictEqual(
      parsedStreams.map((stream) => ({
        blocks: stream.blockIndexes,
        size: stream.size,
      })),
      streams
    );
  });

  describe("example.pdb", (t) => {
    let filePromise = (async () =>
      new NodeBufferReader(
        await fs.promises.readFile(path.join(__dirname, "pdb/example.pdb"))
      ))();

    it("can read stream directory block indexes", async () => {
      let file = await filePromise;
      let superBlock = await parsePDBHeaderAsync(file);
      let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
      assert.deepStrictEqual(
        streams.map((stream) => ({
          blocks: stream.blockIndexes,
          size: stream.size,
        })),
        [
          { blocks: [7], size: 76 },
          { blocks: [84], size: 174 },
          { blocks: [83, 8, 74, 75, 76, 77, 78, 79, 80, 81, 82], size: 41736 },
          { blocks: [65, 66, 67, 68, 69, 70, 71, 72, 73], size: 32795 },
          { blocks: [92, 90, 91], size: 8456 },
          { blocks: [86, 85, 87, 88, 89], size: 18718 },
          { blocks: [93], size: 3188 },
          { blocks: [94], size: 960 },
          { blocks: [], size: 0 },
          { blocks: [], size: 0 },
          { blocks: [24, 25], size: 4452 },
          { blocks: [26, 27], size: 6048 },
          { blocks: [28, 29], size: 5248 },
          { blocks: [30, 31, 32], size: 10740 },
          { blocks: [23], size: 200 },
          { blocks: [10], size: 648 },
          { blocks: [12], size: 728 },
          { blocks: [9], size: 308 },
          { blocks: [16], size: 740 },
          { blocks: [17], size: 560 },
          { blocks: [18, 19], size: 4284 },
          { blocks: [20, 21], size: 4764 },
          { blocks: [22], size: 636 },
          { blocks: [33], size: 212 },
          { blocks: [34, 35], size: 6080 },
          { blocks: [36], size: 488 },
          { blocks: [37, 38], size: 7528 },
          { blocks: [39], size: 648 },
          { blocks: [40], size: 192 },
          { blocks: [41, 42], size: 4372 },
          { blocks: [43, 44], size: 6180 },
          { blocks: [45, 46], size: 5920 },
          { blocks: [47, 48], size: 6340 },
          { blocks: [49], size: 1500 },
          { blocks: [50], size: 1620 },
          { blocks: [51], size: 1532 },
          { blocks: [52, 53], size: 6352 },
          { blocks: [54, 55, 56], size: 10728 },
          { blocks: [57], size: 1088 },
          { blocks: [58], size: 152 },
          { blocks: [59], size: 3528 },
          { blocks: [61], size: 2860 },
          { blocks: [60, 62, 63, 64], size: 13812 },
          { blocks: [], size: 0 },
          { blocks: [], size: 4294967295 },
          { blocks: [], size: 0 },
        ]
      );
    });

    it("can read DBI stream", async () => {
      // Stream #3 from example.pdb.
      let dbiReader = new PDBBlocksReader(
        await filePromise,
        [65, 66, 67, 68, 69, 70, 71, 72, 73],
        /*blockSize=*/ 4096,
        /*byteSize=*/ 32795,
        /*streamIndex=*/ 3
      );

      let dbi = await parsePDBDBIStreamAsync(dbiReader);
      assert.strictEqual(dbi.modules.length, 29);

      assert.strictEqual(
        dbi.modules[0].linkedObjectPath,
        "C:\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\pdb\\example.obj"
      );
      assert.strictEqual(
        dbi.modules[0].sourceObjectPath,
        "C:\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\pdb\\example.obj"
      );
      assert.strictEqual(dbi.modules[0].debugInfoStreamIndex, 15);

      assert.strictEqual(
        dbi.modules[1].linkedObjectPath,
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.31.31103\\lib\\x64\\MSVCRT.lib"
      );
      assert.strictEqual(
        dbi.modules[1].sourceObjectPath,
        "d:\\a01\\_work\\43\\s\\Intermediate\\vctools\\msvcrt.nativeproj_110336922\\objr\\amd64\\dll_dllmain.obj"
      );
      assert.strictEqual(dbi.modules[1].debugInfoStreamIndex, 37);

      assert.strictEqual(dbi.modules[28].sourceObjectPath, "* Linker *");
      assert.strictEqual(dbi.modules[28].linkedObjectPath, "");
      assert.strictEqual(dbi.modules[28].debugInfoStreamIndex, 35);
    });

    it("can read TPI stream", async () => {
      // Stream #2 from example.pdb.
      let tpiReader = new PDBBlocksReader(
        await filePromise,
        [83, 8, 74, 75, 76, 77, 78, 79, 80, 81, 82],
        /*blockSize=*/ 4096,
        /*byteSize=*/ 41736,
        /*streamIndex=*/ 2
      );

      let dbi = await parsePDBTPIStreamHeaderAsync(tpiReader);
      assert.ok(dbi.typeReader instanceof SubFileReader);
      assert.ok(dbi.typeReader.subFileOffset, 0x20);
      assert.ok(dbi.typeReader.subFileSize, 41680);
      // TODO[start-type-id]
    });

    it("has example.cpp caller and callee functions", async () => {
      let file = await filePromise;
      let superBlock = await parsePDBHeaderAsync(file);
      let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
      let functions = await findAllCodeViewFunctions2Async(streams[15]);
      let functionNames = functions.map((func) => func.name).sort();
      assert.deepStrictEqual(functionNames, ["callee", "caller"]);
    });

    it("has example.cpp caller variables", async () => {
      let file = await filePromise;
      let superBlock = await parsePDBHeaderAsync(file);
      let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
      let func = (await findAllCodeViewFunctions2Async(streams[15]))[1];
      let locals = await getCodeViewFunctionLocalsAsync(
        func.reader,
        func.byteOffset
      );
      let localNames = locals.map((local) => local.name).sort();
      assert.deepStrictEqual(localNames, ["a"]);
    });

    it("has example.cpp callee variables", async () => {
      let file = await filePromise;
      let superBlock = await parsePDBHeaderAsync(file);
      let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
      let func = (await findAllCodeViewFunctions2Async(streams[15]))[0];
      let locals = await getCodeViewFunctionLocalsAsync(
        func.reader,
        func.byteOffset
      );
      let localNames = locals.map((local) => local.name).sort();
      assert.deepStrictEqual(localNames, ["a", "b", "c", "d", "e"]);
    });

    it("calculates caller stack size for 'callee' function", async () => {
      let file = await filePromise;
      let superBlock = await parsePDBHeaderAsync(file);
      let streams = await parsePDBStreamDirectoryAsync(file, superBlock);
      let func = (await findAllCodeViewFunctions2Async(streams[15]))[0];
      let tpiHeader = await parsePDBTPIStreamHeaderAsync(streams[2]);
      // TODO[start-type-id]
      let typeTable = await parseCodeViewTypesWithoutHeaderAsync(
        tpiHeader.typeReader
      );
      assert.strictEqual(await func.getCallerStackSizeAsync(typeTable), 40);
    });
  });

  it("rejects .obj file", async () => {
    let reader = new NodeBufferReader(
      await fs.promises.readFile(path.join(__dirname, "coff/small.obj"))
    );
    await assertRejectsAsync(async () => {
      await parsePDBHeaderAsync(reader);
    }, PDBMagicMismatchError);
  });

  it("rejects empty file", async () => {
    let reader = new ArrayBufferReader(new ArrayBuffer(0));
    await assertRejectsAsync(async () => {
      await parsePDBHeaderAsync(reader);
    }, PDBMagicMismatchError);
  });
});
