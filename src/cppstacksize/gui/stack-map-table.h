#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/line-tables.h>
#include <memory_resource>
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
  struct Touch_Location {
    Line_Source_Info line_source_info = Line_Source_Info::out_of_bounds();
    // C string allocated inside touch_data_cache_strings_, or nullptr.
    const char *errors_for_tool_tip = nullptr;
  };

  void update_touch_locations();
  char *make_touch_location_string(std::string_view);

  Project *project_;
  Logger *logger_;
  Stack_Map stack_map_;
  // NOTE[touch-locations-size]: touch_locations_[i] corresponds to
  // stack_map_.touches[i].
  std::vector<Touch_Location> touch_locations_;
  const CodeView_Function *function_ = nullptr;

  std::pmr::monotonic_buffer_resource touch_location_strings_;
};
}
