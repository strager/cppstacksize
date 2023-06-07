#include <QAction>
#include <QFileDialog>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
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

  this->setCentralWidget(&this->function_table_);
}

void Main_Window::do_open() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter(tr("Binaries (*.dll *.exe *.obj *.pdb)"));

  bool updated = false;
  if (dialog.exec()) {
    this->project_.clear();
    QStringList selected_paths = dialog.selectedFiles();
    for (QString &path : selected_paths) {
      std::string path_std_string = std::move(path).toStdString();
      qDebug() << "adding file" << path_std_string.c_str() << "to project";
      this->project_.add_file(path_std_string,
                              Loaded_File::load(path_std_string.c_str()));
      updated = true;
    }
  }

  if (updated) {
    this->function_table_model_.sync_data_from_project();
  }
}
}
