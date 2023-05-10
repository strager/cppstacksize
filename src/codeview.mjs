import { SubFileReader } from "../src/reader.mjs";

// CodeView signatures:
let CV_SIGNATURE_C13 = 4;

// Subsection types:
let DEBUG_S_SYMBOLS = 0xf1;

// Symbol types:
let S_GPROC32_ID = 0x1147;

function alignUp(n, alignment) {
  let mask = alignment - 1;
  return (n + mask) & ~mask;
}

export function findAllCodeViewFunctions(reader) {
  let signature = reader.u32(0);
  if (signature !== CV_SIGNATURE_C13) {
    throw new UnsupportedCodeViewError(
      `unrecognized CodeView signature: 0x${signature.toString(16)}`
    );
  }

  let functions = [];
  let offset = 4;
  while (offset < reader.size) {
    offset = alignUp(offset, 4);
    let subsectionType = reader.u32(offset);
    offset += 4;
    let subsectionSize = reader.u32(offset);
    offset += 4;
    switch (subsectionType) {
      case DEBUG_S_SYMBOLS:
        findAllCodeViewFunctionsInSubsection(
          new SubFileReader(reader, offset, subsectionSize),
          functions
        );
        break;
      default:
        // Ignore.
        break;
    }
    offset += subsectionSize;
  }
  return functions;
}

function findAllCodeViewFunctionsInSubsection(reader, outFunctions) {
  let offset = 0;
  while (offset < reader.size) {
    let recordSize = reader.u16(offset + 0);
    let recordType = reader.u16(offset + 2);
    switch (recordType) {
      case S_GPROC32_ID: {
        outFunctions.push({
          name: reader.utf8CString(offset + 39),
          reader: reader,
          byteOffset: offset,
        });
        break;
      }
      default:
        break;
    }

    offset += recordSize + 2;
  }
}
