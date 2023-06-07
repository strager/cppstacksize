#include <QAction>
#include <QFileDialog>
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
}

void Main_Window::do_open() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter(tr("Binaries (*.dll *.exe *.obj *.pdb)"));
  if (dialog.exec()) {
    QStringList selected_paths = dialog.selectedFiles();
    qDebug() << "selected:";
    for (QStringView path : selected_paths) {
      qDebug() << "-" << path;
    }
  }
}
}
