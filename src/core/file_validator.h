#pragma once

#include <cstdint>
#include <filesystem>

enum class FileCategory : std::int8_t { Audio, Video, Image };

namespace FileValidator {

bool isValid(const std::filesystem::path &path, FileCategory category);

} // namespace FileValidator
