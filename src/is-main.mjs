import url from "node:url";

export function isModuleNodeMain(importMeta) {
  let importPath = url.fileURLToPath(importMeta.url);
  return importPath === process.argv[1];
}
