#include <QApplication>
#include <QCommandLineParser>
#include <cppstacksize/gui/main-window.h>

using namespace cppstacksize;

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("files", "List of files to open on start.",
                               "[FILE ...]");
  parser.process(app);

  Main_Window window;
  window.open_files(parser.positionalArguments());
  window.show();
  return app.exec();
}
