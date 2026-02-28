#include "core/file_validator.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

class FileValidatorTest : public ::testing::Test {
protected:
  std::filesystem::path tmpDir;

  void SetUp() override {
    tmpDir = std::filesystem::temp_directory_path() / "file_validator_test";
    std::filesystem::create_directories(tmpDir);

    const std::vector<std::string> testFiles = {
        "test.mp3",  "test.wav", "test.mp4", "test.mov", "test.png", "test.jpg",
        "test.jpeg", "test.txt", "test.MP3", "test.MP4", "test.JPG", "noext",
    };

    for (const auto &name : testFiles) {
      std::ofstream{tmpDir / name};
    }
  }

  void TearDown() override { std::filesystem::remove_all(tmpDir); }
};

TEST_F(FileValidatorTest, ValidAudio) {
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.mp3", FileCategory::Audio));
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.wav", FileCategory::Audio));
}

TEST_F(FileValidatorTest, ValidVideo) {
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.mp4", FileCategory::Video));
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.mov", FileCategory::Video));
}

TEST_F(FileValidatorTest, ValidImage) {
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.png", FileCategory::Image));
  EXPECT_TRUE(FileValidator::isValid(tmpDir / "test.jpg", FileCategory::Image));
  EXPECT_TRUE(
      FileValidator::isValid(tmpDir / "test.jpeg", FileCategory::Image));
}

TEST_F(FileValidatorTest, WrongCategory) {
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.mp3", FileCategory::Image));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.png", FileCategory::Audio));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.mp4", FileCategory::Image));
}

TEST_F(FileValidatorTest, InvalidExtension) {
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.txt", FileCategory::Audio));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.txt", FileCategory::Video));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.txt", FileCategory::Image));
}

TEST_F(FileValidatorTest, NonExistentFile) {
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "nonexistent.mp3", FileCategory::Audio));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "nonexistent.mp3", FileCategory::Video));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "nonexistent.mp3", FileCategory::Image));
}

TEST_F(FileValidatorTest, CaseSensitivity) {
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.MP3", FileCategory::Audio));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.MP4", FileCategory::Video));
  EXPECT_FALSE(
      FileValidator::isValid(tmpDir / "test.JPG", FileCategory::Image));
}

TEST_F(FileValidatorTest, NoExtension) {
  EXPECT_FALSE(FileValidator::isValid(tmpDir / "noext", FileCategory::Audio));
  EXPECT_FALSE(FileValidator::isValid(tmpDir / "noext", FileCategory::Video));
  EXPECT_FALSE(FileValidator::isValid(tmpDir / "noext", FileCategory::Image));
}
