#pragma once

#include <QString>
#include <cstdint>
#include <filesystem>
#include <optional>

namespace core::file {

enum class Category : std::int8_t { Audio, Video, Image };

struct FileInfo {
  std::filesystem::path path;
  Category category;
  std::optional<double> duration;
};

bool isValid(const core::file::FileInfo &fileInfo);

QString generateOutputPath(const QString &audioPath, const QString &outputDir);

} // namespace core::file
