import { GUID } from "./guid.mjs";
import { ReaderBase, SubFileReader } from "./reader.mjs";
import { getCOFFSectionsAsync } from "./coff.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

// Documentation:
// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

let IMAGE_DEBUG_TYPE_CODEVIEW = 2;

// A Windows PE (.dll or .exe) or COFF (.obj) file.
class PEFile {
  reader;
  sections = [];
  debugDirectory = [];

  constructor(reader) {
    this.reader = reader;
  }
}

export function parsePEFileAsync(reader) {
  return withLoadScopeAsync(async () => {
    let pe = new PEFile(reader);
    pe.sections = await getPESectionsAsync(reader);
    pe.debugDirectory = await getPEDebugDirectoryAsync(reader);
    return pe;
  });
}

function getPESectionsAsync(reader) {
  return withLoadScopeAsync(async () => {
    let peSignatureOffset = reader.u32(0x3c);
    // TODO(strager): Check PE signature.
    let coffFileHeaderOffset = peSignatureOffset + 4;
    return await getCOFFSectionsAsync(
      new SubFileReader(reader, coffFileHeaderOffset)
    );
  });
}

function getPEDebugDirectoryAsync(reader) {
  return withLoadScopeAsync(async () => {
    let peSignatureOffset = reader.u32(0x3c);
    // TODO(strager): Check PE signature.
    let coffHeaderOffset = peSignatureOffset + 4;
    let optionalHeaderSize = reader.u16(coffHeaderOffset + 16);
    if (optionalHeaderSize === 0) {
      throw new PEParseError("missing optional header");
    }

    let optionalHeaderReader = new SubFileReader(
      reader,
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
    let debugDirectoryReader = new RVAReaderSlow(
      reader,
      await getPESectionsAsync(reader),
      debugDataDirectoryRVA,
      debugDataDirectorySize
    );

    let debugDirectory = [];
    let offset = 0;
    while (offset < debugDirectoryReader.size) {
      debugDirectory.push({
        type: debugDirectoryReader.u32(offset + 12),
        dataSize: debugDirectoryReader.u32(offset + 16),
        dataRVA: debugDirectoryReader.u32(offset + 20),
        dataFileOffset: debugDirectoryReader.u32(offset + 24),
      });
      offset += 28;
    }
    return debugDirectory;
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
    let section = this.#sections.find(
      (section) =>
        section.virtualAddress <= rva &&
        rva + 4 < section.virtualAddress + section.virtualSize
    );
    if (section === undefined) {
      throw new RangeError(
        `cannot find section for rva 0x${rva.toString(16)} size=4`
      );
    }
    console.error(
      section,
      this.#baseRVA.toString(16),
      offset.toString(16),
      rva.toString(16)
    );
    // TODO(strager): Zero-pad if out of bounds.
    return this.#reader.u32(
      rva - section.virtualAddress + section.dataFileOffset
    );
  }
}