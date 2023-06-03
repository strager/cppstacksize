#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/codeview-constants.h>
#include <cppstacksize/logger.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/util.h>
#include <exception>
#include <string>
#include <utility>
#include <vector>

// TODO(strager): Switch to <format>.
#include <fmt/format.h>

namespace cppstacksize {
using namespace std::literals::string_view_literals;

class Unsupported_CodeView_Error : public std::exception {};

// TODO(strager): Use ExternalPDBFileReference.
class CodeView_Types_In_Separate_PDB_File : public std::exception {
 public:
  std::u8string pdb_path;
  GUID pdb_guid;

  explicit CodeView_Types_In_Separate_PDB_File(std::u8string pdb_path,
                                               GUID pdb_guid)
      : pdb_path(std::move(pdb_path)), pdb_guid(pdb_guid) {}

  const char* what() const noexcept override {
    return "CodeView types cannot be loaded because they are in a separate PDB "
           "file";
  }
};

template <class Reader>
class CodeView_Type_Table {
 public:
  explicit CodeView_Type_Table(Reader* reader, U32 start_type_id)
      : reader_(reader), start_type_id_(start_type_id) {}

  void add_type_entry_at_offset_(U64 offset) {
    this->type_entry_offsets_.push_back(offset);
  }

  std::optional<U64> get_offset_of_type_entry_(U32 type_id) {
    U32 index = type_id - this->start_type_id_;
    if (index < 0 || index >= this->type_entry_offsets_.size()) {
      return std::nullopt;
    }
    return this->type_entry_offsets_[index];
  }

  std::optional<Sub_File_Reader<Reader>> get_reader_for_type_entry_(
      U32 type_id) {
    std::optional<U64> offset = this->get_offset_of_type_entry_(type_id);
    if (!offset.has_value()) {
      return std::nullopt;
    }
    U64 size = this->reader_->u16(*offset);
    return Sub_File_Reader<Reader>(this->reader_, *offset, size + 2);
  }

  std::vector<U64> type_entry_offsets_;
  Reader* reader_;
  U32 start_type_id_;
};

template <class Reader>
CodeView_Type_Table<Reader> parse_codeview_types(Reader* reader) {
  return parse_codeview_types(reader, fallback_logger);
}

template <class Reader>
CodeView_Type_Table<Reader> parse_codeview_types(Reader* reader,
                                                 Logger& logger) {
  U32 signature = reader->u32(0);
  if (signature != CV_SIGNATURE_C13) {
    throw Unsupported_CodeView_Error();
  }
  return parse_codeview_types_without_header(reader, 4, logger);
}

template <class Reader>
CodeView_Type_Table<Reader> parse_codeview_types_without_header(
    Reader* reader, Logger& logger) {
  return parse_codeview_types_without_header(reader, 0, logger);
}

template <class Reader>
CodeView_Type_Table<Reader> parse_codeview_types_without_header(Reader* reader,
                                                                U64 offset,
                                                                Logger&) {
  // FIXME[start-type-id]: This should be a parameter. PDB can overwrite the
  // initial type ID.
  U32 start_type_id = 0x1000;
  CodeView_Type_Table<Reader> table(reader, start_type_id);
  while (offset < reader->size()) {
    U64 record_size = reader->u16(offset + 0);
    U16 record_type = reader->u16(offset + 2);
    if (record_type == LF_TYPESERVER2) {
      std::u8string pdb_path = reader->utf_8_c_string(offset + 24);
      U8 pdb_guid_bytes[16];
      reader->copy_bytes_into(pdb_guid_bytes, 8);
      throw CodeView_Types_In_Separate_PDB_File(std::move(pdb_path),
                                                GUID(pdb_guid_bytes));
    }
    table.add_type_entry_at_offset_(offset);
    offset += record_size + 2;
  }
  return table;
}

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

  U32 get_caller_stack_size(CodeView_Type_Table<Reader>& type_table) {
    return this->get_caller_stack_size(type_table, type_table, fallback_logger);
  }

  U32 get_caller_stack_size(CodeView_Type_Table<Reader>& type_table,
                            CodeView_Type_Table<Reader>& type_index_table,
                            Logger& logger) {
    Reader& reader = *type_table.reader_;
    U32 type_id = this->type_id;
    if (this->has_func_id_type) {
      Reader& index_reader = *type_index_table.reader_;
      std::optional<U64> func_id_type_offset =
          type_index_table.get_offset_of_type_entry_(type_id);
      if (!func_id_type_offset.has_value()) {
        logger.log(fmt::format("cannot find type with ID: 0x{:x}", type_id),
                   this->reader.locate(this->byte_offset));
        return -1;
      }
      // TODO(strager): Check size.
      U64 func_id_type_record_type_offset = *func_id_type_offset + 2;
      U64 func_id_type_record_type =
          index_reader.u16(func_id_type_record_type_offset);
      switch (func_id_type_record_type) {
        case LF_FUNC_ID:
        case LF_MFUNC_ID:
          type_id = index_reader.u32(*func_id_type_offset + 8);
          break;
        default:
          logger.log(fmt::format("unrecognized function ID record type: 0x{:x}",
                                 func_id_type_record_type),
                     reader.locate(func_id_type_record_type_offset));
          return -1;
      }
    }

    std::optional<U64> func_type_offset =
        type_table.get_offset_of_type_entry_(type_id);
    if (!func_type_offset.has_value()) {
      // FIXME(strager): Location is wrong if this->has_func_id_type is true.
      logger.log(fmt::format("cannot find type with ID: 0x{:x}", type_id),
                 this->reader.locate(this->byte_offset));
      return -1;
    }
    U64 func_type_record_type_offset = *func_type_offset + 2;
    // TODO(strager): Check size.
    U16 func_type_record_type = reader.u16(func_type_record_type_offset);

    U32 this_type_id = T_NOTYPE;
    U64 calling_convention_offset;
    U64 parameter_count;
    switch (func_type_record_type) {
      case LF_PROCEDURE:
        calling_convention_offset = *func_type_offset + 8;
        parameter_count = reader.u16(*func_type_offset + 10);
        break;

      case LF_MFUNCTION:
        this_type_id = reader.u32(*func_type_offset + 12);
        calling_convention_offset = *func_type_offset + 16;
        parameter_count = reader.u16(*func_type_offset + 18);
        break;

      default:
        logger.log(fmt::format("unrecognized function type record type: 0x{:x}",
                               func_type_record_type),
                   reader.locate(func_type_record_type_offset));
        return -1;
    }

    U8 calling_convention = reader.u8(calling_convention_offset);
    switch (calling_convention) {
      case CV_CALL_NEAR_C: {
        if (this_type_id != T_NOTYPE) {
          // HACK(strager): Assume that thisTypeID refers to a pointer type.
          // The 'this' parameter would thus be one register (u64) wide like
          // other parameters.
          parameter_count += 1;
        }
        return std::max(parameter_count, U64{4}) * 8;
      }

      default:
        logger.log(
            fmt::format("unrecognized function calling convention: 0x{:x}",
                        calling_convention),
            reader.locate(calling_convention_offset));
        return -1;
    }
  }
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

// FIXME(strager): Why do we need findAllCodeViewFunctionsAsync with .obj but
// findAllCodeViewFunctions2Async with .pdb?
template <class Reader>
void find_all_codeview_functions_2(
    Reader* reader, std::vector<CodeView_Function<Reader>>& out_functions,
    Logger& logger = fallback_logger) {
  U32 signature = reader->u32(0);
  if (signature != CV_SIGNATURE_C13) {
    throw new Unsupported_CodeView_Error();
  }

  find_all_codeview_functions_in_subsection<Reader>(
      Sub_File_Reader(reader, 4, reader->size() - 4), out_functions, logger);
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
std::optional<CodeView_Type> get_codeview_type(
    U32 type_id, CodeView_Type_Table<Reader>* type_table, Logger& logger) {
  if (type_id < special_type_size_map.size()) {
    U8 maybe_size = special_type_size_map[type_id];
    if (maybe_size != 0xff) {
      return CodeView_Type{
          .byte_size = maybe_size,
          .name = std::u8string(special_type_name_map[type_id]),
      };
    }
  }

  std::optional<Sub_File_Reader<Reader>> type_entry_reader =
      type_table->get_reader_for_type_entry_(type_id);
  if (!type_entry_reader.has_value()) {
    // FIXME(strager): This Location is wrong.
    logger.log(fmt::format("cannot find type with ID: 0x{:x}", type_id),
               Location());
    return std::nullopt;
  }
  U16 type_entry_type = type_entry_reader->u16(2);
  switch (type_entry_type) {
    case LF_POINTER: {
      U32 pointee_type_id = type_entry_reader->u32(4);
      U32 pointer_attributes = type_entry_reader->u32(8);
      bool is_const = pointer_attributes & (1 << 10);
      U32 pointer_type = pointer_attributes & 0x1f;

      U64 byte_size;
      switch (pointer_type) {
        case CV_PTR_64:
          byte_size = 8;
          break;

        default:
          logger.log(
              fmt::format("unsupported pointer type: 0x{:x}", pointer_type),
              type_entry_reader->locate(0));
          byte_size = 0;
          break;
      }

      std::optional<CodeView_Type> type =
          get_codeview_type(pointee_type_id, type_table, logger);
      if (!type.has_value()) {
        type = CodeView_Type{.byte_size = 0, .name = u8"<unknown>"};
      }
      if (type->name.ends_with(u8"*")) {
        type->name += u8"*";
      } else {
        type->name += u8" *";
      }
      if (is_const) {
        type->name += u8"const";
      }
      type->byte_size = byte_size;
      return type;
    }

    case LF_CLASS:
    case LF_STRUCTURE: {
      // TODO(strager): Deduplicate code with LF_UNION.
      U16 properties = type_entry_reader->u16(6);
      bool is_forward_declaration = properties & (1 << 7);
      (void)is_forward_declaration;
      // TODO(strager): Support big structs (size >= 0x8000). I think these are
      // encoded with LF_LONG.
      U64 byte_size = type_entry_reader->u16(20);
      std::u8string name = type_entry_reader->utf_8_c_string(22);
      return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
    }

    case LF_UNION: {
      // TODO(strager): Deduplicate code with LF_STRUCTURE.
      U16 properties = type_entry_reader->u16(6);
      bool is_forward_declaration = properties & (1 << 7);
      (void)is_forward_declaration;
      // TODO(strager): Support big unions (size >= 0x8000). I think these are
      // encoded with LF_LONG.
      U64 byte_size = type_entry_reader->u16(12);
      std::u8string name = type_entry_reader->utf_8_c_string(14);
      return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
    }

    case LF_ARRAY: {
      U32 element_type_id = type_entry_reader->u32(4);
      std::optional<CodeView_Type> element_type =
          get_codeview_type(element_type_id, type_table, logger);
      // TODO(strager): Support big arrays (size >= 0x8000). I think these are
      // encoded with LF_LONG.
      U64 byte_size = type_entry_reader->u16(12);
      std::u8string name = element_type.has_value()
                               ? element_type->name + u8"[]"
                               : u8"<unknown>[]";
      return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
    }

    case LF_ENUM: {
      U32 underlying_type_id = type_entry_reader->u32(8);
      std::optional<CodeView_Type> underlying_type =
          get_codeview_type(underlying_type_id, type_table, logger);
      U64 byte_size =
          underlying_type.has_value() ? underlying_type->byte_size : -1;
      std::u8string name = type_entry_reader->utf_8_c_string(16);
      return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
    }

    case LF_MODIFIER: {
      U32 modified_type_id = type_entry_reader->u32(4);
      U16 modifiers = type_entry_reader->u16(8);
      bool is_const = modifiers & (1 << 0);
      bool is_volatile = modifiers & (1 << 1);
      bool is_unaligned = modifiers & (1 << 2);

      std::optional<CodeView_Type> type =
          get_codeview_type(modified_type_id, type_table, logger);
      if (!type.has_value()) {
        return std::nullopt;
      }
      if (is_volatile) {
        type->name = u8"volatile " + type->name;
      }
      if (is_const) {
        type->name = u8"const " + type->name;
      }
      // TODO(strager): is_unaligned
      (void)is_unaligned;
      return type;
    }

    case LF_PROCEDURE:
      return CodeView_Type{.byte_size = static_cast<U64>(-1),
                           .name = u8"<func>"};

    default:
      logger.log(fmt::format("unknown entry kind 0x{:x} for type ID 0x{:x}",
                             type_entry_type, type_id),
                 type_entry_reader->locate(0));
      return std::nullopt;
  }
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
