import { BlobLoader, LoaderReader, withLoadScopeAsync } from "./loader.mjs";
import { findAllCodeViewFunctionsAsync } from "./codeview.mjs";
import { findCOFFSectionsByNameAsync } from "./coff.mjs";

let funcs = [];

async function onUploadFileAsync(file) {
  let loader = new BlobLoader(file);
  let reader = new LoaderReader(loader);
  for (let sectionReader of await findCOFFSectionsByNameAsync(
    reader,
    ".debug$S"
  )) {
    for (let func of await findAllCodeViewFunctionsAsync(sectionReader)) {
      funcs.push(func);
    }
  }

  for (let func of funcs) {
    let td = document.createElement("td");
    td.textContent = func.name;
    let tr = document.createElement("tr");
    tr.appendChild(td);
    functionTableTbodyElement.appendChild(tr);
  }
}

let functionTableElement = document.getElementById("function-table");
let functionTableTbodyElement = functionTableElement.querySelector("tbody");

let filePickerElement = document.getElementById("file-picker");
filePickerElement.addEventListener("change", (_event) => {
  // TODO(strager): Support multiple files.
  onUploadFileAsync(filePickerElement.files[0]);
});
