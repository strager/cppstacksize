import { SubFileReader } from "./reader.mjs";

export async function findCOFFSectionsByNameAsync(reader, sectionName) {
  let magic = reader.u16(0);
  if (magic != 0x8664) {
    throw new COFFParseError(`unexpected magic: 0x${magic.toString(16)}`);
  }
  let sectionCount = reader.u16(2);
  let optionalHeaderSize = reader.u16(16);
  if (optionalHeaderSize != 0) {
    throw new COFFParseError(
      `unexpected optional header size: 0x${optionalHeaderSize.toString(16)}`
    );
  }

  let foundSections = [];
  for (let sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
    let section = parseCOFFSection(reader, 20 + sectionIndex * 40);
    if (section.name === sectionName) {
      foundSections.push(
        new SubFileReader(reader, section.dataFileOffset, section.dataSize)
      );
    }
  }
  return foundSections;
}

function parseCOFFSection(reader, offset) {
  return {
    name: reader.fixedWidthString(offset, 8),
    dataSize: reader.u32(offset + 16),
    dataFileOffset: reader.u32(offset + 20),
  };
}
