#pragma once

#include "config.h"

#include <filesystem>

namespace FileValidator {

bool isValid(const std::filesystem::path &path, FileCategory category);

} // namespace FileValidator
