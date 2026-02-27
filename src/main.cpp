#include <QtWidgets>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QWidget window;
  window.resize(600, 600);
  window.show();

  return app.exec();
}
