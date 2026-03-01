#pragma once

#include <cstdint>
#include <filesystem>

enum class FileCategory : std::int8_t { Audio, Video, Image };

struct Config {
  std::filesystem::path audioPath;
  std::filesystem::path visualPath;
  std::filesystem::path outputPath;
  FileCategory visualType;
};
