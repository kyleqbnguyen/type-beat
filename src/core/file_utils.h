#pragma once

#include <cstdint>
#include <filesystem>

namespace core::file {

enum class Category : std::int8_t { Audio, Video, Image };

struct Info {
  std::filesystem::path path;
  Category category;
};

bool isValid(const core::file::Info &fileInfo);

} // namespace core::file

