#pragma once

#include <QAbstractTableModel>
#include <QCache>
#include <compare>
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
  struct Touch_Group_Location {
    U32 line_number;

    friend std::strong_ordering operator<=>(
        const Touch_Group_Location &, const Touch_Group_Location &) = default;
  };

  struct Touch_Location {
    Line_Source_Info line_source_info = Line_Source_Info::out_of_bounds();
    // C string allocated inside touch_data_cache_strings_, or nullptr.
    const char *errors_for_tool_tip = nullptr;

    Touch_Group_Location group_key() {
      // TODO[line-table-file]: Include the file ID.
      return Touch_Group_Location{
          .line_number = line_source_info.line_number,
      };
    }
  };

  // A list of Stack_Map_Touch-s grouped by Touch_Group_Location (file and line
  // number).
  struct Touch_Group {
    // The first and last index into stack_map_.touches_ and touch_locations_
    // with the group's line number.
    //
    // Invariant: touch_locations_[first_index].group_key() ==
    //            touch_locations_[last_index ].group_key()
    //
    // It is possible that
    // touch_locations_[first_index].line_source_info.line_number is different
    // than touch_locations_[first_index + 1].line_source_info.line_number.
    U64 first_index;
    U64 last_index;

    // Number of bytes touched for each Stack_Map_Touch in this group.
    U64 total_touched_size;
  };

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
  std::vector<Touch_Location> touch_locations_;
  std::vector<Touch_Group> touch_groups_;
  const CodeView_Function *function_ = nullptr;

  std::pmr::monotonic_buffer_resource touch_location_strings_;
};
}
