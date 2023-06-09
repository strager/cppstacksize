#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/base.h>
#include <cppstacksize/codeview.h>
#include <cppstacksize/gui/stack-map-table.h>
#include <cppstacksize/gui/style.h>
#include <cppstacksize/line-tables.h>
#include <cppstacksize/project.h>
#include <map>

namespace cppstacksize {
Stack_Map_Table_Model::Stack_Map_Table_Model(Project* project, Logger* logger,
                                             QObject* parent)
    : QAbstractTableModel(parent), project_(project), logger_(logger) {}

Stack_Map_Table_Model::~Stack_Map_Table_Model() = default;

int Stack_Map_Table_Model::rowCount(const QModelIndex&) const {
  return narrow_cast<int>(this->touch_groups_.size());
}

int Stack_Map_Table_Model::columnCount(const QModelIndex&) const { return 2; }

QVariant Stack_Map_Table_Model::data(const QModelIndex& index, int role) const {
  CSS_ASSERT(index.row() >= 0);
  U64 row = narrow_cast<U64>(index.row());
  CSS_ASSERT(row < this->touch_groups_.size());
  if (row >= this->touch_groups_.size()) {
    return QVariant();
  }
  const Touch_Group& touch_group = this->touch_groups_[index.row()];
  const Touch_Location& location =
      this->touch_locations_[touch_group.first_index];

  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        case 0:
          if (location.line_source_info.is_out_of_bounds()) {
            return QString("?");
          }
          return QString("%1:%2").arg(
              "(todo)", QString::number(location.line_source_info.line_number));
        case 1:
          return narrow_cast<qulonglong>(touch_group.total_touched_size);
        default:
          __builtin_unreachable();
          break;
      }
      break;

    case Qt::ToolTipRole:
      switch (index.column()) {
        case 0:
          if (location.errors_for_tool_tip != nullptr) {
            // TODO(strager): Show all errors for each location in the group,
            // not just the first location's errors.
            return QString(location.errors_for_tool_tip);
          }
          if (location.line_source_info.is_out_of_bounds()) {
            return QString("line information is out of bounds");
          }
          return QVariant();
        case 1:
        default:
          break;
      }
      break;

    case Qt::BackgroundRole:
      switch (index.column()) {
        case 0:
          if (location.line_source_info.is_out_of_bounds()) {
            return warning_background_brush;
          }
          return QVariant();
        case 1:
        default:
          break;
      }
      break;
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
  this->touch_locations_.clear();

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
  this->update_touch_locations();
  this->update_touch_groups();

  this->endResetModel();
}

void Stack_Map_Table_Model::update_touch_locations() {
  this->touch_locations_.clear();
  this->touch_locations_.reserve(this->stack_map_.touches.size());

  Line_Tables* line_tables = this->project_->get_line_tables();
  if (line_tables == nullptr) {
    // See NOTE[touch-locations-size].
    this->touch_locations_.resize(this->stack_map_.touches.size());
    return;
  }

  for (Stack_Map_Touch& touch : this->stack_map_.touches) {
    Capturing_Logger logger(this->logger_);
    Touch_Location touch_location = {
        .line_source_info = line_tables->source_info_for_offset(
            this->function_->line_tables_handle,
            this->function_->code_section_index,
            this->function_->code_offset + touch.offset, logger),
    };
    std::string errors_for_tool_tip =
        logger.get_logged_messages_string_for_tool_tip();
    if (!errors_for_tool_tip.empty()) {
      touch_location.errors_for_tool_tip =
          this->make_touch_location_string(errors_for_tool_tip);
    }
    this->touch_locations_.push_back(touch_location);
  }

  // See NOTE[touch-locations-size].
  CSS_ASSERT(this->touch_locations_.size() == this->stack_map_.touches.size());
}

void Stack_Map_Table_Model::update_touch_groups() {
  // See NOTE[touch-locations-size].
  CSS_ASSERT(this->touch_locations_.size() == this->stack_map_.touches.size());

  std::pmr::monotonic_buffer_resource memory;
  std::pmr::map<Touch_Group_Location, U64> location_to_group_index(&memory);

  this->touch_groups_.clear();
  for (U64 i = 0; i < this->stack_map_.touches.size(); ++i) {
    U32 touched_size = this->stack_map_.touches[i].byte_count;
    // TODO(strager): Make byte_count 0 inside the Stack_Map_Touch instead of
    // fixing it up here.
    if (touched_size == (U32)-1) {
      touched_size = 0;
    }
    auto [existing_it, inserted] = location_to_group_index.try_emplace(
        this->touch_locations_[i].group_key(), this->touch_groups_.size());
    if (inserted) {
      this->touch_groups_.push_back(Touch_Group{
          .first_index = i,
          .last_index = i,
          .total_touched_size = touched_size,
      });
    } else {
      Touch_Group& group = this->touch_groups_.at(existing_it->second);
      group.last_index = i;
      group.total_touched_size += touched_size;
    }
  }
}

char* Stack_Map_Table_Model::make_touch_location_string(std::string_view s) {
  char* heap_string = static_cast<char*>(
      this->touch_location_strings_.allocate(s.size() + 1, /*alignment=*/1));
  char* out = heap_string;
  out = std::copy(s.begin(), s.end(), out);
  *out++ = '\0';
  return heap_string;
}
}
