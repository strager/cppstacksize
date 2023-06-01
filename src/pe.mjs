import { GUID } from "./guid.mjs";
import { ReaderBase, SubFileReader } from "./reader.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

// Documentation:
// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

let IMAGE_DEBUG_TYPE_CODEVIEW = 2;

export class PEMagicMismatchError extends Error {
  constructor() {
    super("PE magic mismatched");
  }
}

// A Windows PE (.dll or .exe) or COFF (.obj) file.
class PEFile {
  reader;
  sections = [];
  debugDirectory = [];

  constructor(reader) {
    this.reader = reader;
  }

  // Returns a Reader for each section with the given name.
  findSectionsByName(sectionName) {
    let foundSections = [];
    for (let section of this.sections) {
      if (section.name === sectionName) {
        foundSections.push(this.readerForSection(section));
      }
    }
    return foundSections;
  }

  _parseSections(offset) {
    let coffMagic = this.reader.u16(offset);
    if (coffMagic != 0x8664) {
      throw new PEMagicMismatchError();
    }
    let sectionCount = this.reader.u16(offset + 2);
    let optionalHeaderSize = this.reader.u16(offset + 16);
    let sectionTableOffset = offset + 20 + optionalHeaderSize;
    for (let sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
      this.sections.push(
        parseCOFFSection(this.reader, sectionTableOffset + sectionIndex * 40)
      );
    }
  }

  _parseOptionalHeader(coffHeaderOffset) {
    let optionalHeaderSize = this.reader.u16(coffHeaderOffset + 16);
    if (optionalHeaderSize === 0) {
      throw new PEParseError("missing optional header");
    }

    let optionalHeaderReader = new SubFileReader(
      this.reader,
      coffHeaderOffset + 20,
      optionalHeaderSize
    );
    let optionalHeaderMagic = optionalHeaderReader.u16(0);
    let dataDirectoryOffset;
    if (optionalHeaderMagic === 0x10b) {
      // PE32
      dataDirectoryOffset = 96;
    } else if (optionalHeaderMagic === 0x20b) {
      // PE32+
      dataDirectoryOffset = 112;
    } else {
      throw new PEParseError(
        `unexpected optional header magic: 0x${optionalHeaderMagic.toString(
          16
        )}`
      );
    }

    let dataDirectoryReader = new SubFileReader(
      optionalHeaderReader,
      dataDirectoryOffset
    );
    let hasDebugDataDirectory = dataDirectoryReader.size >= 56;
    if (!hasDebugDataDirectory) {
      return [];
    }
    let debugDataDirectoryRVA = dataDirectoryReader.u32(48);
    let debugDataDirectorySize = dataDirectoryReader.u32(48 + 4);
    let debugDirectoryReader = this.readerForRVA(
      debugDataDirectoryRVA,
      debugDataDirectorySize
    );

    let offset = 0;
    while (offset < debugDirectoryReader.size) {
      this.debugDirectory.push({
        type: debugDirectoryReader.u32(offset + 12),
        dataSize: debugDirectoryReader.u32(offset + 16),
        dataRVA: debugDirectoryReader.u32(offset + 20),
        dataFileOffset: debugDirectoryReader.u32(offset + 24),
      });
      offset += 28;
    }
  }

  readerForSection(section) {
    return new SubFileReader(
      this.reader,
      section.dataFileOffset,
      section.dataSize
    );
  }

  readerForRVA(baseRVA, size) {
    // TODO(strager): Intelligent fallback if input spans multiple sections.
    // TODO(strager): Support 0 padding if baseRVA+size extends beyond dataSize.
    let section = this.sections.find(
      (section) =>
        section.virtualAddress <= baseRVA &&
        baseRVA + size <
          section.virtualAddress +
            Math.min(section.virtualSize, section.dataSize)
    );
    if (section === undefined) {
      throw new RangeError(
        `cannot find section for RVA 0x${baseRVA.toString(
          16
        )} size=0x${size.toString(16)}`
      );
    }
    return new SubFileReader(
      this.reader,
      baseRVA - section.virtualAddress + section.dataFileOffset,
      size
    );
  }
}

export function parsePEFileAsync(reader) {
  return withLoadScopeAsync(async () => {
    let pe = new PEFile(reader);

    let isDOSExecutable = reader.u16(0) === 0x5a4d; // "MZ"
    if (isDOSExecutable) {
      let peSignatureOffset = reader.u32(0x3c);
      if (reader.u32(peSignatureOffset) !== 0x00004550) {
        // "PE\0\0"
        throw new PEMagicMismatchError();
      }
      let coffHeaderOffset = peSignatureOffset + 4;
      pe._parseSections(coffHeaderOffset);
      pe._parseOptionalHeader(coffHeaderOffset);
    } else {
      pe._parseSections(0);
    }

    return pe;
  });
}

export function getPEPDBReferenceAsync(pe) {
  return withLoadScopeAsync(async () => {
    for (let entry of pe.debugDirectory) {
      if (entry.type === IMAGE_DEBUG_TYPE_CODEVIEW) {
        let result = parseCodeViewDebugDirectoryData(
          new SubFileReader(pe.reader, entry.dataFileOffset, entry.dataSize)
        );
        if (result !== null) {
          return result;
        }
      }
    }
    return null;
  });
}

export function parseCOFFSection(reader, offset) {
  return {
    name: reader.fixedWidthString(offset, 8),
    virtualSize: reader.u32(offset + 8),
    virtualAddress: reader.u32(offset + 12),
    dataSize: reader.u32(offset + 16),
    dataFileOffset: reader.u32(offset + 20),
  };
}

function parseCodeViewDebugDirectoryData(reader) {
  let magic = reader.u32(0);
  if (magic != 0x53445352) {
    // "RSDS"
    return null;
  }
  let pdbPath = reader.utf8CString(24);
  let pdbGUIDBytes = new Uint8Array(16);
  reader.copyBytesInto(pdbGUIDBytes, 4, 16);
  return new ExternalPDBFileReference(pdbPath, new GUID(pdbGUIDBytes));
}

export class ExternalPDBFileReference {
  pdbGUID;
  pdbPath;

  constructor(pdbPath, pdbGUID) {
    this.pdbGUID = pdbGUID;
    this.pdbPath = pdbPath;
  }
}

class RVAReaderSlow extends ReaderBase {
  #reader;
  #sections;
  #baseRVA;
  #size;

  constructor(reader, sections, baseRVA, size) {
    super();
    this.#reader = reader;
    this.#sections = sections;
    this.#baseRVA = baseRVA;
    this.#size = size;
  }

  get size() {
    return this.#size;
  }

  u32(offset) {
    let rva = this.#baseRVA + offset;
    // TODO(strager): Zero-pad if out of bounds.
    return this.#reader.u32(
      rva - section.virtualAddress + section.dataFileOffset
    );
  }
}
