#pragma once

#include <QMainWindow>
#include <cppstacksize/project.h>

namespace cppstacksize {
class Main_Window : public QMainWindow {
  Q_OBJECT
 public:
  explicit Main_Window();

 private slots:
  void do_open();

 private:
  Project project_;
};
}
