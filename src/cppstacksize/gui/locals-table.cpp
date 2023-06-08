#include <cppstacksize/base.h>
#include <cppstacksize/codeview.h>
#include <cppstacksize/gui/locals-table.h>
#include <cppstacksize/gui/style.h>
#include <cppstacksize/project.h>

namespace cppstacksize {
Locals_Table_Model::Locals_Table_Model(Project* project, Logger* logger,
                                       QObject* parent)
    : QAbstractTableModel(parent),
      project_(project),
      logger_(logger),
      local_data_cache_(100) {}

Locals_Table_Model::~Locals_Table_Model() = default;

int Locals_Table_Model::rowCount(const QModelIndex&) const {
  return narrow_cast<int>(this->locals_.size());
}

int Locals_Table_Model::columnCount(const QModelIndex&) const { return 3; }

QVariant Locals_Table_Model::data(const QModelIndex& index, int role) const {
  CSS_ASSERT(index.row() >= 0);
  U64 row = narrow_cast<U64>(index.row());
  CSS_ASSERT(row < this->locals_.size());
  if (row >= this->locals_.size()) {
    return QVariant();
  }
  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        case 0: {
          const CodeView_Function_Local& local = this->locals_[index.row()];
          return QString(local.name.c_str());
        }
        case 1: {
          Cached_Local_Data* data = this->get_local_data(row);
          if (!data->type.has_value()) {
            return "?";
          }
          return QString(data->type->name.c_str());
        }
        case 2: {
          Cached_Local_Data* data = this->get_local_data(row);
          if (!data->type.has_value()) {
            return "?";
          }
          return narrow_cast<qulonglong>(data->type->byte_size);
        }
        default:
          __builtin_unreachable();
          break;
      }
      break;

    case Qt::BackgroundRole:
      switch (index.column()) {
        case 1:
        case 2:
          if (!this->get_local_data(row)->type.has_value()) {
            return warning_background_brush;
          }
          break;

        case 0:
        default:
          break;
      }
      break;

    case Qt::ToolTipRole:
      switch (index.column()) {
        case 1:
        case 2: {
          Cached_Local_Data* data = this->get_local_data(row);
          if (!data->errors_for_tool_tip.empty()) {
            return QString(data->errors_for_tool_tip.c_str());
          }
          break;
        }

        case 0:
        default:
          break;
      }
      break;

    default:
      break;
  }
  return QVariant();
}

QVariant Locals_Table_Model::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("variable");
      case 1:
        return QString("type");
      case 2:
        return QString("size");
    }
  }
  return QVariant();
}

void Locals_Table_Model::set_function(const CodeView_Function* function) {
  this->beginResetModel();
  this->function_ = function;
  if (this->function_) {
    // TODO(port): Logger.
    this->locals_ = this->function_->get_locals(this->function_->byte_offset);
  } else {
    this->locals_.clear();
  }
  this->local_data_cache_.clear();
  this->endResetModel();
}

Locals_Table_Model::Cached_Local_Data* Locals_Table_Model::get_local_data(
    U64 row) const {
  CSS_ASSERT(row < this->locals_.size());

  Cached_Local_Data* data = this->local_data_cache_[row];
  if (data == nullptr) {
    Capturing_Logger logger(this->logger_);
    const CodeView_Function_Local& local = this->locals_[row];
    data = new Cached_Local_Data{
        .type = local.get_type(this->project_->get_type_table(), logger),
    };
    data->errors_for_tool_tip =
        logger.get_logged_messages_string_for_tool_tip();
    this->local_data_cache_.insert(row, data);
  }
  return data;
}
}
