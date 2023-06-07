#include <QApplication>
#include <cppstacksize/gui/main-window.h>

using namespace cppstacksize;

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  Main_Window window;
  window.show();
  return app.exec();
}
