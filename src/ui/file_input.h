#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <cstdint>

namespace ui {

class FileInput : public QWidget {
  Q_OBJECT

public:
  enum class Mode : int8_t { Open, Save };

  explicit FileInput(const QString &filter, Mode mode = Mode::Open,
                     QWidget *parent = nullptr);

  [[nodiscard]]QString path() const;
  void setPath(const QString &path);

signals:
  void pathChanged(const QString &path);

private slots:
  void onBrowseClicked();

private:
  QLineEdit *lineEdit_;
  QPushButton *browseButton_;
  QString filter_;
  Mode mode_;
};

} // namespace ui
