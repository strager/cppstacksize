#include <QApplication>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <cppstacksize/gui/function-table.h>
#include <cppstacksize/gui/main-window.h>

using namespace cppstacksize;

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  Main_Window window;

  QTableView function_table;
  function_table.setShowGrid(false);
  function_table.verticalHeader()->setVisible(false);
  function_table.setSortingEnabled(true);
  function_table.setSelectionBehavior(QAbstractItemView::SelectRows);
  function_table.setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  Function_Table_Model function_table_model;
  QSortFilterProxyModel proxy_model;
  proxy_model.setSourceModel(&function_table_model);
  function_table.setModel(&proxy_model);

  window.setCentralWidget(&function_table);
  window.show();

  return app.exec();
}
