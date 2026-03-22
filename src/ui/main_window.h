#pragma once

#include "core/ffmpeg.h"
#include "settings/app_settings.h"
#include "ui/file_input.h"

#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>

namespace ui {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void onGenerateClicked();
  void onCancelClicked();
  void onRenderFinished();
  void onRenderError(const QString &error);
  void onProgressUpdated(int percent);
  void updateGenerateButton();
  void onOpenOutputFolder();
  void onAbout();

private:
  void setupUi();
  void setupMenuBar();
  void setStatus(const QString &message, bool isError = false);
  void setUiEnabled(bool enabled);
  bool checkDiskSpace(const QString &outputPath);

  FileInput *visualInput_;
  FileInput *audioInput_;
  FileInput *outputInput_;
  QPushButton *generateButton_;
  QPushButton *cancelButton_;
  QPushButton *openFolderButton_;
  QLabel *statusLabel_;
  QProgressBar *progressBar_;

  settings::AppSettings settings_;
  core::ffmpeg::Renderer renderer_;

  QString pendingOutputPath_;

  bool outputManuallyEdited_;
  bool settingOutputProgrammatically_;
};

} // namespace ui
