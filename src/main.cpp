#include "gui/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("NodeCode");
  app.setOrganizationName("NodeCode");
  MainWindow window;
  window.show();
  return app.exec();
}
