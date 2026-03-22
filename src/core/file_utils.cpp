#include "core/file_utils.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcFileUtils, "typebeat.fileutils")

namespace core::file {

OutputPathResult generateOutputPath(const QString &audioPath,
                                    const QString &outputDir) {
  QFileInfo audioInfo(audioPath);
  QString baseName = audioInfo.completeBaseName();
  QString outputName = baseName + QStringLiteral("_type_beat");

  QDir dir(outputDir);

  if (!dir.exists()) {
    if (!dir.mkpath(QStringLiteral("."))) {
      QString err = QStringLiteral("Failed to create output directory: %1")
                        .arg(outputDir);
      qCWarning(lcFileUtils) << err;
      return {QString(), err};
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
    QString err = QStringLiteral("Too many files with name '%1' in %2")
                      .arg(outputName, outputDir);
    qCWarning(lcFileUtils) << err;
    return {QString(), err};
  }

  return {outputPath, QString()};
}

} // namespace core::file
