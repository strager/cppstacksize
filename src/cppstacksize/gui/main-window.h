#pragma once

#include <QItemSelection>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <cppstacksize/gui/function-table.h>
#include <cppstacksize/gui/locals-table.h>
#include <cppstacksize/project.h>

namespace cppstacksize {
class Main_Window : public QMainWindow {
  Q_OBJECT
 public:
  explicit Main_Window();

  void open_files(std::span<const QString>);

 private slots:
  void do_open();
  void changed_selected_function(const QItemSelection &selected,
                                 const QItemSelection &deselected);

 private:
  Project project_;

  QTableView function_table_;
  Function_Table_Model function_table_model_ =
      Function_Table_Model(&this->project_);
  QSortFilterProxyModel function_table_sorter_;

  QTableView locals_table_;
  Locals_Table_Model locals_table_model_ = Locals_Table_Model(&this->project_);
  QSortFilterProxyModel locals_table_sorter_;
};
}
