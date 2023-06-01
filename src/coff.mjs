import { SubFileReader } from "./reader.mjs";
import { parsePEFileAsync } from "./pe.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

export async function findCOFFSectionsByNameAsync(reader, sectionName) {
  let pe = await parsePEFileAsync(reader);
  return pe.findSectionsByName(sectionName);
}

export async function getCOFFSectionsAsync(reader) {
  let pe = await parsePEFileAsync(reader);
  return pe.sections;
}
