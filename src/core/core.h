#pragma once

#include <cstdint>
#include <filesystem>

namespace core {

enum class FileCategory : std::int8_t { Audio, Video, Image };

struct FileInfo {
  std::filesystem::path path;
  FileCategory category;
};

} // namespace core
