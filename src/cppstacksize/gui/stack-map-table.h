#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/line-tables.h>
#include <vector>

namespace cppstacksize {
class Logger;
class Project;
struct CodeView_Function;

class Stack_Map_Table_Model : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit Stack_Map_Table_Model(Project *project, Logger *,
                                 QObject *parent = nullptr);
  ~Stack_Map_Table_Model();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;

  void set_function(const CodeView_Function *function);

 private:
  struct Cached_Touch_Data {
    Line_Source_Info line_source_info;
    std::string errors_for_tool_tip;
  };

  // Possibly returns nullptr.
  Cached_Touch_Data *get_touch_data(const QModelIndex &index) const;
  Cached_Touch_Data *get_touch_data(U64 row) const;

  Project *project_;
  Logger *logger_;
  Stack_Map stack_map_;
  const CodeView_Function *function_ = nullptr;
  mutable QCache<U64, Cached_Touch_Data> touch_data_cache_;
};
}
