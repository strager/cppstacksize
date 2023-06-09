#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <cppstacksize/gui/main-window.h>
#include <cstdio>

namespace cppstacksize {
Main_Window::Main_Window() {
  QMenu *file_menu = this->menuBar()->addMenu("&File");
  QAction *open_action = new QAction("&Open...");
  open_action->setShortcuts(QKeySequence::Open);
  open_action->setStatusTip("Load PDB or DLL files");
  connect(open_action, &QAction::triggered, this, &Main_Window::do_open);
  file_menu->addAction(open_action);

  this->function_table_.setShowGrid(false);
  this->function_table_.verticalHeader()->setVisible(false);
  this->function_table_.setSortingEnabled(true);
  this->function_table_.setSelectionBehavior(QAbstractItemView::SelectRows);
  this->function_table_.setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  this->function_table_sorter_.setSourceModel(&this->function_table_model_);
  this->function_table_.setModel(&this->function_table_sorter_);
  connect(this->function_table_.selectionModel(),
          &QItemSelectionModel::selectionChanged, this,
          &Main_Window::changed_selected_function);
  this->setCentralWidget(&this->function_table_);

  this->locals_table_.setShowGrid(false);
  this->locals_table_.verticalHeader()->setVisible(false);
  this->locals_table_.setSortingEnabled(true);
  this->locals_table_.setSelectionBehavior(QAbstractItemView::SelectRows);
  this->locals_table_.setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  this->locals_table_sorter_.setSourceModel(&this->locals_table_model_);
  this->locals_table_.setModel(&this->locals_table_sorter_);
  QDockWidget *dock = new QDockWidget();
  dock->setWidget(&this->locals_table_);
  this->addDockWidget(Qt::RightDockWidgetArea, dock);

  this->stack_map_table_.setShowGrid(false);
  this->stack_map_table_.verticalHeader()->setVisible(false);
  this->stack_map_table_.setSortingEnabled(true);
  this->stack_map_table_.setSelectionBehavior(QAbstractItemView::SelectRows);
  this->stack_map_table_.setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  this->stack_map_table_sorter_.setSourceModel(&this->stack_map_table_model_);
  this->stack_map_table_.setModel(&this->stack_map_table_sorter_);
  dock = new QDockWidget();
  dock->setWidget(&this->stack_map_table_);
  this->addDockWidget(Qt::RightDockWidgetArea, dock);

  this->log_table_.setShowGrid(false);
  this->log_table_.verticalHeader()->setVisible(true);
  this->log_table_.setSortingEnabled(false);
  this->log_table_.setSelectionBehavior(QAbstractItemView::SelectRows);
  this->log_table_.setModel(&this->logger_);
  dock = new QDockWidget();
  dock->setWidget(&this->log_table_);
  this->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void Main_Window::open_files(std::span<const QString> file_paths) {
  this->project_.clear();

  for (const QString &path : file_paths) {
    std::string path_std_string = path.toStdString();
    qDebug() << "adding file" << path_std_string.c_str() << "to project";
    this->project_.add_file(path_std_string,
                            Loaded_File::load(path_std_string.c_str()));
  }

  this->function_table_model_.sync_data_from_project();
}

void Main_Window::do_open() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter(tr("Binaries (*.dll *.exe *.obj *.pdb)"));

  if (dialog.exec()) {
    this->open_files(dialog.selectedFiles());
  }
}

void Main_Window::changed_selected_function(
    const QItemSelection &selected,
    [[maybe_unused]] const QItemSelection &deselected) {
  QList<QModelIndex> selected_indexes = selected.indexes();

  {
    std::optional<int> selected_row;
    for (QModelIndex &index : selected_indexes) {
      if (selected_row.has_value() && *selected_row != index.row()) {
        qWarning() << "unexpectedly selected multiple rows";
        CSS_ASSERT(false);
        break;
      }
      selected_row = index.row();
    }
  }

  const CodeView_Function *selected_function =
      selected_indexes.empty() ? nullptr
                               : this->function_table_model_.get_function(
                                     this->function_table_sorter_.mapToSource(
                                         selected_indexes.at(0)));
  this->locals_table_model_.set_function(selected_function);
  this->stack_map_table_model_.set_function(selected_function);
}
}
