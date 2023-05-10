import { SubFileReader } from "./reader.mjs";

export function findCOFFSectionsByName(file, sectionName) {
  let magic = file.u16(0);
  if (magic != 0x8664) {
    throw new COFFParseError(`unexpected magic: 0x${magic.toString(16)}`);
  }
  let sectionCount = file.u16(2);
  let optionalHeaderSize = file.u16(16);
  if (optionalHeaderSize != 0) {
    throw new COFFParseError(
      `unexpected optional header size: 0x${optionalHeaderSize.toString(16)}`
    );
  }

  let foundSections = [];
  for (let sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
    let section = parseCOFFSection(file, 20 + sectionIndex * 40);
    if (section.name === sectionName) {
      foundSections.push(
        new SubFileReader(file, section.dataFileOffset, section.dataSize)
      );
    }
  }
  return foundSections;
}

function parseCOFFSection(file, offset) {
  return {
    name: file.fixedWidthString(offset, 8),
    dataSize: file.u32(offset + 16),
    dataFileOffset: file.u32(offset + 20),
  };
}
