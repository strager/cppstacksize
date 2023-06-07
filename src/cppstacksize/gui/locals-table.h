#pragma once

#include <QAbstractTableModel>
#include <cppstacksize/codeview.h>
#include <vector>

namespace cppstacksize {
class Project;
struct CodeView_Function;

class Locals_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Locals_Table_Model(Project *project, QObject *parent = nullptr);
  ~Locals_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void set_function(const CodeView_Function *function);

 private:
  Project *project_;
  const CodeView_Function *function_ = nullptr;
  std::vector<CodeView_Function_Local> locals_;
};
}
