#include <cppstacksize/base.h>
#include <cppstacksize/codeview.h>
#include <cppstacksize/gui/locals-table.h>
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
  if (role == Qt::DisplayRole) {
    CSS_ASSERT(index.row() >= 0);
    U64 row = narrow_cast<U64>(index.row());
    CSS_ASSERT(row < this->locals_.size());
    if (row >= this->locals_.size()) {
      return QVariant();
    }
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
    const CodeView_Function_Local& local = this->locals_[row];
    // TODO(port): Local logger.
    Logger& logger = *this->logger_;
    data = new Cached_Local_Data{
        .type = local.get_type(this->project_->get_type_table(), logger),
    };
    this->local_data_cache_.insert(row, data);
  }
  return data;
}
}
