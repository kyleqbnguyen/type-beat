#include "ui/file_input.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>

namespace ui {

FileInput::FileInput(const QString &filter, Mode mode, QWidget *parent)
    : QWidget{parent}, lineEdit_{new QLineEdit(this)},
      browseButton_{new QPushButton(tr("Browse"), this)}, filter_{filter},
      mode_{mode} {
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  lineEdit_->setMinimumHeight(32);
  browseButton_->setFixedWidth(90);
  browseButton_->setMinimumHeight(32);

  layout->addWidget(lineEdit_);
  layout->addWidget(browseButton_);

  connect(browseButton_, &QPushButton::clicked, this,
          &FileInput::onBrowseClicked);
  connect(lineEdit_, &QLineEdit::textChanged, this, &FileInput::pathChanged);
}

QString FileInput::path() const { return lineEdit_->text(); }

void FileInput::setPath(const QString &path) { lineEdit_->setText(path); }

void FileInput::setError(bool error) {
  lineEdit_->setStyleSheet(
      error ? QStringLiteral("border: 1.5px solid #cc3333;") : QString());
}

void FileInput::onBrowseClicked() {
  QString startDir;
  QString currentPath = lineEdit_->text();

  if (!currentPath.isEmpty()) {
    QFileInfo info(currentPath);
    startDir = info.absolutePath();
  }

  QString selectedPath;

  if (mode_ == Mode::Open) {
    selectedPath = QFileDialog::getOpenFileName(this, tr("Select File"),
                                                startDir, filter_);
  } else {
    selectedPath = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                currentPath, filter_);
  }

  if (!selectedPath.isEmpty()) {
    lineEdit_->setText(selectedPath);
  }
}

} // namespace ui
