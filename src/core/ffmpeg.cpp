#include "core/ffmpeg.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(lcRenderer, "typebeat.renderer")

namespace core::ffmpeg {

Renderer::Renderer(QObject *parent) : QObject(parent) {
  connect(&process_, &QProcess::finished, this, &Renderer::onProcessFinished);
  connect(&process_, &QProcess::errorOccurred, this, &Renderer::onProcessError);
  connect(&process_, &QProcess::readyReadStandardError, this,
          &Renderer::onReadyReadStderr);
  process_.setProcessChannelMode(QProcess::SeparateChannels);
}

Renderer::~Renderer() {
  if (process_.state() != QProcess::NotRunning) {
    qCWarning(lcRenderer) << "Renderer destroyed while process running, "
                             "killing FFmpeg";
    process_.kill();
    process_.waitForFinished(3000);
  }
}

QString Renderer::fullStderr() const { return fullStderr_; }

void Renderer::render(const QString &visualPath, const QString &audioPath,
                      const QString &outputPath, const RenderConfig &config) {
  if (process_.state() != QProcess::NotRunning) {
    emit errorOccurred(QStringLiteral("Render already in progress"));
    return;
  }

  errorEmitted_ = false;
  cancelled_ = false;
  durationSeconds_ = 0.0;
  fullStderr_.clear();
  config_ = config;

  QString ffmpeg = ffmpegPath();

  if (!QFile::exists(ffmpeg)) {
    emit errorOccurred(QStringLiteral("ffmpeg not found at: %1").arg(ffmpeg));
    return;
  }

  if (!QFile::exists(visualPath)) {
    emit errorOccurred(
        QStringLiteral("Visual file not found: %1").arg(visualPath));
    return;
  }

  if (!QFile::exists(audioPath)) {
    emit errorOccurred(
        QStringLiteral("Audio file not found: %1").arg(audioPath));
    return;
  }

  QStringList args;

  if (isImageFile(visualPath)) {
    args = buildImageArgs(visualPath, audioPath, outputPath);
  } else {
    args = buildVideoArgs(visualPath, audioPath, outputPath);
  }

  qCInfo(lcRenderer) << "Starting FFmpeg:" << ffmpeg << args.join(' ');

  process_.start(ffmpeg, args);
}

void Renderer::cancel() {
  if (process_.state() == QProcess::NotRunning) {
    return;
  }
  cancelled_ = true;
  qCInfo(lcRenderer) << "Cancelling render";
  process_.terminate();
  if (!process_.waitForFinished(5000)) {
    qCWarning(lcRenderer) << "FFmpeg did not terminate gracefully, killing";
    process_.kill();
    process_.waitForFinished(3000);
  }
}

bool Renderer::isRunning() const {
  return process_.state() != QProcess::NotRunning;
}

void Renderer::onReadyReadStderr() {
  QString output = QString::fromUtf8(process_.readAllStandardError());
  fullStderr_.append(output);

  if (durationSeconds_ <= 0.0) {
    double dur = parseDuration(fullStderr_);
    if (dur > 0.0) {
      durationSeconds_ = dur;
    }
  }

  if (durationSeconds_ > 0.0) {
    double currentTime = parseTime(output);
    if (currentTime >= 0.0) {
      int percent = static_cast<int>((currentTime / durationSeconds_) * 100.0);
      if (percent > 100) {
        percent = 100;
      }
      emit progressUpdated(percent);
    }
  }
}

void Renderer::onProcessFinished(int exitCode,
                                 QProcess::ExitStatus exitStatus) {
  fullStderr_.append(QString::fromUtf8(process_.readAllStandardError()));

  if (errorEmitted_) {
    return;
  }

  if (cancelled_) {
    errorEmitted_ = true;
    emit errorOccurred(QStringLiteral("Render cancelled"));
    return;
  }

  if (exitStatus == QProcess::CrashExit) {
    errorEmitted_ = true;
    qCWarning(lcRenderer) << "FFmpeg crashed. Full output:" << fullStderr_;
    emit errorOccurred(QStringLiteral("ffmpeg process crashed"));
    return;
  }

  if (exitCode != 0) {
    errorEmitted_ = true;
    qCWarning(lcRenderer) << "FFmpeg failed with exit code" << exitCode
                          << "Full output:" << fullStderr_;
    emit errorOccurred(
        QStringLiteral("ffmpeg failed (exit code %1)").arg(exitCode));
    return;
  }

  qCInfo(lcRenderer) << "Render completed successfully";
  emit finished();
}

void Renderer::onProcessError(QProcess::ProcessError error) {
  if (errorEmitted_) {
    return;
  }
  errorEmitted_ = true;

  QString message;

  switch (error) {
  case QProcess::FailedToStart:
    message = QStringLiteral("Failed to start ffmpeg");
    break;
  case QProcess::Crashed:
    if (cancelled_) {
      message = QStringLiteral("Render cancelled");
    } else {
      message = QStringLiteral("ffmpeg process crashed");
    }
    break;
  case QProcess::Timedout:
    message = QStringLiteral("ffmpeg process timed out");
    break;
  case QProcess::WriteError:
    message = QStringLiteral("Failed to write to ffmpeg process");
    break;
  case QProcess::ReadError:
    message = QStringLiteral("Failed to read from ffmpeg process");
    break;
  case QProcess::UnknownError:
  default:
    message = QStringLiteral("Unknown error occurred with ffmpeg");
    break;
  }

  qCWarning(lcRenderer) << "FFmpeg error:" << message;
  emit errorOccurred(message);
}

double Renderer::parseDuration(const QString &output) {
  static const QRegularExpression re(QStringLiteral(
      R"(Duration:\s*(\d{1,2}):(\d{1,2}):(\d{1,2})\.(\d{1,2}))"));
  double maxDuration = -1.0;
  auto it = re.globalMatch(output);
  while (it.hasNext()) {
    auto match = it.next();
    double hours = match.captured(1).toDouble();
    double minutes = match.captured(2).toDouble();
    double seconds = match.captured(3).toDouble();
    double centis = match.captured(4).toDouble();
    double dur = hours * 3600.0 + minutes * 60.0 + seconds + centis / 100.0;
    if (dur > maxDuration) {
      maxDuration = dur;
    }
  }
  return maxDuration;
}

double Renderer::parseTime(const QString &line) {
  static const QRegularExpression re(
      QStringLiteral(R"(time=(\d{1,2}):(\d{1,2}):(\d{1,2})\.(\d{1,2}))"));
  double lastTime = -1.0;
  auto it = re.globalMatch(line);
  while (it.hasNext()) {
    auto match = it.next();
    double hours = match.captured(1).toDouble();
    double minutes = match.captured(2).toDouble();
    double seconds = match.captured(3).toDouble();
    double centis = match.captured(4).toDouble();
    lastTime = hours * 3600.0 + minutes * 60.0 + seconds + centis / 100.0;
  }
  return lastTime;
}

QString Renderer::ffmpegPath() {
#ifdef Q_OS_WIN
  return QDir(QCoreApplication::applicationDirPath())
      .filePath(QStringLiteral("ffmpeg.exe"));
#else
  return QDir(QCoreApplication::applicationDirPath())
      .filePath(QStringLiteral("ffmpeg"));
#endif
}

bool Renderer::isImageFile(const QString &path) {
  QString ext = QFileInfo(path).suffix().toLower();
  return ext == QStringLiteral("png") || ext == QStringLiteral("jpg") ||
         ext == QStringLiteral("jpeg") || ext == QStringLiteral("gif") ||
         ext == QStringLiteral("bmp") || ext == QStringLiteral("webp");
}

QStringList Renderer::buildImageArgs(const QString &visualPath,
                                     const QString &audioPath,
                                     const QString &outputPath) const {
  QString scaleFilter =
      QStringLiteral("scale=%1:%2:force_original_aspect_ratio=decrease,"
                     "pad=%1:%2:(ow-iw)/2:(oh-ih)/2")
          .arg(config_.width)
          .arg(config_.height);
  return {
      QStringLiteral("-y"),
      QStringLiteral("-loop"),
      QStringLiteral("1"),
      QStringLiteral("-i"),
      visualPath,
      QStringLiteral("-i"),
      audioPath,
      QStringLiteral("-map"),
      QStringLiteral("0:v:0"),
      QStringLiteral("-map"),
      QStringLiteral("1:a:0"),
      QStringLiteral("-c:v"),
      config_.videoCodec,
      QStringLiteral("-tune"),
      QStringLiteral("stillimage"),
      QStringLiteral("-preset"),
      config_.preset,
      QStringLiteral("-crf"),
      QString::number(config_.imageCrf),
      QStringLiteral("-c:a"),
      config_.audioCodec,
      QStringLiteral("-b:a"),
      config_.audioBitrate,
      QStringLiteral("-pix_fmt"),
      QStringLiteral("yuv420p"),
      QStringLiteral("-shortest"),
      QStringLiteral("-vf"),
      scaleFilter,
      outputPath,
  };
}

QStringList Renderer::buildVideoArgs(const QString &visualPath,
                                     const QString &audioPath,
                                     const QString &outputPath) const {
  QString scaleFilter =
      QStringLiteral("scale=%1:%2:force_original_aspect_ratio=decrease,"
                     "pad=%1:%2:(ow-iw)/2:(oh-ih)/2")
          .arg(config_.width)
          .arg(config_.height);
  return {
      QStringLiteral("-y"),
      QStringLiteral("-stream_loop"),
      QStringLiteral("-1"),
      QStringLiteral("-i"),
      visualPath,
      QStringLiteral("-i"),
      audioPath,
      QStringLiteral("-map"),
      QStringLiteral("0:v:0"),
      QStringLiteral("-map"),
      QStringLiteral("1:a:0"),
      QStringLiteral("-c:v"),
      config_.videoCodec,
      QStringLiteral("-preset"),
      config_.preset,
      QStringLiteral("-crf"),
      QString::number(config_.videoCrf),
      QStringLiteral("-c:a"),
      config_.audioCodec,
      QStringLiteral("-b:a"),
      config_.audioBitrate,
      QStringLiteral("-pix_fmt"),
      QStringLiteral("yuv420p"),
      QStringLiteral("-shortest"),
      QStringLiteral("-vf"),
      scaleFilter,
      outputPath,
  };
}

} // namespace core::ffmpeg
