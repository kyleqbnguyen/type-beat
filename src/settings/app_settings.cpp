#include "settings/app_settings.h"

#include <QLoggingCategory>
#include <QSettings>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(lcSettings, "typebeat.settings")

namespace {

const QString kOutputDir = QStringLiteral("output_dir");
const QString kLastVisualPath = QStringLiteral("last_visual_path");
const QString kLastAudioPath = QStringLiteral("last_audio_path");
const QString kWindowGeometry = QStringLiteral("window_geometry");

QSettings &getSettings() {
  static QSettings settings(QStringLiteral("TypeBeat"),
                            QStringLiteral("TypeBeat"));
  if (settings.status() != QSettings::NoError) {
    qCWarning(lcSettings) << "QSettings error status:" << settings.status();
  }
  return settings;
}

} // namespace

namespace settings {

AppSettings::AppSettings() {
  // Check settings health on construction
  auto &s = getSettings();
  if (s.status() != QSettings::NoError) {
    qCWarning(lcSettings) << "Settings file may be corrupted or unreadable";
  }
}

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

QByteArray AppSettings::windowGeometry() const {
  return getSettings().value(kWindowGeometry).toByteArray();
}

void AppSettings::setWindowGeometry(const QByteArray &geometry) {
  getSettings().setValue(kWindowGeometry, geometry);
}

void AppSettings::resetToDefaults() {
  qCInfo(lcSettings) << "Resetting all settings to defaults";
  getSettings().clear();
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
