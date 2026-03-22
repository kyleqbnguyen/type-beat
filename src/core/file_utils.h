#pragma once

#include <QString>

namespace core::file {

struct OutputPathResult {
  QString path;
  QString error;
  [[nodiscard]] bool ok() const { return error.isEmpty() && !path.isEmpty(); }
};

OutputPathResult generateOutputPath(const QString &audioPath,
                                    const QString &outputDir);

} // namespace core::file
