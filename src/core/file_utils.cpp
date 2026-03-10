#include "core/file_utils.h"

#include <QDir>
#include <QFileInfo>

namespace core::file {

QString generateOutputPath(const QString &audioPath, const QString &outputDir) {
  QFileInfo audioInfo(audioPath);
  QString baseName = audioInfo.completeBaseName();
  QString outputName = baseName + QStringLiteral("_type_beat");

  QDir dir(outputDir);

  if (!dir.exists()) {
    if (!dir.mkpath(QStringLiteral("."))) {
      return QString();
    }
  }

  QString outputPath = dir.filePath(outputName + QStringLiteral(".mp4"));

  int counter = 1;
  const int maxAttempts = 10000;
  while (QFileInfo::exists(outputPath) && counter < maxAttempts) {
    outputPath =
        dir.filePath(outputName + QStringLiteral("_%1.mp4").arg(counter));
    ++counter;
  }

  if (counter >= maxAttempts) {
    return QString();
  }

  return outputPath;
}

} // namespace core::file
