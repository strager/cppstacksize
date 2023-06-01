import { SubFileReader } from "./reader.mjs";
import { parsePEFileAsync } from "./pe.mjs";
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

export async function getCOFFSectionsAsync(reader) {
  let pe = await parsePEFileAsync(reader);
  return pe.sections;
}
