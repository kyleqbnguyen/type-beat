#include "settings/app_settings.h"

#include <QSettings>
#include <QStandardPaths>

namespace {

const QString kOutputDir = QStringLiteral("output_dir");
const QString kLastVisualPath = QStringLiteral("last_visual_path");
const QString kLastAudioPath = QStringLiteral("last_audio_path");

QSettings &getSettings() {
  static QSettings settings(QStringLiteral("TypeBeat"),
                            QStringLiteral("TypeBeat"));
  return settings;
}

} // namespace

namespace settings {

AppSettings::AppSettings() = default;

QString AppSettings::outputDir() const {
  return getSettings().value(kOutputDir, defaultOutputDir()).toString();
}

void AppSettings::setOutputDir(const QString &path) {
  getSettings().setValue(kOutputDir, path);
}

QString AppSettings::lastVisualPath() const {
  return getSettings().value(kLastVisualPath).toString();
}

void AppSettings::setLastVisualPath(const QString &path) {
  getSettings().setValue(kLastVisualPath, path);
}

QString AppSettings::lastAudioPath() const {
  return getSettings().value(kLastAudioPath).toString();
}

void AppSettings::setLastAudioPath(const QString &path) {
  getSettings().setValue(kLastAudioPath, path);
}

QString AppSettings::defaultOutputDir() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  if (path.isEmpty()) {
    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  }
  return path;
}

} // namespace settings
