#ifndef FILE_VALIDATOR_H
#define FILE_VALIDATOR_H

#include <filesystem>

enum class FileCategory { Audio, Video, Image };

namespace FileValidator {

bool isValid(const std::filesystem::path &path, FileCategory category);

} // namespace FileValidator 

#endif // FILE_VALIDATOR_H
