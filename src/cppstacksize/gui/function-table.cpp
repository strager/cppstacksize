#include <cppstacksize/base.h>
#include <cppstacksize/codeview.h>
#include <cppstacksize/gui/function-table.h>
#include <cppstacksize/project.h>

namespace cppstacksize {
Function_Table_Model::Function_Table_Model(Project* project, Logger* logger,
                                           QObject* parent)
    : QAbstractTableModel(parent), project_(project), logger_(logger) {}

Function_Table_Model::~Function_Table_Model() = default;

int Function_Table_Model::rowCount(const QModelIndex&) const {
  return narrow_cast<int>(this->functions_.size());
}

int Function_Table_Model::columnCount(const QModelIndex&) const { return 3; }

QVariant Function_Table_Model::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    const CodeView_Function* func = this->get_function(index);
    if (func == nullptr) {
      return QVariant();
    }
    switch (index.column()) {
      case 0:
        return QString(func->name.c_str());
      case 1:
        return func->self_stack_size;
      case 2:
        if (this->type_table_ != nullptr &&
            this->type_index_table_ != nullptr) {
          Logger& func_logger = *this->logger_;  // TODO(port)
          return func->get_caller_stack_size(
              *this->type_table_, *this->type_index_table_, func_logger);
          // TODO(port):
          // if (func_logger.did_log_message()) {
          //   td.title = func_logger.get_logged_messages_string_for_tool_tip();
          // }
        } else {
          // TODO(strager): Indicate which PDB file needs to be loaded.
          // TODO(port): td.title = "CodeView types cannot be loaded because
          // they are in a separate PDB file";
        }
      default:
        __builtin_unreachable();
        break;
    }
  }
  return QVariant();
}

QVariant Function_Table_Model::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("function");
      case 1:
        return QString("self size");
      case 2:
        return QString("params size");
    }
  }
  return QVariant();
}

void Function_Table_Model::sync_data_from_project() {
  this->beginResetModel();

  this->functions_ = this->project_->get_all_functions(*this->logger_);
  this->type_table_ = this->project_->get_type_table(*this->logger_);
  this->type_index_table_ =
      this->project_->get_type_index_table(*this->logger_);

  this->endResetModel();
}

const CodeView_Function* Function_Table_Model::get_function(
    const QModelIndex& index) const {
  CSS_ASSERT(index.row() >= 0);
  U64 row = narrow_cast<U64>(index.row());
  CSS_ASSERT(row < this->functions_.size());
  if (row >= this->functions_.size()) {
    return nullptr;
  }
  return &this->functions_[index.row()];
}
}
