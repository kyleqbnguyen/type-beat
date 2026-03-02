#pragma once

#include "render_config.h"

#include <filesystem>

namespace FileValidator {

bool isValid(const std::filesystem::path &path, FileCategory category);

} // namespace FileValidator
