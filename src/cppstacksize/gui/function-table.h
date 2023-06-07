#pragma once

#include <QAbstractTableModel>
#include <cppstacksize/codeview.h>
#include <optional>

namespace cppstacksize {
class Project;
struct CodeView_Function;

class Function_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Function_Table_Model(Project *project, QObject *parent = nullptr);
  ~Function_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void sync_data_from_project();

 private:
  std::vector<CodeView_Function> functions_;
  std::optional<CodeView_Type_Table> type_table_;
  std::optional<CodeView_Type_Table> type_index_table_;
  Project *project_;
};
}
