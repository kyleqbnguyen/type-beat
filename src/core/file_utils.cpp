#include "core/file_utils.h"

#include <QDir>
#include <QFileInfo>

namespace core::file {

QString generateOutputPath(const QString &audioPath, const QString &outputDir) {
  QFileInfo audioInfo(audioPath);
  QString baseName = audioInfo.completeBaseName();
  QString outputName = baseName + QStringLiteral("_type_beat");

  QDir dir(outputDir);
  QString outputPath = dir.filePath(outputName + QStringLiteral(".mp4"));

  int counter = 1;
  while (QFileInfo::exists(outputPath)) {
    outputPath =
        dir.filePath(outputName + QStringLiteral("_%1.mp4").arg(counter));
    ++counter;
  }

  return outputPath;
}

} // namespace core::file
