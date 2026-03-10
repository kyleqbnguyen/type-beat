#include "core/ffprobe.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace core::ffprobe {

DurationProbe::DurationProbe(QObject *parent) : QObject(parent) {
  connect(&process_, &QProcess::finished, this,
          &DurationProbe::onProcessFinished);
  connect(&process_, &QProcess::errorOccurred, this,
          &DurationProbe::onProcessError);
}

void DurationProbe::probe(const QString &filePath) {
  QString ffprobe{ffprobePath()};

  if (!QFile::exists(ffprobe)) {
    emit errorOccurred(QStringLiteral("ffprobe not found at: %1").arg(ffprobe));
    return;
  }

  if (!QFile::exists(filePath)) {
    emit errorOccurred(QStringLiteral("File not found: %1").arg(filePath));
    return;
  }

  QStringList args{
      "-v", "quiet", "-print_format", "json", "-show_format", filePath,
  };

  process_.start(ffprobe, args);
}

void DurationProbe::onProcessFinished(int exitCode,
                                      QProcess::ExitStatus exitStatus) {
  if (exitStatus == QProcess::CrashExit) {
    emit errorOccurred(QStringLiteral("ffprobe process crashed"));
    return;
  }

  if (exitCode != 0) {
    emit errorOccurred(
        QStringLiteral("ffprobe failed with exit code: %1").arg(exitCode));
    return;
  }

  QByteArray output{process_.readAllStandardOutput()};
  QJsonParseError parseError{};
  QJsonDocument doc{QJsonDocument::fromJson(output, &parseError)};

  if (parseError.error != QJsonParseError::NoError) {
    emit errorOccurred(QStringLiteral("Failed to parse ffprobe output: %1")
                           .arg(parseError.errorString()));
    return;
  }

  QJsonObject root{doc.object()};
  QJsonObject format{root.value(QStringLiteral("format")).toObject()};

  if (!format.contains(QStringLiteral("duration"))) {
    emit errorOccurred(QStringLiteral("Duration not found in ffprobe output"));
    return;
  }

  QString durationStr{format.value(QStringLiteral("duration")).toString()};
  bool ok{false};
  double seconds{durationStr.toDouble(&ok)};

  if (!ok) {
    emit errorOccurred(
        QStringLiteral("Failed to parse duration value: %1").arg(durationStr));
    return;
  }

  emit durationReady(seconds);
}

void DurationProbe::onProcessError(QProcess::ProcessError error) {
  QString message{};

  switch (error) {
  case QProcess::FailedToStart:
    message = QStringLiteral("Failed to start ffprobe");
    break;
  case QProcess::Crashed:
    message = QStringLiteral("ffprobe process crashed");
    break;
  case QProcess::Timedout:
    message = QStringLiteral("ffprobe process timed out");
    break;
  case QProcess::WriteError:
    message = QStringLiteral("Failed to write to ffprobe process");
    break;
  case QProcess::ReadError:
    message = QStringLiteral("Failed to read from ffprobe process");
    break;
  case QProcess::UnknownError:
  default:
    message = QStringLiteral("Unknown error occurred with ffprobe");
    break;
  }

  emit errorOccurred(message);
}

QString DurationProbe::ffprobePath() {
  return QCoreApplication::applicationDirPath() + QStringLiteral("/ffprobe");
}

} // namespace core::ffprobe
