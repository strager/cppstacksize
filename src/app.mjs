import { BlobLoader, LoaderReader, withLoadScopeAsync } from "./loader.mjs";
import { CapturingLogger, fallbackLogger } from "./logger.mjs";
import {
  CodeViewTypeTable,
  CodeViewTypesInSeparatePDBFileError,
  findAllCodeViewFunctions2Async,
  findAllCodeViewFunctionsAsync,
  getCodeViewFunctionLocalsAsync,
  parseCodeViewTypesAsync,
  parseCodeViewTypesWithoutHeaderAsync,
} from "./codeview.mjs";
import {
  PDBMagicMismatchError,
  parsePDBDBIStreamAsync,
  parsePDBHeaderAsync,
  parsePDBStreamDirectoryAsync,
  parsePDBTPIStreamHeaderAsync,
} from "./pdb.mjs";
import { findCOFFSectionsByNameAsync } from "./coff.mjs";

let funcs = [];
let typeTable = null;
let typeTableError = null;

async function onUploadFileAsync(file) {
  funcs.length = 0;
  typeTable = null;
  typeTableError = null;
  clearFunctionDetailsAsync();
  showFunctionSelection(null);

  let loader = new BlobLoader(file);
  let reader = new LoaderReader(loader);

  try {
    await parsePDBAsync(reader);
  } catch (e) {
    if (e instanceof PDBMagicMismatchError) {
      await parseCOFFAsync(reader);
    } else {
      throw e;
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
    if (typeTable !== null) {
      let funcLogger = new CapturingLogger(logger);
      td.textContent = `${await func.getCallerStackSizeAsync(
        typeTable,
        funcLogger
      )}`;
      if (funcLogger.didLogMessage) {
        td.title = funcLogger.getLoggedMessagesStringForToolTip();
      }
    } else if (typeTableError !== null) {
      td.title = typeTableError.toString();
    }
    tr.appendChild(td);
    functionTableTbodyElement.appendChild(tr);
  }
}

async function parsePDBAsync(reader) {
  let superBlock = await parsePDBHeaderAsync(reader, logger);
  let parsedStreams = await parsePDBStreamDirectoryAsync(
    reader,
    superBlock,
    logger
  );
  let dbi = await parsePDBDBIStreamAsync(parsedStreams[3], logger);
  for (let module of dbi.modules) {
    let codeViewStream = parsedStreams[module.debugInfoStreamIndex];
    for (let func of await findAllCodeViewFunctions2Async(
      codeViewStream,
      logger
    )) {
      funcs.push(func);
    }
  }
  let tpiHeader = await parsePDBTPIStreamHeaderAsync(parsedStreams[2], logger);
  // TODO[start-type-id]
  typeTable = await parseCodeViewTypesWithoutHeaderAsync(
    tpiHeader.typeReader,
    logger
  );
}

async function parseCOFFAsync(reader) {
  for (let sectionReader of await findCOFFSectionsByNameAsync(
    reader,
    ".debug$T"
  )) {
    try {
      typeTable = await parseCodeViewTypesAsync(sectionReader);
    } catch (e) {
      if (e instanceof CodeViewTypesInSeparatePDBFileError) {
        // TODO(strager): See if the user attached a .pdb file too.
        typeTableError = e;
        console.warn(e);
      } else {
        throw e;
      }
    }
    break;
  }

  for (let sectionReader of await findCOFFSectionsByNameAsync(
    reader,
    ".debug$S"
  )) {
    for (let func of await findAllCodeViewFunctionsAsync(
      sectionReader,
      logger
    )) {
      funcs.push(func);
    }
  }
}

async function clearFunctionDetailsAsync() {
  stackFrameTableTbodyElement.innerHTML = "";
}

async function showFunctionDetailsAsync(func) {
  clearFunctionDetailsAsync();
  let locals = await getCodeViewFunctionLocalsAsync(
    func.reader,
    func.byteOffset,
    logger
  );
  for (let local of locals) {
    let tr = document.createElement("tr");
    let td = document.createElement("td");
    td.textContent = local.name;
    tr.appendChild(td);
    td = document.createElement("td");
    let localLogger = new CapturingLogger(logger);
    td.textContent = `${await local.getByteSizeAsync(typeTable, localLogger)}`;
    if (localLogger.didLogMessage) {
      td.title = localLogger.getLoggedMessagesStringForToolTip();
    }
    tr.appendChild(td);
    stackFrameTableTbodyElement.appendChild(tr);
  }
}

let functionTableElement = document.getElementById("function-table");
let functionTableTbodyElement = functionTableElement.querySelector("tbody");

let functionTableSelectionElement = document.createElement("div");
functionTableSelectionElement.classList.add("row-selection");
let functionTableSelectionScrollBoxElement =
  document.getElementById("list-section");
functionTableSelectionScrollBoxElement.appendChild(
  functionTableSelectionElement
);

functionTableTbodyElement.addEventListener("click", async (event) => {
  let funcRowElement = event.target.closest("tr.func");
  if (funcRowElement === null) {
    return;
  }
  showFunctionSelection(funcRowElement);
  let func = funcs[funcRowElement.dataset.funcIndex];
  await showFunctionDetailsAsync(func);
});

function showFunctionSelection(selectedRowElement) {
  if (selectedRowElement === null) {
    functionTableSelectionElement.style.display = "none";
  } else {
    let rowRect = selectedRowElement.getBoundingClientRect();
    let scrollBoxRect =
      functionTableSelectionScrollBoxElement.getBoundingClientRect();
    functionTableSelectionElement.style.left = `${
      rowRect.left -
      scrollBoxRect.left +
      functionTableSelectionScrollBoxElement.scrollLeft
    }px`;
    functionTableSelectionElement.style.top = `${
      rowRect.top -
      scrollBoxRect.top +
      functionTableSelectionScrollBoxElement.scrollTop
    }px`;
    functionTableSelectionElement.style.width = `${rowRect.width}px`;
    functionTableSelectionElement.style.height = `${rowRect.height}px`;
    functionTableSelectionElement.style.display = "block";
  }
}

let stackFrameTableElement = document.getElementById("stack-frame-table");
let stackFrameTableTbodyElement = stackFrameTableElement.querySelector("tbody");

let filePickerElement = document.getElementById("file-picker");
filePickerElement.addEventListener("change", (_event) => {
  // TODO(strager): Support multiple files.
  onUploadFileAsync(filePickerElement.files[0]);
});

let logTableTbodyElement = document.querySelector("#log-table tbody");
let logCounterElement = document.getElementById("log-counter");
let logButtonElement = document.getElementById("log-button");
let logger = new (class HTMLTableLogger {
  #logCount = 0;

  log(message, location) {
    this.#logCount += 1;

    fallbackLogger.log(message, location);

    let tr = document.createElement("tr");
    let td = document.createElement("td");
    td.textContent = this.#logCount - 1;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent = `0x${location.fileOffset.toString(16)}`;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent =
      location.streamIndex === null ? "" : `#${location.streamIndex}`;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent =
      location.streamOffset === null
        ? ""
        : `0x${location.streamOffset.toString(16)}`;
    tr.appendChild(td);
    td = document.createElement("td");
    td.textContent = message;
    tr.appendChild(td);
    logTableTbodyElement.appendChild(tr);

    logCounterElement.textContent = `${this.#logCount}`;
  }
})();
logButtonElement.addEventListener("click", (_event) => {
  document.body.classList.toggle("show-logs");
});
