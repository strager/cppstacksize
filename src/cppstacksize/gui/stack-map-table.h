#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/stack-map-touch-group.h>
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
  // Updates this->touch_locations_.
  void update_touch_locations();

  // Precondition: this->touch_locations_ is up to date.
  void update_touch_groups();

  char *make_touch_location_string(std::string_view);

  Project *project_;
  Logger *logger_;
  Stack_Map stack_map_;
  // NOTE[touch-locations-size]: touch_locations_[i] corresponds to
  // stack_map_.touches[i].
  std::vector<Stack_Map_Touch_Location> touch_locations_;
  Stack_Map_Touch_Groups touch_groups_;
  const CodeView_Function *function_ = nullptr;

  std::pmr::monotonic_buffer_resource touch_location_strings_;
};
}
