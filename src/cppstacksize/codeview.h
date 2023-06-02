#ifndef CPPSTACKSIZE_CODEVIEW_H
#define CPPSTACKSIZE_CODEVIEW_H

#include <cppstacksize/base.h>
#include <cppstacksize/codeview-constants.h>
#include <cppstacksize/logger.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/util.h>
#include <exception>
#include <string>
#include <vector>

// TODO(strager): Switch to <format>.
#include <fmt/format.h>

namespace cppstacksize {
using namespace std::literals::string_view_literals;

class Unsupported_CodeView_Error : public std::exception {};

template <class Reader>
class CodeView_Type_Table {};

struct CodeView_Type {
  U64 byte_size;
  std::u8string name;
};

template <class Reader>
struct CodeView_Function {
  std::u8string name;
  Sub_File_Reader<Reader> reader;
  U64 byte_offset;
  U32 self_stack_size = static_cast<U32>(-1);

  // Section number. Almost certainly refers to a .text section.
  //
  // TODO[coff-relocations]: This is probably wrong because we don't perform
  // relocations.
  U32 code_section_index = static_cast<U32>(-1);

  // Byte offset within the section described by codeSectionIndex.
  //
  // TODO[coff-relocations]: This is probably wrong because we don't perform
  // relocations.
  U32 code_offset = static_cast<U32>(-1);
  // Number of bytes for this function's machine code.
  U32 code_size = static_cast<U32>(-1);

  // Associated PE or COFF file, if any.
  PE_File<Reader>* pe_file = nullptr;

  bool has_func_id_type;
  U32 type_id;
};

template <class Reader>
std::vector<CodeView_Function<Reader>> find_all_codeview_functions(
    Reader* reader) {
  std::vector<CodeView_Function<Reader>> out_functions;
  find_all_codeview_functions(reader, out_functions);
  return out_functions;
}

template <class Reader>
void find_all_codeview_functions(
    Reader* reader, std::vector<CodeView_Function<Reader>>& out_functions) {
  find_all_codeview_functions(reader, out_functions, fallback_logger);
}

template <class Reader>
void find_all_codeview_functions(
    Reader* reader, std::vector<CodeView_Function<Reader>>& out_functions,
    Logger& logger) {
  U32 signature = reader->u32(0);
  if (signature != CV_SIGNATURE_C13) {
    throw Unsupported_CodeView_Error();
  }

  U64 offset = 4;
  for (;;) {
    offset = align_up(offset, 4);
    if (offset >= reader->size()) {
      break;
    }
    U32 subsection_type = reader->u32(offset);
    offset += 4;
    U32 subsection_size = reader->u32(offset);
    offset += 4;
    switch (subsection_type) {
      case DEBUG_S_SYMBOLS:
        find_all_codeview_functions_in_subsection<Reader>(
            Sub_File_Reader(reader, offset, subsection_size), out_functions,
            logger);
        break;
      default:
        // Ignore.
        break;
    }
    offset += subsection_size;
  }
}

template <class Reader>
void find_all_codeview_functions_in_subsection(
    Sub_File_Reader<Reader> reader,
    std::vector<CodeView_Function<Reader>>& out_functions, Logger& logger) {
  U64 offset = 0;

  while (offset < reader.size()) {
    U64 record_size = reader.u16(offset + 0);
    if (record_size < 2) {
      logger.log(fmt::format("record has unusual size: {}", record_size),
                 reader.locate(offset + 0));
      break;
    }
    U16 record_type = reader.u16(offset + 2);
    switch (record_type) {
      case S_GPROC32:
      case S_GPROC32_ID: {
        CodeView_Function<Reader> func{
            .name = reader.utf_8_c_string(offset + 39),
            .reader = reader,
            .byte_offset = offset,
            .code_section_index = U32{reader.u16(offset + 36)} - 1,
            .code_offset = reader.u32(offset + 32),
            .code_size = reader.u32(offset + 16),
            .has_func_id_type = record_type == S_GPROC32_ID,
            .type_id = reader.u32(offset + 28),
        };
        out_functions.push_back(func);
        break;
      }

      case S_FRAMEPROC: {
        // FIXME(strager): This should instead check if the last function added
        // was added by us.
        if (out_functions.empty()) {
          logger.log("found S_FRAMEPROC with no corresponding S_GPROC32"sv,
                     reader.locate(offset + 0));
          break;
        }
        CodeView_Function<Reader>& func =
            out_functions[out_functions.size() - 1];
        func.self_stack_size = reader.u32(offset + 4);
        break;
      }

      default:
        break;
    }

    offset += record_size + 2;
  }
}

template <class Reader>
struct CodeView_Function_Local {
  std::u8string name;
  U32 sp_offset;
  U32 type_id;
  Sub_File_Reader<Reader> reader;
  U64 record_offset;

  std::optional<CodeView_Type> get_type(
      CodeView_Type_Table<Reader>* type_table) {
    return this->get_type(type_table, fallback_logger);
  }

  std::optional<CodeView_Type> get_type(CodeView_Type_Table<Reader>* type_table,
                                        Logger& logger) {
    std::optional<CodeView_Type> type =
        get_codeview_type(this->type_id, type_table, logger);
    if (!type.has_value()) {
      logger.log(fmt::format("local has unknown type: 0x{:x}", this->type_id),
                 this->reader.locate(this->record_offset));
      return std::nullopt;
    }
    return type;
  }
};

template <class Reader>
std::optional<CodeView_Type> get_codeview_type(
    U32 type_id, CodeView_Type_Table<Reader>* reader) {
  return get_codeview_type(type_id, reader, fallback_logger);
}

template <class Reader>
std::optional<CodeView_Type> get_codeview_type(U32 type_id,
                                               CodeView_Type_Table<Reader>*,
                                               Logger&) {
  if (type_id < special_type_size_map.size()) {
    U8 maybe_size = special_type_size_map[type_id];
    if (maybe_size != 0xff) {
      return CodeView_Type{
          .byte_size = maybe_size,
          .name = std::u8string(special_type_name_map[type_id]),
      };
    }
  }
  return std::nullopt;
}

template <class Reader>
std::vector<CodeView_Function_Local<Reader>> get_codeview_function_locals(
    Sub_File_Reader<Reader> reader, U64 offset) {
  std::vector<CodeView_Function_Local<Reader>> out_locals;
  get_codeview_function_locals<Reader>(reader, offset, out_locals,
                                       fallback_logger);
  return out_locals;
}

template <class Reader>
void get_codeview_function_locals(
    Sub_File_Reader<Reader> reader, U64 offset,
    std::vector<CodeView_Function_Local<Reader>>& out_locals, Logger&) {
  U32 depth = 1;
  while (offset < reader.size()) {
    U64 record_size = reader.u16(offset + 0);
    U16 record_type = reader.u16(offset + 2);
    switch (record_type) {
      case S_REGREL32: {
        CodeView_Function_Local<Reader> local{
            .name = reader.utf_8_c_string(offset + 14),
            // TODO(strager): Verify that the register is RSP.
            .sp_offset = reader.u32(offset + 4),
            .type_id = reader.u32(offset + 8),
            .reader = reader,
            .record_offset = offset,
        };
        out_locals.push_back(local);
        break;
      }
      case S_BLOCK32:
        depth += 1;
        break;
      case S_END:
        depth -= 1;
        if (depth == 0) {
          goto done;
        }
        break;
      case S_PROC_ID_END:
        goto done;
      default:
        break;
    }

    offset += record_size + 2;
  }
done:;
}
}

#endif
