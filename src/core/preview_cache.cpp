#include "core/preview_cache.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QStorageInfo>

Q_LOGGING_CATEGORY(lcPreviewCache, "typebeat.preview_cache")

namespace core {

namespace {

QByteArray fileIdentity(const QString &path) {
  QFileInfo info(path);
  QByteArray buf;
  buf.append(path.toUtf8());
  buf.append('|');
  buf.append(QByteArray::number(info.size()));
  buf.append('|');
  buf.append(QByteArray::number(info.lastModified().toMSecsSinceEpoch()));
  return buf;
}

QByteArray profileIdentity(const RenderProfile &p) {
  QByteArray buf;
  buf.append(QByteArray::number(p.width));
  buf.append('x');
  buf.append(QByteArray::number(p.height));
  buf.append('|');
  buf.append(p.videoCodec.toUtf8());
  buf.append('|');
  buf.append(p.audioCodec.toUtf8());
  buf.append('|');
  buf.append(p.audioBitrate.toUtf8());
  buf.append('|');
  buf.append(p.encoderPreset.toUtf8());
  buf.append('|');
  buf.append(QByteArray::number(p.imageCrf));
  buf.append('|');
  buf.append(QByteArray::number(p.videoCrf));
  return buf;
}

} // namespace

PreviewCache::PreviewCache() {
  QString base =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (base.isEmpty()) {
    base = QDir::tempPath();
  }
  cacheDir_ = QDir(base).filePath(QStringLiteral("preview"));
}

PreviewCache::PreviewCache(const QString &cacheDir) : cacheDir_(cacheDir) {}

bool PreviewCache::ensureCacheDirExists() const {
  QDir dir;
  if (!dir.mkpath(cacheDir_)) {
    qCWarning(lcPreviewCache)
        << "Failed to create preview cache dir:" << cacheDir_;
    return false;
  }
  return true;
}

QString PreviewCache::computeKey(const PreviewRequest &request) const {
  if (request.visualPath.isEmpty() || request.audioPath.isEmpty()) {
    return QString();
  }
  if (!QFileInfo::exists(request.visualPath) ||
      !QFileInfo::exists(request.audioPath)) {
    return QString();
  }

  QCryptographicHash hash(QCryptographicHash::Sha256);
  hash.addData(QByteArray::number(kRenderPipelineVersion));
  hash.addData("|");
  hash.addData(fileIdentity(request.visualPath));
  hash.addData("|");
  hash.addData(fileIdentity(request.audioPath));
  hash.addData("|");
  hash.addData(profileIdentity(request.profile));
  return QString::fromLatin1(hash.result().toHex());
}

QString PreviewCache::pathForKey(const QString &key) const {
  if (key.isEmpty()) {
    return QString();
  }
  return QDir(cacheDir_).filePath(key + QStringLiteral(".mp4"));
}

bool PreviewCache::hasArtifact(const QString &key) const {
  const QString path = pathForKey(key);
  if (path.isEmpty()) {
    return false;
  }
  QFileInfo info(path);
  return info.exists() && info.isFile() && info.size() > 0;
}

qint64 PreviewCache::availableBytes() const {
  QStorageInfo storage(QFileInfo(cacheDir_).absolutePath());
  if (storage.isValid() && storage.isReady()) {
    return storage.bytesAvailable();
  }
  return -1;
}

void PreviewCache::removeArtifact(const QString &key) const {
  const QString path = pathForKey(key);
  if (path.isEmpty()) {
    return;
  }
  QFile::remove(path);
}

void PreviewCache::cleanupStale(const QString &keepKey) const {
  QDir dir(cacheDir_);
  if (!dir.exists()) {
    return;
  }
  const QString keepFile =
      keepKey.isEmpty() ? QString() : (keepKey + QStringLiteral(".mp4"));
  const QFileInfoList entries =
      dir.entryInfoList({QStringLiteral("*.mp4")}, QDir::Files);
  for (const QFileInfo &info : entries) {
    if (!keepFile.isEmpty() && info.fileName() == keepFile) {
      continue;
    }
    QFile::remove(info.absoluteFilePath());
  }
}

} // namespace core
