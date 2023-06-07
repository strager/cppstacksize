#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <cppstacksize/codeview.h>
#include <optional>
#include <vector>

namespace cppstacksize {
class Logger;
class Project;
struct CodeView_Function;

class Locals_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Locals_Table_Model(Project *project, Logger *,
                              QObject *parent = nullptr);
  ~Locals_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void set_function(const CodeView_Function *function);

 private:
  struct Cached_Local_Data {
    std::optional<CodeView_Type> type;
  };

  Cached_Local_Data *get_local_data(U64 row) const;

  Project *project_;
  Logger *logger_;
  const CodeView_Function *function_ = nullptr;
  std::vector<CodeView_Function_Local> locals_;
  mutable QCache<U64, Cached_Local_Data> local_data_cache_;
};
}
