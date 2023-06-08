#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/codeview-constants.h>
#include <cppstacksize/logger.h>
#include <cppstacksize/pdb-reader.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/util.h>
#include <exception>
#include <string>
#include <utility>
#include <variant>
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

struct CodeView_Type {
  U64 byte_size;
  std::u8string name;
};

class CodeView_Type_Table {
 public:
  template <class Reader>
  explicit CodeView_Type_Table(const Reader* reader, U32 start_type_id)
      : reader_(reader), start_type_id_(start_type_id) {}

  void add_type_entry_at_offset_(U64 offset) {
    this->type_entry_offsets_.push_back(offset);
  }

  std::optional<CodeView_Type> get_type(U32 type_id,
                                        Logger& logger = fallback_logger) {
    if (type_id < special_type_size_map.size()) {
      U8 maybe_size = special_type_size_map[type_id];
      if (maybe_size != 0xff) {
        return CodeView_Type{
            .byte_size = maybe_size,
            .name = std::u8string(special_type_name_map[type_id]),
        };
      }
    }

    std::optional<U64> offset = this->get_offset_of_type_entry_(type_id);
    if (!offset.has_value()) {
      // FIXME(strager): This Location is wrong.
      logger.log(fmt::format("cannot find type with ID: 0x{:x}", type_id),
                 Location());
      return std::nullopt;
    }

    return std::visit(
        [&](auto* reader) {
          U64 size = reader->u16(*offset);
          Sub_File_Reader entry_reader(reader, *offset, size + 2);
          return this->get_codeview_type_from_type_entry_(entry_reader, type_id,
                                                          logger);
        },
        this->reader_);
  }

  std::optional<U64> get_offset_of_type_entry_(U32 type_id) const {
    U32 index = type_id - this->start_type_id_;
    if (index < 0 || index >= this->type_entry_offsets_.size()) {
      return std::nullopt;
    }
    return this->type_entry_offsets_[index];
  }

  std::vector<U64> type_entry_offsets_;
  std::variant<const Sub_File_Reader<Span_Reader>*,
               const Sub_File_Reader<PDB_Blocks_Reader<Span_Reader>>*>
      reader_;
  U32 start_type_id_;

 private:
  template <class R>
  std::optional<CodeView_Type> get_codeview_type_from_type_entry_(
      Sub_File_Reader<R> type_entry_reader, U32 type_id, Logger& logger) {
    U16 type_entry_type = type_entry_reader.u16(2);
    switch (type_entry_type) {
      case LF_POINTER: {
        U32 pointee_type_id = type_entry_reader.u32(4);
        U32 pointer_attributes = type_entry_reader.u32(8);
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
                type_entry_reader.locate(0));
            byte_size = 0;
            break;
        }

        std::optional<CodeView_Type> type =
            this->get_type(pointee_type_id, logger);
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
        U16 properties = type_entry_reader.u16(6);
        bool is_forward_declaration = properties & (1 << 7);
        (void)is_forward_declaration;
        // TODO(strager): Support big structs (size >= 0x8000). I think these
        // are encoded with LF_LONG.
        U64 byte_size = type_entry_reader.u16(20);
        std::u8string name = type_entry_reader.utf_8_c_string(22);
        return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
      }

      case LF_UNION: {
        // TODO(strager): Deduplicate code with LF_STRUCTURE.
        U16 properties = type_entry_reader.u16(6);
        bool is_forward_declaration = properties & (1 << 7);
        (void)is_forward_declaration;
        // TODO(strager): Support big unions (size >= 0x8000). I think these are
        // encoded with LF_LONG.
        U64 byte_size = type_entry_reader.u16(12);
        std::u8string name = type_entry_reader.utf_8_c_string(14);
        return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
      }

      case LF_ARRAY: {
        U32 element_type_id = type_entry_reader.u32(4);
        std::optional<CodeView_Type> element_type =
            this->get_type(element_type_id, logger);
        // TODO(strager): Support big arrays (size >= 0x8000). I think these are
        // encoded with LF_LONG.
        U64 byte_size = type_entry_reader.u16(12);
        std::u8string name = element_type.has_value()
                                 ? element_type->name + u8"[]"
                                 : u8"<unknown>[]";
        return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
      }

      case LF_ENUM: {
        U32 underlying_type_id = type_entry_reader.u32(8);
        std::optional<CodeView_Type> underlying_type =
            this->get_type(underlying_type_id, logger);
        U64 byte_size =
            underlying_type.has_value() ? underlying_type->byte_size : -1;
        std::u8string name = type_entry_reader.utf_8_c_string(16);
        return CodeView_Type{.byte_size = byte_size, .name = std::move(name)};
      }

      case LF_MODIFIER: {
        U32 modified_type_id = type_entry_reader.u32(4);
        U16 modifiers = type_entry_reader.u16(8);
        bool is_const = modifiers & (1 << 0);
        bool is_volatile = modifiers & (1 << 1);
        bool is_unaligned = modifiers & (1 << 2);

        std::optional<CodeView_Type> type =
            this->get_type(modified_type_id, logger);
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
                   type_entry_reader.locate(0));
        return std::nullopt;
    }
  }
};

template <class Reader>
CodeView_Type_Table parse_codeview_types(Reader* reader) {
  return parse_codeview_types(reader, fallback_logger);
}

template <class Reader>
CodeView_Type_Table parse_codeview_types(Reader* reader, Logger& logger) {
  U32 signature = reader->u32(0);
  if (signature != CV_SIGNATURE_C13) {
    throw Unsupported_CodeView_Error();
  }
  return parse_codeview_types_without_header(reader, 4, logger);
}

template <class Reader>
CodeView_Type_Table parse_codeview_types_without_header(
    Reader* reader, Logger& logger = fallback_logger) {
  return parse_codeview_types_without_header(reader, 0, logger);
}

template <class Reader>
CodeView_Type_Table parse_codeview_types_without_header(Reader* reader,
                                                        U64 offset, Logger&) {
  // FIXME[start-type-id]: This should be a parameter. PDB can overwrite the
  // initial type ID.
  U32 start_type_id = 0x1000;
  CodeView_Type_Table table(reader, start_type_id);
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

struct CodeView_Function_Local;

struct CodeView_Function {
  std::u8string name;
  std::variant<Sub_File_Reader<Span_Reader>,
               Sub_File_Reader<PDB_Blocks_Reader<Span_Reader>>>
      reader;
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
  PE_File<Span_Reader>* pe_file = nullptr;

  bool has_func_id_type;
  U32 type_id;

  U32 get_caller_stack_size(const CodeView_Type_Table& type_table,
                            Logger& logger = fallback_logger) {
    return this->get_caller_stack_size(type_table, type_table, logger);
  }

  U32 get_caller_stack_size(const CodeView_Type_Table& type_table,
                            const CodeView_Type_Table& type_index_table,
                            Logger& logger = fallback_logger) const {
    // TODO(strager): Refactor the std::visit mess.
    return std::visit(
        [&](auto* reader_ptr) -> U32 {
          auto& reader = *reader_ptr;
          U32 type_id = this->type_id;
          if (this->has_func_id_type) {
            bool ok = std::visit(
                [&](auto* index_reader_ptr) -> bool {
                  auto& index_reader = *index_reader_ptr;
                  std::optional<U64> func_id_type_offset =
                      type_index_table.get_offset_of_type_entry_(type_id);
                  if (!func_id_type_offset.has_value()) {
                    logger.log(fmt::format("cannot find type with ID: 0x{:x}",
                                           type_id),
                               this->location());
                    return false;
                  }
                  // TODO(strager): Check size.
                  U64 func_id_type_record_type_offset =
                      *func_id_type_offset + 2;
                  U64 func_id_type_record_type =
                      index_reader.u16(func_id_type_record_type_offset);
                  switch (func_id_type_record_type) {
                    case LF_FUNC_ID:
                    case LF_MFUNC_ID:
                      type_id = index_reader.u32(*func_id_type_offset + 8);
                      return true;
                    default:
                      logger.log(
                          fmt::format(
                              "unrecognized function ID record type: 0x{:x}",
                              func_id_type_record_type),
                          reader.locate(func_id_type_record_type_offset));
                      return false;
                  }
                },
                type_index_table.reader_);
            if (!ok) {
              return -1;
            }
          }

          std::optional<U64> func_type_offset =
              type_table.get_offset_of_type_entry_(type_id);
          if (!func_type_offset.has_value()) {
            // FIXME(strager): Location is wrong if this->has_func_id_type is
            // true.
            logger.log(fmt::format("cannot find type with ID: 0x{:x}", type_id),
                       this->location());
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
              logger.log(
                  fmt::format("unrecognized function type record type: 0x{:x}",
                              func_type_record_type),
                  reader.locate(func_type_record_type_offset));
              return -1;
          }

          U8 calling_convention = reader.u8(calling_convention_offset);
          switch (calling_convention) {
            case CV_CALL_NEAR_C: {
              if (this_type_id != T_NOTYPE) {
                // HACK(strager): Assume that thisTypeID refers to a pointer
                // type. The 'this' parameter would thus be one register (u64)
                // wide like other parameters.
                parameter_count += 1;
              }
              return std::max(parameter_count, U64{4}) * 8;
            }

            default:
              logger.log(fmt::format(
                             "unrecognized function calling convention: 0x{:x}",
                             calling_convention),
                         reader.locate(calling_convention_offset));
              return -1;
          }
        },
        type_table.reader_);
  }

  std::vector<CodeView_Function_Local> get_locals(
      U64 offset, Logger& logger = fallback_logger) const;

  // Returns null if no PEFile is associated with this function.
  std::optional<Sub_File_Reader<Span_Reader>> get_instruction_bytes_reader(
      Logger& logger = fallback_logger) const {
    if (this->pe_file == nullptr) {
      return std::nullopt;
    }
    if (this->code_section_index >= this->pe_file->sections.size()) {
      logger.log(fmt::format("could not find section index {} in PE file "
                             "referenced by CodeView function",
                             this->code_section_index),
                 this->location());
      return std::nullopt;
    }
    PE_Section& code_section =
        this->pe_file->sections[this->code_section_index];
    Sub_File_Reader section_reader =
        this->pe_file->reader_for_section(code_section);
    return section_reader.sub_reader(this->code_offset, this->code_size);
  }

  Location location() const {
    return std::visit(
        [&](auto& reader) { return reader.locate(this->byte_offset); },
        this->reader);
  }
};

template <class Reader>
std::vector<CodeView_Function> find_all_codeview_functions(Reader* reader) {
  std::vector<CodeView_Function> out_functions;
  find_all_codeview_functions(reader, out_functions);
  return out_functions;
}

template <class Reader>
void find_all_codeview_functions(
    Reader* reader, std::vector<CodeView_Function>& out_functions) {
  find_all_codeview_functions(reader, out_functions, fallback_logger);
}

template <class Reader>
void find_all_codeview_functions(Reader* reader,
                                 std::vector<CodeView_Function>& out_functions,
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
        find_all_codeview_functions_in_subsection(
            reader->sub_reader(offset, subsection_size), out_functions, logger);
        break;
      case DEBUG_S_LINES:
        // TODO[obj-lines]
        break;
      default:
        // Ignore.
        break;
    }
    offset += subsection_size;
  }
}

template <class Reader>
std::vector<CodeView_Function> find_all_codeview_functions_2(
    Reader* reader, Logger& logger = fallback_logger) {
  std::vector<CodeView_Function> functions;
  find_all_codeview_functions_2(reader, functions, logger);
  return functions;
}

// FIXME(strager): Why do we need findAllCodeViewFunctionsAsync with .obj but
// findAllCodeViewFunctions2Async with .pdb?
template <class Reader>
void find_all_codeview_functions_2(
    Reader* reader, std::vector<CodeView_Function>& out_functions,
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
    std::vector<CodeView_Function>& out_functions, Logger& logger) {
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
        CodeView_Function func{
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
        CodeView_Function& func = out_functions[out_functions.size() - 1];
        func.self_stack_size = reader.u32(offset + 4);
        break;
      }

      default:
        break;
    }

    offset += record_size + 2;
  }
}

struct CodeView_Function_Local {
  std::u8string name;
  U32 sp_offset;
  U32 type_id;
  Location location;

  std::optional<CodeView_Type> get_type(
      CodeView_Type_Table* type_table, Logger& logger = fallback_logger) const {
    std::optional<CodeView_Type> type =
        type_table->get_type(this->type_id, logger);
    if (!type.has_value()) {
      logger.log(fmt::format("local has unknown type: 0x{:x}", this->type_id),
                 this->location);
      return std::nullopt;
    }
    return type;
  }
};

template <class Reader>
std::vector<CodeView_Function_Local> get_codeview_function_locals(
    Sub_File_Reader<Reader> reader, U64 offset) {
  std::vector<CodeView_Function_Local> out_locals;
  get_codeview_function_locals<Reader>(reader, offset, out_locals,
                                       fallback_logger);
  return out_locals;
}

template <class Reader>
void get_codeview_function_locals(
    Sub_File_Reader<Reader> reader, U64 offset,
    std::vector<CodeView_Function_Local>& out_locals, Logger&) {
  U32 depth = 1;
  while (offset < reader.size()) {
    U64 record_size = reader.u16(offset + 0);
    U16 record_type = reader.u16(offset + 2);
    switch (record_type) {
      case S_REGREL32: {
        CodeView_Function_Local local{
            .name = reader.utf_8_c_string(offset + 14),
            // TODO(strager): Verify that the register is RSP.
            .sp_offset = reader.u32(offset + 4),
            .type_id = reader.u32(offset + 8),
            .location = reader.locate(offset),
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

inline std::vector<CodeView_Function_Local> CodeView_Function::get_locals(
    U64 offset, Logger& logger) const {
  std::vector<CodeView_Function_Local> out_locals;
  std::visit(
      [&](auto& reader) {
        get_codeview_function_locals(reader, offset, out_locals, logger);
      },
      this->reader);
  return out_locals;
}
}
