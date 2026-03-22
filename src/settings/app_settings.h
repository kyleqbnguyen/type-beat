#pragma once

#include <QString>

namespace settings {

class AppSettings {
public:
  AppSettings();

  QString outputDir() const;
  void setOutputDir(const QString &path);

  QString lastVisualPath() const;
  void setLastVisualPath(const QString &path);

  QString lastAudioPath() const;
  void setLastAudioPath(const QString &path);

  QByteArray windowGeometry() const;
  void setWindowGeometry(const QByteArray &geometry);

  void resetToDefaults();

private:
  static QString defaultOutputDir();
};

} // namespace settings
