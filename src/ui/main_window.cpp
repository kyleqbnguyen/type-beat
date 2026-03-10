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

const QString visualFilter =
    QStringLiteral("Visual (*.png *.jpg *.jpeg *.mp4 *.mov)");
const QString audioFilter = QStringLiteral("Audio (*.mp3 *.wav)");
const QString outputFilter = QStringLiteral("Video (*.mp4)");

constexpr int windowWidth = 550;
constexpr int windowHeight = 240;
constexpr int windowMargin = 24;
constexpr int formRowSpacing = 16;
constexpr int buttonTopSpacing = 24;
constexpr int statusTopSpacing = 12;
constexpr int buttonHeight = 40;

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), visualInput_(nullptr), audioInput_(nullptr),
      outputInput_(nullptr), generateButton_(nullptr), statusLabel_(nullptr),
      progressBar_(nullptr), renderer_(nullptr), outputManuallyEdited_(false),
      settingOutputProgrammatically_(false) {
  setupUi();
  setWindowTitle(tr("Type Beat Generator"));
  resize(windowWidth, windowHeight);
}

void MainWindow::setupUi() {
  auto *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  auto *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(windowMargin, windowMargin, windowMargin,
                                 windowMargin);

  auto *formLayout = new QFormLayout();
  formLayout->setVerticalSpacing(formRowSpacing);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

  visualInput_ = new FileInput(visualFilter, FileInput::Mode::Open, this);
  visualInput_->setPath(settings_.lastVisualPath());
  formLayout->addRow(tr("Visual"), visualInput_);

  audioInput_ = new FileInput(audioFilter, FileInput::Mode::Open, this);
  audioInput_->setPath(settings_.lastAudioPath());
  formLayout->addRow(tr("Audio"), audioInput_);

  outputInput_ = new FileInput(outputFilter, FileInput::Mode::Save, this);
  formLayout->addRow(tr("Output"), outputInput_);

  mainLayout->addLayout(formLayout);

  mainLayout->addSpacing(buttonTopSpacing);

  generateButton_ = new QPushButton(tr("Generate"), this);
  generateButton_->setEnabled(false);
  generateButton_->setMinimumHeight(buttonHeight);
  mainLayout->addWidget(generateButton_);

  mainLayout->addSpacing(statusTopSpacing);

  statusLabel_ = new QLabel(tr("Ready"), this);
  mainLayout->addWidget(statusLabel_);

  progressBar_ = new QProgressBar(this);
  progressBar_->setMinimum(0);
  progressBar_->setMaximum(0);
  progressBar_->setVisible(false);
  mainLayout->addWidget(progressBar_);

  mainLayout->addStretch();

  connect(visualInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(audioInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(outputInput_, &FileInput::pathChanged, this,
          &MainWindow::updateGenerateButton);
  connect(generateButton_, &QPushButton::clicked, this,
          &MainWindow::onGenerateClicked);

  auto updateOutputPath = [this]() {
    if (outputManuallyEdited_) {
      return;
    }
    QString visualPath = visualInput_->path();
    QString audioPath = audioInput_->path();
    if (!visualPath.isEmpty() && QFileInfo::exists(visualPath) &&
        !audioPath.isEmpty() && QFileInfo::exists(audioPath)) {
      QString outputPath =
          core::file::generateOutputPath(audioPath, settings_.outputDir());
      settingOutputProgrammatically_ = true;
      outputInput_->setPath(outputPath);
      settingOutputProgrammatically_ = false;
    }
  };

  connect(visualInput_, &FileInput::pathChanged, this, updateOutputPath);
  connect(audioInput_, &FileInput::pathChanged, this, updateOutputPath);

  connect(outputInput_, &FileInput::pathChanged, this, [this]() {
    if (!settingOutputProgrammatically_) {
      outputManuallyEdited_ = true;
    }
  });

  updateGenerateButton();
  updateOutputPath();
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
  QString visualPath = visualInput_->path();
  QString audioPath = audioInput_->path();
  pendingOutputPath_ = outputInput_->path();

  if (QFileInfo::exists(pendingOutputPath_)) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("File Exists"),
        tr("The file '%1' already exists. Do you want to overwrite it?")
            .arg(pendingOutputPath_),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) {
      return;
    }
  }

  settings_.setLastVisualPath(visualPath);
  settings_.setLastAudioPath(audioPath);
  settings_.setOutputDir(QFileInfo(pendingOutputPath_).absolutePath());

  setUiEnabled(false);
  setStatus(tr("Rendering video..."));
  progressBar_->setVisible(true);

  renderer_ = new core::ffmpeg::Renderer(this);
  connect(renderer_, &core::ffmpeg::Renderer::finished, this,
          &MainWindow::onRenderFinished);
  connect(renderer_, &core::ffmpeg::Renderer::errorOccurred, this,
          &MainWindow::onRenderError);

  renderer_->render(visualPath, audioPath, pendingOutputPath_);
}

void MainWindow::onRenderFinished() {
  renderer_->deleteLater();
  renderer_ = nullptr;

  setStatus(tr("Done! Saved to: %1").arg(pendingOutputPath_));
  progressBar_->setVisible(false);
  setUiEnabled(true);
}

void MainWindow::onRenderError(const QString &error) {
  renderer_->deleteLater();
  renderer_ = nullptr;

  setStatus(tr("Error: %1").arg(error));
  progressBar_->setVisible(false);
  setUiEnabled(true);
}

void MainWindow::setStatus(const QString &message) {
  statusLabel_->setText(message);
}

void MainWindow::setUiEnabled(bool enabled) {
  visualInput_->setEnabled(enabled);
  audioInput_->setEnabled(enabled);
  outputInput_->setEnabled(enabled);
  if (enabled) {
    updateGenerateButton();
  } else {
    generateButton_->setEnabled(false);
  }
}

} // namespace ui
