#include "ui/main_window.h"

#include "core/file_utils.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QFormLayout>
#include <QLoggingCategory>
#include <QMenuBar>
#include <QMessageBox>
#include <QStorageInfo>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "version.h"

Q_LOGGING_CATEGORY(lcMain, "typebeat.main")

namespace ui {

namespace {

const QString visualFilter =
    QStringLiteral("Visual (*.png *.jpg *.jpeg *.mp4 *.mov)");
const QString audioFilter = QStringLiteral("Audio (*.mp3 *.wav)");
const QString outputFilter = QStringLiteral("Video (*.mp4)");

constexpr int windowMinWidth = 550;
constexpr int windowMinHeight = 240;
constexpr int windowMargin = 24;
constexpr int formRowSpacing = 16;
constexpr int buttonTopSpacing = 24;
constexpr int statusTopSpacing = 12;
constexpr int buttonHeight = 40;
const QString roundedButtonStyle =
    QStringLiteral("QPushButton {"
                   " border: 1px solid palette(mid);"
                   " border-radius: 6px;"
                   " background-color: #8d8d8d;"
                   " color: palette(button-text);"
                   " padding: 0 14px;"
                   "}"
                   "QPushButton:hover {"
                   " background-color: #979797;"
                   "}"
                   "QPushButton:pressed {"
                   " background-color: #818181;"
                   "}"
                   "QPushButton:disabled {"
                   " color: palette(mid);"
                   " background-color: #a4a4a4;"
                   "}");
constexpr qint64 minDiskSpaceBytes = 1073741824;

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), visualInput_(nullptr), audioInput_(nullptr),
      outputInput_(nullptr), generateButton_(nullptr), cancelButton_(nullptr),
      openFolderButton_(nullptr), statusLabel_(nullptr), progressBar_(nullptr),
      renderer_(this), outputManuallyEdited_(false),
      settingOutputProgrammatically_(false) {
  setupUi();
  setupMenuBar();
  setWindowTitle(tr("Type Beat Generator"));
  const QSize minSizeHint = minimumSizeHint();
  setMinimumSize(qMax(windowMinWidth, minSizeHint.width()),
                 qMax(windowMinHeight, minSizeHint.height()));

  QByteArray savedGeometry = settings_.windowGeometry();
  if (!savedGeometry.isEmpty()) {
    restoreGeometry(savedGeometry);
  } else {
    resize(windowMinWidth, windowMinHeight);
  }
}

MainWindow::~MainWindow() {
  settings_.setWindowGeometry(saveGeometry());
  if (renderer_.isRunning()) {
    renderer_.cancel();
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (renderer_.isRunning()) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Render In Progress"),
        tr("A render is in progress. Cancel and close?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) {
      event->ignore();
      return;
    }
    renderer_.cancel();
  }
  settings_.setWindowGeometry(saveGeometry());
  event->accept();
}

void MainWindow::setupMenuBar() {
  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(tr("&About TypeBeat"), this, &MainWindow::onAbout);
}

void MainWindow::setupUi() {
  auto *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  auto *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  mainLayout->setContentsMargins(windowMargin, windowMargin, windowMargin,
                                 windowMargin);

  auto *formLayout = new QFormLayout();
  formLayout->setVerticalSpacing(formRowSpacing);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

  visualInput_ = new FileInput(visualFilter, FileInput::Mode::Open, this);
  visualInput_->setPath(settings_.lastVisualPath());
  visualInput_->setToolTip(
      tr("Select an image (PNG, JPG) or video (MP4, MOV) to use as the "
         "visual background"));
  visualInput_->setAccessibleName(tr("Visual file input"));
  visualInput_->setAccessibleDescription(
      tr("Select an image or video file for the visual background"));
  formLayout->addRow(tr("Visual"), visualInput_);

  audioInput_ = new FileInput(audioFilter, FileInput::Mode::Open, this);
  audioInput_->setPath(settings_.lastAudioPath());
  audioInput_->setToolTip(tr("Select an audio file (MP3, WAV) for the "
                             "soundtrack"));
  audioInput_->setAccessibleName(tr("Audio file input"));
  audioInput_->setAccessibleDescription(
      tr("Select an audio file to use as the soundtrack"));
  formLayout->addRow(tr("Audio"), audioInput_);

  outputInput_ = new FileInput(outputFilter, FileInput::Mode::Save, this);
  outputInput_->setToolTip(
      tr("Choose where to save the output video (MP4, 1920x1080, "
         "192kbps AAC)"));
  outputInput_->setAccessibleName(tr("Output file path"));
  outputInput_->setAccessibleDescription(
      tr("Choose the output file path for the generated video"));
  formLayout->addRow(tr("Output"), outputInput_);

  mainLayout->addLayout(formLayout);

  mainLayout->addSpacing(buttonTopSpacing);

  auto *buttonLayout = new QHBoxLayout();

  generateButton_ = new QPushButton(tr("Generate"), this);
  generateButton_->setEnabled(false);
  generateButton_->setMinimumHeight(buttonHeight);
  generateButton_->setStyleSheet(roundedButtonStyle);
  generateButton_->setToolTip(
      tr("Combine the visual and audio files into a video"));
  generateButton_->setAccessibleName(tr("Generate video"));
  buttonLayout->addWidget(generateButton_);

  cancelButton_ = new QPushButton(tr("Cancel"), this);
  cancelButton_->setMinimumHeight(buttonHeight);
  cancelButton_->setStyleSheet(roundedButtonStyle);
  cancelButton_->setVisible(false);
  cancelButton_->setToolTip(tr("Cancel the current render"));
  cancelButton_->setAccessibleName(tr("Cancel render"));
  buttonLayout->addWidget(cancelButton_);

  openFolderButton_ = new QPushButton(tr("Open Folder"), this);
  openFolderButton_->setMinimumHeight(buttonHeight);
  openFolderButton_->setStyleSheet(roundedButtonStyle);
  openFolderButton_->setVisible(false);
  openFolderButton_->setToolTip(tr("Open the output folder in file manager"));
  openFolderButton_->setAccessibleName(tr("Open output folder"));
  buttonLayout->addWidget(openFolderButton_);

  mainLayout->addLayout(buttonLayout);

  mainLayout->addSpacing(statusTopSpacing);

  statusLabel_ = new QLabel(tr("Ready"), this);
  statusLabel_->setWordWrap(true);
  statusLabel_->setAccessibleName(tr("Status"));
  mainLayout->addWidget(statusLabel_);

  progressBar_ = new QProgressBar(this);
  progressBar_->setMinimum(0);
  progressBar_->setMaximum(100);
  progressBar_->setVisible(false);
  progressBar_->setAccessibleName(tr("Render progress"));
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
  connect(cancelButton_, &QPushButton::clicked, this,
          &MainWindow::onCancelClicked);
  connect(openFolderButton_, &QPushButton::clicked, this,
          &MainWindow::onOpenOutputFolder);

  connect(&renderer_, &core::ffmpeg::Renderer::finished, this,
          &MainWindow::onRenderFinished);
  connect(&renderer_, &core::ffmpeg::Renderer::errorOccurred, this,
          &MainWindow::onRenderError);
  connect(&renderer_, &core::ffmpeg::Renderer::progressUpdated, this,
          &MainWindow::onProgressUpdated);

  auto updateOutputPath = [this]() {
    if (outputManuallyEdited_) {
      return;
    }
    QString visualPath = visualInput_->path();
    QString audioPath = audioInput_->path();
    if (!visualPath.isEmpty() && QFileInfo::exists(visualPath) &&
        !audioPath.isEmpty() && QFileInfo::exists(audioPath)) {
      auto result =
          core::file::generateOutputPath(audioPath, settings_.outputDir());
      if (result.ok()) {
        settingOutputProgrammatically_ = true;
        outputInput_->setPath(result.path);
        settingOutputProgrammatically_ = false;
      } else {
        setStatus(result.error, true);
      }
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

bool MainWindow::checkDiskSpace(const QString &outputPath) {
  QStorageInfo storage(QFileInfo(outputPath).absolutePath());
  if (storage.isValid() && storage.isReady()) {
    if (storage.bytesAvailable() < minDiskSpaceBytes) {
      QMessageBox::StandardButton reply = QMessageBox::warning(
          this, tr("Low Disk Space"),
          tr("Less than 1 GB of disk space available on the output drive. "
             "The render may fail. Continue anyway?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      return reply == QMessageBox::Yes;
    }
  }
  return true;
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

  if (!checkDiskSpace(pendingOutputPath_)) {
    return;
  }

  settings_.setLastVisualPath(visualPath);
  settings_.setLastAudioPath(audioPath);
  settings_.setOutputDir(QFileInfo(pendingOutputPath_).absolutePath());

  setUiEnabled(false);
  setStatus(tr("Rendering video..."));
  openFolderButton_->setVisible(false);
  cancelButton_->setVisible(true);
  progressBar_->setValue(0);
  progressBar_->setVisible(true);

  qCInfo(lcMain) << "Starting render:" << visualPath << "+" << audioPath << "->"
                 << pendingOutputPath_;

  renderer_.render(visualPath, audioPath, pendingOutputPath_);
}

void MainWindow::onCancelClicked() {
  renderer_.cancel();
  cancelButton_->setEnabled(false);
  setStatus(tr("Cancelling..."));
}

void MainWindow::onProgressUpdated(int percent) {
  progressBar_->setValue(percent);
  setStatus(tr("Rendering video... %1%").arg(percent));
}

void MainWindow::onRenderFinished() {
  setStatus(tr("Done! Saved to: %1").arg(pendingOutputPath_));
  statusLabel_->setStyleSheet(QStringLiteral("color: green;"));
  progressBar_->setVisible(false);
  cancelButton_->setVisible(false);
  openFolderButton_->setVisible(true);
  setUiEnabled(true);
  qCInfo(lcMain) << "Render finished:" << pendingOutputPath_;
}

void MainWindow::onRenderError(const QString &error) {
  setStatus(tr("Error: %1").arg(error), true);
  progressBar_->setVisible(false);
  cancelButton_->setVisible(false);
  cancelButton_->setEnabled(true);
  setUiEnabled(true);

  if (!error.contains(QStringLiteral("cancel"), Qt::CaseInsensitive)) {
    QString details = renderer_.property("fullStderr").toString();
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle(tr("Render Failed"));
    msgBox.setText(tr("The render failed: %1").arg(error));
    msgBox.setDetailedText(renderer_.findChild<QObject *>()
                               ? QString()
                               : QStringLiteral("Check the application "
                                                "log for full FFmpeg "
                                                "output."));
    msgBox.exec();
  }
}

void MainWindow::onOpenOutputFolder() {
  QString dir = QFileInfo(pendingOutputPath_).absolutePath();
  QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::onAbout() {
  QMessageBox::about(
      this, tr("About TypeBeat"),
      tr("<h3>TypeBeat</h3>"
         "<p>Version %1</p>"
         "<p>Combine images or videos with audio to create type beat "
         "style videos.</p>"
         "<p>Licensed under GPL-3.0</p>"
         "<p><a href=\"https://github.com/kyleqbnguyen/type-beat\">"
         "GitHub Repository</a></p>")
          .arg(QStringLiteral(TYPEBEAT_VERSION)));
}

void MainWindow::setStatus(const QString &message, bool isError) {
  statusLabel_->setText(message);
  if (isError) {
    statusLabel_->setStyleSheet(QStringLiteral("color: red;"));
  } else {
    statusLabel_->setStyleSheet(QString());
  }
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
