import { SubFileReader } from "./reader.mjs";
import { getCOFFSectionsAsync } from "./coff.mjs";
import { withLoadScopeAsync } from "./loader.mjs";

export function getPESectionsAsync(reader) {
  return withLoadScopeAsync(async () => {
    let peSignatureOffset = reader.u32(0x3c);
    // TODO(strager): Check PE signature.
    let coffFileHeaderOffset = peSignatureOffset + 4;
    return await getCOFFSectionsAsync(
      new SubFileReader(reader, coffFileHeaderOffset)
    );
  });
}
