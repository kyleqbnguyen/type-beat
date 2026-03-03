#include "file_utils.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace {

constexpr std::size_t categoryCount = 3;

const std::array<std::unordered_set<std::string>, categoryCount> validExts = {{
    {".mp3", ".wav"},          // Audio
    {".mp4", ".mov"},          // Video
    {".png", ".jpg", ".jpeg"}, // Image
}};

} // anonymous namespace

bool core::file::isValid(const core::file::Info &fileInfo) {
  if (!std::filesystem::exists(fileInfo.path)) {
    return false;
  }

  auto index = static_cast<std::size_t>(fileInfo.category);
  if (index >= categoryCount) {
    return false;
  }

  std::string ext{fileInfo.path.extension()};

  return validExts.at(index).contains(ext);
}
