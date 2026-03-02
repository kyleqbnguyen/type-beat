#pragma once

#include <cstdint>
#include <filesystem>

enum class FileCategory : std::int8_t { Audio, Video, Image };

namespace RenderConfig {

struct Config {
  std::filesystem::path audioPath;
  std::filesystem::path visualPath;
  std::filesystem::path outputPath;
  FileCategory visualType;
};

} // namespace RenderConfig 
