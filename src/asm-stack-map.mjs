import loadCPPStackSizeAsync from "../src/build/cppstacksize-wasm.mjs";
let cppStackSizeModule = await loadCPPStackSizeAsync();

let css_create_analyzer_x86_64 = cppStackSizeModule.cwrap(
  "css_create_analyzer_x86_64",
  "number",
  []
);
let css_destroy_analyzer = cppStackSizeModule.cwrap(
  "css_destroy_analyzer",
  null,
  ["number"]
);
let css_set_machine_code = cppStackSizeModule.cwrap(
  "css_set_machine_code",
  null,
  ["number", "array", "number"]
);
let css_get_stack_map = cppStackSizeModule.cwrap("css_get_stack_map", null, [
  "number",
  "number",
  "number",
]);
let malloc = cppStackSizeModule.cwrap("malloc", "number", ["number"]);
let free = cppStackSizeModule.cwrap("free", null, ["number"]);

let cppStackSizeModuleHeap = new DataView(cppStackSizeModule.HEAPU8.buffer);

// CSS_Stack_Map_Entry_Flag
let CSS_STACK_MAP_ENTRY_READ = 1 << 0;
let CSS_STACK_MAP_ENTRY_WRITE = 1 << 1;

let CSS_Stack_Map_Entry = {
  entry_rsp_relative_address: 0, // Int32
  byte_count: 4, // Uint32
  instruction_offset: 8, // Uint32
  flags: 12, // Uint32

  sizeof: 16,
};

let isLittleEndian = (() => {
  let a = new Uint32Array([0x11223344]);
  let dv = new DataView(a.buffer);
  return dv.getUint32(0, /*littleEndian=*/ true) === 0x11223344;
})();

export class Analyzer {
  #analyzerHandle;

  #stackMapEntriesPointer = null;
  #stackMapEntryCount = null;

  #entry = {
    entryRSPRelativeAddress: 0,
    byteCount: 0,
    instructionOffset: 0,
    isRead: false,
    isWrite: false,
  };

  constructor({ arch }) {
    if (arch !== "x86_64") {
      throw new Error("only x86_64 is supported");
    }

    this.#analyzerHandle = css_create_analyzer_x86_64();
  }

  dispose() {
    css_destroy_analyzer(this.#analyzerHandle);
    this.#analyzerHandle = null;
  }

  setMachineCode(code) {
    if (!(code instanceof Uint8Array)) {
      throw new TypeError("machine code must be a Uint8Array");
    }
    css_set_machine_code(this.#analyzerHandle, code, code.byteLength);
    this.#loadStackMap();
  }

  // Returns an object which may be recycled by future calls to this Analyzer's
  // methods.
  getStackMapEntry(index) {
    let pointer =
      this.#stackMapEntriesPointer + index * CSS_Stack_Map_Entry.sizeof;
    let heap = cppStackSizeModuleHeap;
    let entry = this.#entry;
    entry.entryRSPRelativeAddress = heap.getInt32(
      pointer + CSS_Stack_Map_Entry.entry_rsp_relative_address,
      isLittleEndian
    );
    entry.byteCount = heap.getUint32(
      pointer + CSS_Stack_Map_Entry.byte_count,
      isLittleEndian
    );
    entry.instructionOffset = heap.getUint32(
      pointer + CSS_Stack_Map_Entry.instruction_offset,
      isLittleEndian
    );
    let flags = heap.getUint32(
      pointer + CSS_Stack_Map_Entry.flags,
      isLittleEndian
    );
    entry.isRead = (flags & CSS_STACK_MAP_ENTRY_READ) !== 0;
    entry.isWrite = (flags & CSS_STACK_MAP_ENTRY_WRITE) !== 0;
    return entry;
  }

  get stackMapEntryCount() {
    return this.#stackMapEntryCount;
  }

  getStackMapArraySlow() {
    let entries = [];
    for (let i = 0; i < this.stackMapEntryCount; ++i) {
      entries.push({ ...this.getStackMapEntry(i) });
    }
    return entries;
  }

  #loadStackMap() {
    let tempsPointer = malloc(2 * 4);
    try {
      let outEntriesPointer = tempsPointer + 0;
      let outEntryCountPointer = tempsPointer + 4;
      css_get_stack_map(
        this.#analyzerHandle,
        outEntriesPointer,
        outEntryCountPointer
      );
      this.#stackMapEntriesPointer = cppStackSizeModuleHeap.getUint32(
        outEntriesPointer,
        isLittleEndian
      );
      this.#stackMapEntryCount = cppStackSizeModuleHeap.getUint32(
        outEntryCountPointer,
        isLittleEndian
      );
    } finally {
      free(tempsPointer);
    }
  }
}
