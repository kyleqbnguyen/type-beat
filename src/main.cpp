#include "ui/main_window.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("TypeBeat"));
  app.setOrganizationName(QStringLiteral("TypeBeat"));

  ui::MainWindow window;
  window.show();

  return app.exec();
}
