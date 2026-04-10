#pragma once

#include "core/render_profile.h"

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

  core::QualityPreset qualityPreset() const;
  void setQualityPreset(core::QualityPreset preset);

  core::RenderProfile activeProfile() const;

  void resetToDefaults();

private:
  static QString defaultOutputDir();
};

} // namespace settings
