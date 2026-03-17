#include "ui/main_window.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("TypeBeat"));
  app.setOrganizationName(QStringLiteral("TypeBeat"));
  app.setWindowIcon(QIcon(QStringLiteral(":/TypeBeat.png")));

  ui::MainWindow window;
  window.show();

  return app.exec();
}
