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

private:
  static QString defaultOutputDir();
};

} // namespace settings
