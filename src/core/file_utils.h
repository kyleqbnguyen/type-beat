#pragma once

#include <cstdint>
#include <filesystem>

namespace core::file {

enum class Category : std::int8_t { Audio, Video, Image };

struct FileInfo {
  std::filesystem::path path;
  Category category;
};


bool isValid(const core::file::FileInfo &fileInfo);

} // namespace core::file
