#include "core/ffmpeg.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace core::ffmpeg {

namespace {

const QString scaleFilter =
    QStringLiteral("scale=1920:1080:force_original_aspect_ratio=decrease,"
                   "pad=1920:1080:(ow-iw)/2:(oh-ih)/2");

} // namespace

Renderer::Renderer(QObject *parent) : QObject(parent) {
  connect(&process_, &QProcess::finished, this, &Renderer::onProcessFinished);
  connect(&process_, &QProcess::errorOccurred, this, &Renderer::onProcessError);
}

void Renderer::render(const QString &visualPath, const QString &audioPath,
                      const QString &outputPath, double audioDuration) {
  if (process_.state() != QProcess::NotRunning) {
    emit errorOccurred(QStringLiteral("Render already in progress"));
    return;
  }

  errorEmitted_ = false;

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

  QStringList args{};

  if (isImageFile(visualPath)) {
    args = buildImageArgs(visualPath, audioPath, outputPath);
  } else {
    args = buildVideoArgs(visualPath, audioPath, outputPath, audioDuration);
  }

  process_.start(ffmpeg, args);
}

void Renderer::onProcessFinished(int exitCode,
                                 QProcess::ExitStatus exitStatus) {
  if (errorEmitted_) {
    return;
  }

  if (exitStatus == QProcess::CrashExit) {
    errorEmitted_ = true;
    emit errorOccurred(QStringLiteral("ffmpeg process crashed"));
    return;
  }

  if (exitCode != 0) {
    errorEmitted_ = true;
    QString stderrOutput = QString::fromUtf8(process_.readAllStandardError());
    emit errorOccurred(QStringLiteral("ffmpeg failed with exit code %1: %2")
                           .arg(exitCode)
                           .arg(stderrOutput.right(500)));
    return;
  }

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
    message = QStringLiteral("ffmpeg process crashed");
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

  emit errorOccurred(message);
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

// ffmpeg -loop 1 -framerate 1 -i image.png -i audio.mp3
//   -c:v libx264 -tune stillimage -preset ultrafast -crf 18
//   -c:a aac -b:a 192k -pix_fmt yuv420p -r 1 -shortest
//   -vf "scale=..." output.mp4
QStringList Renderer::buildImageArgs(const QString &visualPath,
                                     const QString &audioPath,
                                     const QString &outputPath) const {
  return {
      QStringLiteral("-y"),
      QStringLiteral("-loop"),
      QStringLiteral("1"),
      QStringLiteral("-framerate"),
      QStringLiteral("1"),
      QStringLiteral("-i"),
      visualPath,
      QStringLiteral("-i"),
      audioPath,
      QStringLiteral("-c:v"),
      QStringLiteral("libx264"),
      QStringLiteral("-tune"),
      QStringLiteral("stillimage"),
      QStringLiteral("-preset"),
      QStringLiteral("ultrafast"),
      QStringLiteral("-crf"),
      QStringLiteral("18"),
      QStringLiteral("-c:a"),
      QStringLiteral("aac"),
      QStringLiteral("-b:a"),
      QStringLiteral("192k"),
      QStringLiteral("-pix_fmt"),
      QStringLiteral("yuv420p"),
      QStringLiteral("-r"),
      QStringLiteral("1"),
      QStringLiteral("-shortest"),
      QStringLiteral("-vf"),
      scaleFilter,
      outputPath,
  };
}

// ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3
//   -map 0:v -map 1:a -c:v libx264 -preset ultrafast -crf 23
//   -c:a aac -b:a 192k -pix_fmt yuv420p -t <audio_duration>
//   -vf "scale=..." output.mp4
QStringList Renderer::buildVideoArgs(const QString &visualPath,
                                     const QString &audioPath,
                                     const QString &outputPath,
                                     double audioDuration) const {
  return {
      QStringLiteral("-y"),
      QStringLiteral("-stream_loop"),
      QStringLiteral("-1"),
      QStringLiteral("-i"),
      visualPath,
      QStringLiteral("-i"),
      audioPath,
      QStringLiteral("-map"),
      QStringLiteral("0:v"),
      QStringLiteral("-map"),
      QStringLiteral("1:a"),
      QStringLiteral("-c:v"),
      QStringLiteral("libx264"),
      QStringLiteral("-preset"),
      QStringLiteral("ultrafast"),
      QStringLiteral("-crf"),
      QStringLiteral("23"),
      QStringLiteral("-c:a"),
      QStringLiteral("aac"),
      QStringLiteral("-b:a"),
      QStringLiteral("192k"),
      QStringLiteral("-pix_fmt"),
      QStringLiteral("yuv420p"),
      QStringLiteral("-t"),
      QString::number(audioDuration, 'f', 3),
      QStringLiteral("-vf"),
      scaleFilter,
      outputPath,
  };
}

} // namespace core::ffmpeg
