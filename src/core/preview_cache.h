#pragma once

#include "core/render_profile.h"

#include <QString>

namespace core {

struct PreviewRequest {
  QString visualPath;
  QString audioPath;
  RenderProfile profile;
};

class PreviewCache {
public:
  PreviewCache();
  explicit PreviewCache(const QString &cacheDir);

  [[nodiscard]] QString computeKey(const PreviewRequest &request) const;
  [[nodiscard]] QString cacheDir() const { return cacheDir_; }
  [[nodiscard]] QString pathForKey(const QString &key) const;
  [[nodiscard]] bool hasArtifact(const QString &key) const;
  [[nodiscard]] qint64 availableBytes() const;

  void removeArtifact(const QString &key) const;
  void cleanupStale(const QString &keepKey) const;
  bool ensureCacheDirExists() const;

private:
  QString cacheDir_;
};

} // namespace core
