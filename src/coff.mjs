import { SubFileReader } from "./reader.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

export async function findCOFFSectionsByNameAsync(reader, sectionName) {
  let foundSections = [];
  for (let section of await getCOFFSectionsAsync(reader)) {
    if (section.name === sectionName) {
      foundSections.push(
        new SubFileReader(reader, section.dataFileOffset, section.dataSize)
      );
    }
  }
  return foundSections;
}

export function getCOFFSectionsAsync(reader) {
  return withLoadScopeAsync(() => {
    let magic = reader.u16(0);
    if (magic != 0x8664) {
      throw new COFFParseError(`unexpected magic: 0x${magic.toString(16)}`);
    }
    let sectionCount = reader.u16(2);
    let optionalHeaderSize = reader.u16(16);
    let sectionTableOffset = 20 + optionalHeaderSize;
    let sections = [];
    for (let sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
      sections.push(
        parseCOFFSection(reader, sectionTableOffset + sectionIndex * 40)
      );
    }
    return sections;
  });
}

function parseCOFFSection(reader, offset) {
  return {
    name: reader.fixedWidthString(offset, 8),
    dataSize: reader.u32(offset + 16),
    dataFileOffset: reader.u32(offset + 20),
  };
}
