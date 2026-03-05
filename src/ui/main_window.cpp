#include "ui/main_window.h"

#include "core/ffmpeg.h"
#include "core/file_utils.h"

#include <QFileInfo>
#include <QFormLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

namespace {

const QString kVisualFilter =
    QStringLiteral("Visual (*.png *.jpg *.jpeg *.mp4 *.mov)");
const QString kAudioFilter = QStringLiteral("Audio (*.mp3 *.wav)");
const QString kOutputFilter = QStringLiteral("Video (*.mp4)");

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), visualInput_(nullptr), audioInput_(nullptr),
      outputInput_(nullptr), generateButton_(nullptr), statusLabel_(nullptr),
      durationProbe_(nullptr), renderer_(nullptr) {
  setupUi();
  setWindowTitle(tr("Type Beat Generator"));
  resize(500, 200);
}

void MainWindow::setupUi() {
  auto *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  auto *mainLayout = new QVBoxLayout(centralWidget);

  auto *formLayout = new QFormLayout();

  visualInput_ = new FileInput(kVisualFilter, FileInput::Mode::Open, this);
  visualInput_->setPath(settings_.lastVisualPath());
  formLayout->addRow(tr("Visual:"), visualInput_);

  audioInput_ = new FileInput(kAudioFilter, FileInput::Mode::Open, this);
  audioInput_->setPath(settings_.lastAudioPath());
  formLayout->addRow(tr("Audio:"), audioInput_);

  outputInput_ = new FileInput(kOutputFilter, FileInput::Mode::Save, this);
  formLayout->addRow(tr("Output:"), outputInput_);

  mainLayout->addLayout(formLayout);

  generateButton_ = new QPushButton(tr("Generate"), this);
  generateButton_->setEnabled(false);
  mainLayout->addWidget(generateButton_);

  statusLabel_ = new QLabel(tr("Ready"), this);
  mainLayout->addWidget(statusLabel_);

  mainLayout->addStretch();

  connect(visualInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(audioInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(outputInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(generateButton_, &QPushButton::clicked, this,
          &MainWindow::onGenerateClicked);

  // Auto-generate output path when audio changes
  connect(audioInput_, &FileInput::pathChanged, this,
          [this](const QString &audioPath) {
            if (!audioPath.isEmpty() && QFileInfo::exists(audioPath)) {
              QString outputPath = core::file::generateOutputPath(
                  audioPath, settings_.outputDir());
              outputInput_->setPath(outputPath);
            }
          });

  updateGenerateButton();
}

void MainWindow::updateGenerateButton() {
  QString visualPath = visualInput_->path();
  QString audioPath = audioInput_->path();
  QString outputPath = outputInput_->path();

  bool valid = !visualPath.isEmpty() && !audioPath.isEmpty() &&
               !outputPath.isEmpty() && QFileInfo::exists(visualPath) &&
               QFileInfo::exists(audioPath);

  generateButton_->setEnabled(valid);
}

void MainWindow::onGenerateClicked() {
  pendingVisualPath_ = visualInput_->path();
  pendingAudioPath_ = audioInput_->path();
  pendingOutputPath_ = outputInput_->path();

  // Save paths for next session
  settings_.setLastVisualPath(pendingVisualPath_);
  settings_.setLastAudioPath(pendingAudioPath_);
  settings_.setOutputDir(QFileInfo(pendingOutputPath_).absolutePath());

  setUiEnabled(false);
  setStatus(tr("Getting audio duration..."));

  // Create new probe for this operation
  durationProbe_ = new core::ffprobe::DurationProbe(this);
  connect(durationProbe_, &core::ffprobe::DurationProbe::durationReady, this,
          &MainWindow::onDurationReady);
  connect(durationProbe_, &core::ffprobe::DurationProbe::errorOccurred, this,
          &MainWindow::onDurationError);

  durationProbe_->probe(pendingAudioPath_);
}

void MainWindow::onDurationReady(double seconds) {
  durationProbe_->deleteLater();
  durationProbe_ = nullptr;

  setStatus(tr("Rendering video..."));

  renderer_ = new core::ffmpeg::Renderer(this);
  connect(renderer_, &core::ffmpeg::Renderer::finished, this,
          &MainWindow::onRenderFinished);
  connect(renderer_, &core::ffmpeg::Renderer::errorOccurred, this,
          &MainWindow::onRenderError);

  renderer_->render(pendingVisualPath_, pendingAudioPath_, pendingOutputPath_,
                    seconds);
}

void MainWindow::onDurationError(const QString &error) {
  durationProbe_->deleteLater();
  durationProbe_ = nullptr;

  setStatus(tr("Error: %1").arg(error));
  setUiEnabled(true);
}

void MainWindow::onRenderFinished() {
  renderer_->deleteLater();
  renderer_ = nullptr;

  setStatus(tr("Done! Saved to: %1").arg(pendingOutputPath_));
  setUiEnabled(true);
}

void MainWindow::onRenderError(const QString &error) {
  renderer_->deleteLater();
  renderer_ = nullptr;

  setStatus(tr("Error: %1").arg(error));
  setUiEnabled(true);
}

void MainWindow::setStatus(const QString &message) {
  statusLabel_->setText(message);
}

void MainWindow::setUiEnabled(bool enabled) {
  visualInput_->setEnabled(enabled);
  audioInput_->setEnabled(enabled);
  outputInput_->setEnabled(enabled);
  generateButton_->setEnabled(enabled);
}

} // namespace ui
