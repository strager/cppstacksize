#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/base.h>
#include <cppstacksize/codeview.h>
#include <cppstacksize/gui/stack-map-table.h>
#include <cppstacksize/line-tables.h>
#include <cppstacksize/project.h>

namespace cppstacksize {
Stack_Map_Table_Model::Stack_Map_Table_Model(Project* project, Logger* logger,
                                             QObject* parent)
    : QAbstractTableModel(parent), project_(project), logger_(logger) {}

Stack_Map_Table_Model::~Stack_Map_Table_Model() = default;

int Stack_Map_Table_Model::rowCount(const QModelIndex&) const {
  return narrow_cast<int>(this->stack_map_.touches.size());
}

int Stack_Map_Table_Model::columnCount(const QModelIndex&) const { return 2; }

QVariant Stack_Map_Table_Model::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    CSS_ASSERT(index.row() >= 0);
    U64 row = narrow_cast<U64>(index.row());
    CSS_ASSERT(row < this->stack_map_.touches.size());
    if (row >= this->stack_map_.touches.size()) {
      return QVariant();
    }
    const Stack_Map_Touch& touch = this->stack_map_.touches[index.row()];
    switch (index.column()) {
      case 0: {
        if (this->function_ == nullptr ||
            this->function_->line_tables_handle.is_null()) {
          return QString("+%1").arg(touch.offset);
        }
        Line_Tables* line_tables = this->project_->get_line_tables();
        // TODO(perf): Cache this lookup.
        Line_Source_Info info = line_tables->source_info_for_offset(
            this->function_->line_tables_handle,
            this->function_->code_section_index,
            this->function_->code_offset + touch.offset);
        if (info.is_out_of_bounds()) {
          // TODO(strager): Indicate that this location is out of bounds with a
          // tooltip.
          return QString("+%1").arg(touch.offset);
        }
        return QString("%1:%2").arg("(todo)",
                                    QString::number(info.line_number));
      }
      case 1:
        return touch.byte_count;
      default:
        __builtin_unreachable();
        break;
    }
  }
  return QVariant();
}

QVariant Stack_Map_Table_Model::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("code loc");
      case 1:
        return QString("size");
    }
  }
  return QVariant();
}

void Stack_Map_Table_Model::set_function(const CodeView_Function* function) {
  this->beginResetModel();
  this->stack_map_.clear();
  this->function_ = function;
  if (function) {
    std::optional<Sub_File_Reader<Span_Reader>> instructions_reader =
        function->get_instruction_bytes_reader(*this->logger_);
    if (instructions_reader.has_value()) {
      std::vector<U8> instruction_bytes(instructions_reader->size());
      instructions_reader->copy_bytes_into(instruction_bytes, 0);
      this->stack_map_ = analyze_x86_64_stack_map(instruction_bytes);
    }
  }
  this->endResetModel();
}
}
