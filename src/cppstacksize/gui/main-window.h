#pragma once

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <cppstacksize/gui/function-table.h>
#include <cppstacksize/project.h>

namespace cppstacksize {
class Main_Window : public QMainWindow {
  Q_OBJECT
 public:
  explicit Main_Window();

 private slots:
  void do_open();

 private:
  QTableView function_table_;
  Function_Table_Model function_table_model_;
  QSortFilterProxyModel function_table_sorter_;

  Project project_;
};
}
