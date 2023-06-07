#pragma once

#include <cppstacksize/codeview.h>
#include <cppstacksize/file.h>
#include <cppstacksize/pdb.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/util.h>
#include <memory>
#include <optional>

namespace cppstacksize {
struct Project_File {
  using Reader = Span_Reader;

  Project_File(const Project_File&) = delete;
  Project_File& operator=(const Project_File&) = delete;

  std::string name;
  Loaded_File file;
  Reader reader;  // References file's data.

  std::optional<PDB_Super_Block> pdb_super_block;
  std::optional<std::vector<PDB_Blocks_Reader<Reader>>> pdb_streams;
  std::optional<PDB_DBI> pdb_dbi;
  std::optional<PDB_TPI<PDB_Blocks_Reader<Reader>>> pdb_tpi_header;
  std::optional<PDB_TPI<PDB_Blocks_Reader<Reader>>> pdb_ipi_header;

  std::optional<PE_File<Reader>> pe_file;
  std::vector<Sub_File_Reader<Reader>> debug_s_sections;
  std::vector<Sub_File_Reader<Reader>> debug_t_sections;

  explicit Project_File(std::string&& name, Loaded_File file)
      : name(std::move(name)),
        file(std::move(file)),
        reader(this->file.data()) {}

  void try_load_pe_file(Logger& = fallback_logger) {
    try {
      if (!this->pe_file.has_value()) {
        this->pe_file = parse_pe_file(&this->reader);
      }
    } catch (PE_Magic_Mismatch_Error) {
      return;
    }
  }

  void try_load_pdb_generic_headers(Logger& logger) {
    try {
      if (!this->pdb_super_block.has_value()) {
        this->pdb_super_block = parse_pdb_header(this->reader, logger);
      }
      if (!this->pdb_streams.has_value()) {
        this->pdb_streams = parse_pdb_stream_directory(
            &this->reader, *this->pdb_super_block, logger);
      }
    } catch (PDB_Magic_Mismatch&) {
      return;
    }
  }

  void try_load_debug_t_sections() {
    if (this->debug_t_sections.empty() && this->pe_file.has_value()) {
      this->debug_t_sections =
          this->pe_file->find_sections_by_name(u8".debug$T");
    }
  }

  void try_load_debug_s_sections() {
    if (this->debug_s_sections.empty() && this->pe_file.has_value()) {
      this->debug_s_sections =
          this->pe_file->find_sections_by_name(u8".debug$S");
    }
  }
};

class Project {
 public:
  using Reader = Span_Reader;

  void add_file(std::string name, Loaded_File file) {
    this->files_.push_back(
        std::make_unique<Project_File>(std::move(name), std::move(file)));
  }

  // Possibly returns nullptr.
  CodeView_Type_Table* get_type_table(Logger& logger = fallback_logger) {
    if (this->type_table_is_dirty_) {
      this->load_type_table(logger);
      this->type_table_is_dirty_ = false;
    } else {
      // TODO(strager): Copy logs from prior load?
    }
    return get(this->type_table_cache_);
  }

  // Possibly returns nullptr.
  CodeView_Type_Table* get_type_index_table(Logger& logger = fallback_logger) {
    if (this->type_index_table_is_dirty_) {
      this->load_type_index_table(logger);
      this->type_index_table_is_dirty_ = false;
    } else {
      // TODO(strager): Copy logs from prior load?
    }
    return get(this->type_index_table_cache_);
  }

  std::span<const CodeView_Function> get_all_functions(
      Logger& logger = fallback_logger) {
    if (this->functions_are_dirty_) {
      this->load_functions(logger);
      this->functions_are_dirty_ = false;
    } else {
      // TODO(strager): Copy logs from prior load?
    }
    return this->functions_cache_;
  }

 private:
  void load_type_table(Logger& logger) {
    for (std::unique_ptr<Project_File>& file : this->files_) {
      file->try_load_pdb_generic_headers(logger);
      if (file->pdb_streams.has_value()) {
        if (!file->pdb_tpi_header.has_value()) {
          file->pdb_tpi_header =
              parse_pdb_tpi_stream_header(&file->pdb_streams->at(2), logger);
        }
        // TODO[start-type-id]
        this->type_table_cache_ = parse_codeview_types_without_header(
            &file->pdb_tpi_header->type_reader, logger);
        return;
      }

      file->try_load_debug_t_sections();
      for (Sub_File_Reader<Span_Reader>& section_reader :
           file->debug_t_sections) {
        try {
          this->type_table_cache_ = parse_codeview_types(&section_reader);
          return;
        } catch (CodeView_Types_In_Separate_PDB_File&) {
          // Ignore.
        }
      }
    }
  }

  void load_type_index_table(Logger& logger) {
    for (std::unique_ptr<Project_File>& file : this->files_) {
      file->try_load_pdb_generic_headers(logger);
      if (file->pdb_streams.has_value()) {
        if (!file->pdb_ipi_header.has_value()) {
          file->pdb_ipi_header =
              parse_pdb_tpi_stream_header(&file->pdb_streams->at(4), logger);
        }
        // TODO[start-type-id]
        this->type_index_table_cache_ = parse_codeview_types_without_header(
            &file->pdb_ipi_header->type_reader, logger);
        return;
      }

      file->try_load_debug_t_sections();
      for (Sub_File_Reader<Span_Reader>& section_reader :
           file->debug_t_sections) {
        try {
          this->type_index_table_cache_ = parse_codeview_types(&section_reader);
          return;
        } catch (CodeView_Types_In_Separate_PDB_File&) {
          // Ignore.
        }
      }
    }
  }

  void load_functions(Logger& logger) {
    this->functions_cache_.clear();

    for (std::unique_ptr<Project_File>& file : this->files_) {
      file->try_load_pdb_generic_headers(logger);
      file->try_load_pe_file(logger);
    }

    for (std::unique_ptr<Project_File>& file : this->files_) {
      if (!file->pdb_streams.has_value()) continue;
      if (!file->pdb_dbi.has_value()) {
        file->pdb_dbi = parse_pdb_dbi_stream(file->pdb_streams->at(3), logger);
      }
      for (const PDB_DBI_Module& module : file->pdb_dbi->modules) {
        PDB_Blocks_Reader<Reader>& codeview_stream =
            file->pdb_streams->at(module.debug_info_stream_index);
        find_all_codeview_functions_2(&codeview_stream, this->functions_cache_,
                                      logger);
      }
    }

    for (std::unique_ptr<Project_File>& file : this->files_) {
      if (!file->pe_file.has_value()) continue;
      // TODO(strager): Only attach to functions from PDBs linked with this PE
      // (according to the PDB's GUID).
      for (CodeView_Function& func : this->functions_cache_) {
        func.pe_file = &*file->pe_file;
      }

      file->try_load_debug_s_sections();
      for (Sub_File_Reader<Reader>& section_reader : file->debug_s_sections) {
        find_all_codeview_functions(&section_reader, this->functions_cache_);
      }
    }
  }

  std::vector<std::unique_ptr<Project_File>> files_;

  std::vector<CodeView_Function> functions_cache_;
  bool functions_are_dirty_ = true;

  std::optional<CodeView_Type_Table> type_table_cache_;
  bool type_table_is_dirty_ = true;

  std::optional<CodeView_Type_Table> type_index_table_cache_;
  bool type_index_table_is_dirty_ = true;
};
}
