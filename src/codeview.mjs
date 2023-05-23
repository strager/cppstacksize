import { SubFileReader } from "./reader.mjs";
import { alignUp } from "./util.mjs";
import { fallbackLogger } from "./logger.mjs";
import { withLoadScopeAsync } from "./loader.mjs";
import {
  CV_CALL_NEAR_C,
  CV_PTR_64,
  CV_SIGNATURE_C13,
  DEBUG_S_SYMBOLS,
  LF_FUNC_ID,
  LF_MODIFIER,
  LF_POINTER,
  LF_PROCEDURE,
  LF_TYPESERVER2,
  S_END,
  S_FRAMEPROC,
  S_GPROC32,
  S_GPROC32_ID,
  S_PROC_ID_END,
  S_REGREL32,
  specialTypeNameMap,
  specialTypeSizeMap,
} from "./codeview-constants.mjs";
import { GUID } from "./guid.mjs";

export class CodeViewTypesInSeparatePDBFileError extends Error {
  pdbGUID;
  pdbPath;

  constructor(pdbPath, pdbGUID) {
    super(
      "CodeView types cannot be loaded because they are in a separate PDB file"
    );
    this.pdbGUID = pdbGUID;
    this.pdbPath = pdbPath;
  }
}

export class CodeViewTypeTable {
  #typeEntryOffsets = [];
  #reader;
  #startTypeID;

  constructor(reader, startTypeID) {
    this.#reader = reader;
    this.#startTypeID = startTypeID;
  }

  get _reader() {
    return this.#reader;
  }

  _addTypeEntryAtOffset(offset) {
    this.#typeEntryOffsets.push(offset);
  }

  _getOffsetOfTypeEntry(typeID) {
    let index = typeID - this.#startTypeID;
    if (index < 0 || index >= this.#typeEntryOffsets.length) {
      return null;
    }
    return this.#typeEntryOffsets[index];
  }

  _getReaderForTypeEntry(typeID) {
    let offset = this._getOffsetOfTypeEntry(typeID);
    if (offset === null) {
      return null;
    }
    let size = this.#reader.u16(offset);
    return new SubFileReader(this.#reader, offset, size + 2);
  }
}

export async function parseCodeViewTypesAsync(reader, logger = fallbackLogger) {
  return await withLoadScopeAsync(async () => {
    let signature = reader.u32(0);
    if (signature !== CV_SIGNATURE_C13) {
      throw new UnsupportedCodeViewError(
        `unrecognized CodeView signature: 0x${signature.toString(16)}`
      );
    }
    return parseCodeViewTypesWithoutHeader(reader, 4, logger);
  });
}

export function parseCodeViewTypesWithoutHeaderAsync(
  reader,
  logger = fallbackLogger
) {
  return withLoadScopeAsync(async () => {
    return parseCodeViewTypesWithoutHeader(reader, 0, logger);
  });
}

export async function parseCodeViewTypesWithoutHeader(
  reader,
  offset,
  logger = fallbackLogger
) {
  // FIXME[start-type-id]: This should be a parameter. PDB can overwrite the
  // initial type ID.
  let startTypeID = 0x1000;
  let table = new CodeViewTypeTable(reader, startTypeID);
  while (offset < reader.size) {
    let recordSize = reader.u16(offset + 0);
    let recordType = reader.u16(offset + 2);
    if (recordType === LF_TYPESERVER2) {
      let pdbPath = reader.utf8CString(offset + 24);
      let pdbGUIDBytes = new Uint8Array(16);
      reader.copyBytesInto(pdbGUIDBytes, 8, 16);
      throw new CodeViewTypesInSeparatePDBFileError(
        pdbPath,
        new GUID(pdbGUIDBytes)
      );
    }
    table._addTypeEntryAtOffset(offset);
    offset += recordSize + 2;
  }
  return table;
}

export async function findAllCodeViewFunctionsAsync(
  reader,
  logger = fallbackLogger
) {
  return await withLoadScopeAsync(async () => {
    let signature = reader.u32(0);
    if (signature !== CV_SIGNATURE_C13) {
      throw new UnsupportedCodeViewError(
        `unrecognized CodeView signature: 0x${signature.toString(16)}`
      );
    }

    let functions = [];
    let offset = 4;
    while (offset < reader.size) {
      offset = alignUp(offset, 4);
      let subsectionType = reader.u32(offset);
      offset += 4;
      let subsectionSize = reader.u32(offset);
      offset += 4;
      switch (subsectionType) {
        case DEBUG_S_SYMBOLS:
          await findAllCodeViewFunctionsInSubsectionAsync(
            new SubFileReader(reader, offset, subsectionSize),
            logger,
            functions
          );
          break;
        default:
          // Ignore.
          break;
      }
      offset += subsectionSize;
    }
    return functions;
  });
}

// FIXME(strager): Why do we need findAllCodeViewFunctionsAsync with .obj but
// findAllCodeViewFunctions2Async with .pdb?
export async function findAllCodeViewFunctions2Async(
  reader,
  logger = fallbackLogger
) {
  return await withLoadScopeAsync(async () => {
    let signature = reader.u32(0);
    if (signature !== CV_SIGNATURE_C13) {
      throw new UnsupportedCodeViewError(
        `unrecognized CodeView signature: 0x${signature.toString(16)}`
      );
    }

    let functions = [];
    await findAllCodeViewFunctionsInSubsectionAsync(
      new SubFileReader(reader, 4, reader.size - 4),
      logger,
      functions
    );
    return functions;
  });
}

async function findAllCodeViewFunctionsInSubsectionAsync(
  reader,
  logger,
  outFunctions
) {
  let offset = 0;
  while (offset < reader.size) {
    let recordSize = reader.u16(offset + 0);
    if (recordSize < 2) {
      logger.log(
        `record has unusual size: ${recordSize}`,
        reader.locate(offset + 0)
      );
      break;
    }
    let recordType = reader.u16(offset + 2);
    switch (recordType) {
      case S_GPROC32:
      case S_GPROC32_ID: {
        let func = new CodeViewFunction(
          reader.utf8CString(offset + 39),
          reader,
          /*typeID=*/ reader.u32(offset + 28),
          offset,
          /*hasFuncIDType=*/ recordType === S_GPROC32_ID
        );
        outFunctions.push(func);
        break;
      }

      case S_FRAMEPROC: {
        if (outFunctions.length === 0) {
          logger.log(
            `found S_FRAMEPROC with no corresponding S_GPROC32`,
            reader.locate(offset + 0)
          );
          break;
        }
        let func = outFunctions[outFunctions.length - 1];
        func.selfStackSize = reader.u32(offset + 4);
        break;
      }

      default:
        break;
    }

    offset += recordSize + 2;
  }
}

export class CodeViewFunction {
  name;
  reader;
  byteOffset;
  selfStackSize;
  #hasFuncIDType;
  #typeID;

  constructor(name, reader, typeID, byteOffset, hasFuncIDType) {
    this.name = name;
    this.reader = reader;
    this.#hasFuncIDType = hasFuncIDType;
    this.#typeID = typeID;
    this.byteOffset = byteOffset;
    this.selfStackSize = -1;
  }

  async getCallerStackSizeAsync(
    typeTable,
    typeIndexTable = typeTable,
    logger = fallbackLogger
  ) {
    return withLoadScopeAsync(() => {
      let reader = typeTable._reader;
      let typeID = this.#typeID;
      if (this.#hasFuncIDType) {
        let indexReader = typeIndexTable._reader;
        let funcIDTypeOffset = typeIndexTable._getOffsetOfTypeEntry(typeID);
        // TODO(strager): Check size.
        let funcIDTypeRecordTypeOffset = funcIDTypeOffset + 2;
        let funcIDTypeRecordType = indexReader.u16(funcIDTypeRecordTypeOffset);
        switch (funcIDTypeRecordType) {
          case LF_FUNC_ID:
            typeID = indexReader.u32(funcIDTypeOffset + 8);
            break;
          default:
            logger.log(
              `unrecognized function ID record type: 0x${funcIDTypeRecordType.toString(
                16
              )}`,
              reader.locate(funcIDTypeRecordTypeOffset)
            );
            return -1;
        }
      }

      let funcTypeOffset = typeTable._getOffsetOfTypeEntry(typeID);
      let funcTypeRecordTypeOffset = funcTypeOffset + 2;
      // TODO(strager): Check size.
      let funcTypeRecordType = reader.u16(funcTypeRecordTypeOffset);
      switch (funcTypeRecordType) {
        case LF_PROCEDURE: {
          let callingConventionOffset = funcTypeOffset + 8;
          let callingConvention = reader.u16(callingConventionOffset);
          switch (callingConvention) {
            case CV_CALL_NEAR_C: {
              let parameterCount = reader.u16(funcTypeOffset + 10);
              return Math.max(parameterCount, 4) * 8;
            }

            default:
              logger.log(
                `unrecognized function calling convention: 0x${callingConvention.toString(
                  16
                )}`,
                reader.locate(callingConventionOffset)
              );
              break;
          }
          break;
        }

        default:
          logger.log(
            `unrecognized function type record type: 0x${funcTypeRecordType.toString(
              16
            )}`,
            reader.locate(funcTypeRecordTypeOffset)
          );
          break;
      }
      return -1;
    });
  }
}

export class CodeViewType {
  byteSize;
  name;

  constructor(byteSize, name) {
    this.byteSize = byteSize;
    this.name = name;
  }
}

// A local variable or parameter in a function.
export class CodeViewFunctionLocal {
  name;
  spOffset;
  typeID;
  #reader;
  #recordOffset;

  constructor(name, reader, offset) {
    this.name = name;
    this.spOffset = null;
    this.typeID = null;
    this.#reader = reader;
    this.#recordOffset = offset;
  }

  async getTypeAsync(typeTable, logger = fallbackLogger) {
    let type = await getCodeViewTypeAsync(this.typeID, typeTable, logger);
    if (type === null) {
      logger.log(
        `local has unknown type: 0x${this.typeID.toString(16)}`,
        this.#reader.locate(this.#recordOffset)
      );
      return null;
    }
    return type;
  }
}

export async function getCodeViewTypeAsync(
  typeID,
  typeTable,
  logger = fallbackLogger
) {
  return withLoadScopeAsync(() => {
    return getCodeViewType(typeID, typeTable, logger);
  });
}

export function getCodeViewType(typeID, typeTable, logger) {
  let maybeSize = specialTypeSizeMap[typeID];
  if (maybeSize !== undefined) {
    if (typeof maybeSize === "string") {
      logger.log(
        `unsupported special type: 0x${typeID.toString(16)} (${maybeSize})`,
        null
      );
      return null;
    }
    return new CodeViewType(maybeSize, specialTypeNameMap[typeID]);
  }

  let typeEntryReader = typeTable._getReaderForTypeEntry(typeID);
  let typeEntryType = typeEntryReader.u16(2);
  switch (typeEntryType) {
    case LF_POINTER: {
      let pointeeTypeID = typeEntryReader.u32(4);
      let pointerAttributes = typeEntryReader.u32(8);
      let isConst = (pointerAttributes & (1 << 10)) !== 0;
      let pointerType = pointerAttributes & 0x1f;

      let byteSize;
      switch (pointerType) {
        case CV_PTR_64:
          byteSize = 8;
          break;

        default:
          logger.log(
            `unsupported pointer type: 0x${pointerType.toString(16)}`,
            typeEntryReader.locate(0)
          );
          break;
      }

      let pointeeType = getCodeViewType(pointeeTypeID, typeTable, logger);
      let name = pointeeType === null ? "<unknown>" : pointeeType.name;
      if (name.endsWith("*")) {
        name += "*";
      } else {
        name += " *";
      }
      if (isConst) {
        name += "const";
      }
      return new CodeViewType(byteSize, name);
    }

    case LF_MODIFIER: {
      let modifiedTypeID = typeEntryReader.u32(4);
      let modifiers = typeEntryReader.u16(8);
      let isConst = (modifiers & (1 << 0)) !== 0;
      let isVolatile = (modifiers & (1 << 1)) !== 0;
      let isUnaligned = (modifiers & (1 << 2)) !== 0;

      let modifiedType = getCodeViewType(modifiedTypeID, typeTable, logger);
      if (modifiedType === null) {
        return null;
      }
      let name = modifiedType.name;
      if (isVolatile) {
        name = "volatile " + name;
      }
      if (isConst) {
        name = "const " + name;
      }
      // TODO(strager): isUnaligned
      return new CodeViewType(modifiedType.byteSize, name);
    }

    default:
      logger.log(
        `unknown entry kind 0x${typeEntryType.toString(
          16
        )} for type ID 0x${typeID.toString(16)}`,
        typeEntryReader.locate(0)
      );
      return null;
  }
}

export async function getCodeViewFunctionLocalsAsync(
  reader,
  offset,
  logger = fallbackLogger
) {
  return await withLoadScopeAsync(async () => {
    let locals = [];
    while (offset < reader.size) {
      let recordSize = reader.u16(offset + 0);
      let recordType = reader.u16(offset + 2);
      switch (recordType) {
        case S_REGREL32: {
          let local = new CodeViewFunctionLocal(
            reader.utf8CString(offset + 14),
            reader,
            offset
          );
          // TODO(strager): Verify that the register is RSP.
          local.spOffset = reader.u32(offset + 4);
          local.typeID = reader.u32(offset + 8);
          locals.push(local);
          break;
        }
        case S_END:
        case S_PROC_ID_END:
          offset = reader.size;
          break;
        default:
          break;
      }

      offset += recordSize + 2;
    }
    return locals;
  });
}
