#pragma once

#include <QMainWindow>

namespace cppstacksize {
class Main_Window : public QMainWindow {
  Q_OBJECT
 public:
  explicit Main_Window();

 private slots:
  void do_open();
};
}
