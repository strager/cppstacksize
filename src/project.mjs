import {
  CodeViewTypesInSeparatePDBFileError,
  findAllCodeViewFunctionsAsync,
  findAllCodeViewFunctions2Async,
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
import { fallbackLogger } from "./logger.mjs";
import { findCOFFSectionsByNameAsync } from "./coff.mjs";

export class Project {
  #files = [];

  addFile(name, reader) {
    this.#files.push(new ProjectFile(name, reader));
  }

  async getTypeTableAsync(logger = fallbackLogger) {
    for (let file of this.#files) {
      await file.tryLoadPDBGenericHeadersAsync(logger);
      if (file.pdbStreams !== null) {
        if (file.pdbTPIHeader === null) {
          file.pdbTPIHeader = await parsePDBTPIStreamHeaderAsync(
            file.pdbStreams[2],
            logger
          );
        }
        // TODO[start-type-id]
        return await parseCodeViewTypesWithoutHeaderAsync(
          file.pdbTPIHeader.typeReader,
          logger
        );
      }

      for (let sectionReader of await findCOFFSectionsByNameAsync(
        file.reader,
        ".debug$T"
      )) {
        try {
          return await parseCodeViewTypesAsync(sectionReader);
        } catch (e) {
          if (e instanceof CodeViewTypesInSeparatePDBFileError) {
            // Ignore.
          } else {
            throw e;
          }
        }
      }
    }
    return null;
  }

  async getTypeIndexTableAsync(logger = fallbackLogger) {
    for (let file of this.#files) {
      await file.tryLoadPDBGenericHeadersAsync(logger);
      if (file.pdbStreams !== null) {
        if (file.pdbIPIHeader === null) {
          file.pdbIPIHeader = await parsePDBTPIStreamHeaderAsync(
            file.pdbStreams[4],
            logger
          );
        }
        // TODO[start-type-id]
        return await parseCodeViewTypesWithoutHeaderAsync(
          file.pdbIPIHeader.typeReader,
          logger
        );
      }

      for (let sectionReader of await findCOFFSectionsByNameAsync(
        file.reader,
        ".debug$T"
      )) {
        try {
          return await parseCodeViewTypesAsync(sectionReader);
        } catch (e) {
          if (e instanceof CodeViewTypesInSeparatePDBFileError) {
            // Ignore.
          } else {
            throw e;
          }
        }
      }
    }
    return null;
  }

  async getAllFunctionsAsync(logger = fallbackLogger) {
    let funcs = [];
    for (let file of this.#files) {
      await file.tryLoadPDBGenericHeadersAsync(logger);
      if (file.pdbStreams !== null) {
        if (file.pdbDBI === null) {
          file.pdbDBI = await parsePDBDBIStreamAsync(
            file.pdbStreams[3],
            logger
          );
        }
        for (let module of file.pdbDBI.modules) {
          let codeViewStream = file.pdbStreams[module.debugInfoStreamIndex];
          funcs.push(
            ...(await findAllCodeViewFunctions2Async(codeViewStream, logger))
          );
        }
      } else {
        for (let sectionReader of await findCOFFSectionsByNameAsync(
          file.reader,
          ".debug$S"
        )) {
          funcs.push(...(await findAllCodeViewFunctionsAsync(sectionReader)));
        }
      }
    }
    return funcs;
  }
}

class ProjectFile {
  name;
  reader;

  pdbSuperBlock = null;
  pdbStreams = null;
  pdbDBI = null;
  pdbTPIHeader = null;
  pdbIPIHeader = null;

  constructor(name, reader) {
    this.name = name;
    this.reader = reader;
  }

  async tryLoadPDBGenericHeadersAsync(logger) {
    try {
      if (this.pdbSuperBlock === null) {
        this.pdbSuperBlock = await parsePDBHeaderAsync(this.reader, logger);
      }
      if (this.pdbStreams === null) {
        this.pdbStreams = await parsePDBStreamDirectoryAsync(
          this.reader,
          this.pdbSuperBlock,
          logger
        );
      }
    } catch (e) {
      if (e instanceof PDBMagicMismatchError) {
        return;
      }
      throw e;
    }
  }
}
