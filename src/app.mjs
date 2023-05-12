import { BlobLoader, LoaderReader, withLoadScopeAsync } from "./loader.mjs";
import {
  CodeViewTypeTable,
  findAllCodeViewFunctionsAsync,
  getCodeViewFunctionLocalsAsync,
  parseCodeViewTypesAsync,
} from "./codeview.mjs";
import { findCOFFSectionsByNameAsync } from "./coff.mjs";

let funcs = [];
let typeTable = new CodeViewTypeTable();

async function onUploadFileAsync(file) {
  funcs.length = 0;
  typeTable = new CodeViewTypeTable();
  clearFunctionDetailsAsync();

  let loader = new BlobLoader(file);
  let reader = new LoaderReader(loader);

  for (let sectionReader of await findCOFFSectionsByNameAsync(
    reader,
    ".debug$T"
  )) {
    typeTable = await parseCodeViewTypesAsync(sectionReader);
    break;
  }

  for (let sectionReader of await findCOFFSectionsByNameAsync(
    reader,
    ".debug$S"
  )) {
    for (let func of await findAllCodeViewFunctionsAsync(sectionReader)) {
      funcs.push(func);
    }
  }

  functionTableTbodyElement.innerHTML = "";
  for (let funcIndex = 0; funcIndex < funcs.length; ++funcIndex) {
    let func = funcs[funcIndex];
    let tr = document.createElement("tr");
    tr.classList.add("func");
    tr.dataset.funcIndex = funcIndex;
    let td = document.createElement("td");
    td.textContent = func.name;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent = `${func.selfStackSize}`;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent = `${await func.getCallerStackSizeAsync(typeTable)}`;
    tr.appendChild(td);
    functionTableTbodyElement.appendChild(tr);
  }
}

async function clearFunctionDetailsAsync() {
  stackFrameTableTbodyElement.innerHTML = "";
}

async function showFunctionDetailsAsync(func) {
  clearFunctionDetailsAsync();
  let locals = await getCodeViewFunctionLocalsAsync(
    func.reader,
    func.byteOffset
  );
  for (let local of locals) {
    let tr = document.createElement("tr");
    let td = document.createElement("td");
    td.textContent = local.name;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent = `${local.byteSize}`;
    tr.appendChild(td);
    stackFrameTableTbodyElement.appendChild(tr);
  }
}

let functionTableElement = document.getElementById("function-table");
let functionTableTbodyElement = functionTableElement.querySelector("tbody");

functionTableTbodyElement.addEventListener("click", async (event) => {
  let funcRowElement = event.target.closest("tr.func");
  if (funcRowElement === null) {
    return;
  }
  let func = funcs[funcRowElement.dataset.funcIndex];
  await showFunctionDetailsAsync(func);
});

let stackFrameTableElement = document.getElementById("stack-frame-table");
let stackFrameTableTbodyElement = stackFrameTableElement.querySelector("tbody");

let filePickerElement = document.getElementById("file-picker");
filePickerElement.addEventListener("change", (_event) => {
  // TODO(strager): Support multiple files.
  onUploadFileAsync(filePickerElement.files[0]);
});
