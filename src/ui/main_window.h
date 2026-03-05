#pragma once

#include "core/ffprobe.h"
#include "settings/app_settings.h"
#include "ui/file_input.h"

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>

namespace core::ffmpeg {
class Renderer;
}

namespace ui {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void onGenerateClicked();
  void onDurationReady(double seconds);
  void onDurationError(const QString &error);
  void onRenderFinished();
  void onRenderError(const QString &error);
  void updateGenerateButton();

private:
  void setupUi();
  void setStatus(const QString &message);
  void setUiEnabled(bool enabled);

  FileInput *visualInput_;
  FileInput *audioInput_;
  FileInput *outputInput_;
  QPushButton *generateButton_;
  QLabel *statusLabel_;
  QProgressBar *progressBar_;

  settings::AppSettings settings_;
  core::ffprobe::DurationProbe *durationProbe_;
  core::ffmpeg::Renderer *renderer_;

  QString pendingVisualPath_;
  QString pendingAudioPath_;
  QString pendingOutputPath_;

  bool outputManuallyEdited_;
  bool settingOutputProgrammatically_;
};

} // namespace ui
