#pragma once

#include "core/ffmpeg.h"
#include "core/preview_cache.h"
#include "core/render_profile.h"
#include "settings/app_settings.h"
#include "ui/file_input.h"
#include "ui/preview_widget.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

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
  void onPreviewClicked();
  void onCancelClicked();
  void onRenderFinished();
  void onRenderError(const QString &error);
  void onProgressUpdated(int percent);
  void onPreviewFinished();
  void onPreviewError(const QString &error);
  void onPreviewProgress(int percent);
  void updateGenerateButton();
  void onOpenOutputFolder();
  void onAbout();
  void onPresetChanged(int index);

private:
  enum class JobKind { None, Export, Preview };

  void setupUi();
  void setupQualitySection(QVBoxLayout *parentLayout);
  void setupPreviewSection(QVBoxLayout *parentLayout);
  void setupMenuBar();
  void setStatus(const QString &message, bool isError = false);
  void setUiEnabled(bool enabled);
  bool checkDiskSpace(const QString &outputPath);

  void loadQualitySettings();
  core::RenderProfile activeProfile() const;
  void persistActiveProfile();
  void markPreviewStale();
  bool tryReuseCachedPreviewForExport(const QString &outputPath);

  FileInput *visualInput_;
  FileInput *audioInput_;
  FileInput *outputInput_;

  QComboBox *presetCombo_;

  PreviewWidget *previewWidget_;

  QPushButton *generateButton_;
  QPushButton *cancelButton_;
  QPushButton *openFolderButton_;
  QLabel *statusLabel_;
  QProgressBar *progressBar_;

  settings::AppSettings settings_;
  core::ffmpeg::Renderer renderer_;
  core::ffmpeg::Renderer previewRenderer_;
  core::PreviewCache previewCache_;

  QString pendingOutputPath_;
  QString pendingPreviewPath_;
  QString pendingPreviewKey_;
  JobKind currentJob_;

  bool outputManuallyEdited_;
  bool settingOutputProgrammatically_;
  bool previewClickDeferred_ = false;
  bool hasInputError_ = false;
};

} // namespace ui
