#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <cppstacksize/codeview.h>
#include <optional>
#include <span>

namespace cppstacksize {
class Logger;
class Project;
struct CodeView_Function;

class Function_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Function_Table_Model(Project *, Logger *, QObject *parent = nullptr);
  ~Function_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void sync_data_from_project();
  const CodeView_Function *get_function(const QModelIndex &) const;

 private:
  struct Cached_Function_Data {
    U32 caller_stack_size;
    // If empty, there were no errors.
    std::string errors_for_tool_tip;
  };

  // Possibly returns nullptr.
  Cached_Function_Data *get_function_data(const QModelIndex &index) const;
  Cached_Function_Data *get_function_data(U64 row) const;

  std::span<const CodeView_Function> functions_;
  CodeView_Type_Table *type_table_ = nullptr;
  CodeView_Type_Table *type_index_table_ = nullptr;
  Project *project_;
  Logger *logger_;
  mutable QCache<U64, Cached_Function_Data> function_data_cache_;
};
}
