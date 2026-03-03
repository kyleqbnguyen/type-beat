#include "core/file_validator.h"
#include "core/core.h"

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
  EXPECT_TRUE(FileValidator::isValid(
      {.path = tmpDir / "test.mp3", .category = core::FileCategory::Audio}));

  EXPECT_TRUE(FileValidator::isValid(
      {.path = tmpDir / "test.wav", .category = core::FileCategory::Audio}));
}

TEST_F(FileValidatorTest, ValidImage) {
  EXPECT_TRUE(FileValidator::isValid(
      {.path = tmpDir / "test.png", .category = core::FileCategory::Image}));
  EXPECT_TRUE(FileValidator::isValid(
      {.path = tmpDir / "test.jpg", .category = core::FileCategory::Image}));
  EXPECT_TRUE(FileValidator::isValid(
      {.path = tmpDir / "test.jpeg", .category = core::FileCategory::Image}));
}

TEST_F(FileValidatorTest, WrongCategory) {
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.mp3", .category = core::FileCategory::Image}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.png", .category = core::FileCategory::Audio}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.mp4", .category = core::FileCategory::Image}));
}

TEST_F(FileValidatorTest, InvalidExtension) {
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.txt", .category = core::FileCategory::Audio}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.txt", .category = core::FileCategory::Video}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.txt", .category = core::FileCategory::Image}));
}

TEST_F(FileValidatorTest, NonExistentFile) {
  EXPECT_FALSE(FileValidator::isValid({.path = tmpDir / "nonexistent.mp3",
                                       .category = core::FileCategory::Audio}));
  EXPECT_FALSE(FileValidator::isValid({.path = tmpDir / "nonexistent.mp3",
                                       .category = core::FileCategory::Video}));
  EXPECT_FALSE(FileValidator::isValid({.path = tmpDir / "nonexistent.mp3",
                                       .category = core::FileCategory::Image}));
}

TEST_F(FileValidatorTest, CaseSensitivity) {
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.MP3", .category = core::FileCategory::Audio}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.MP4", .category = core::FileCategory::Video}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "test.JPG", .category = core::FileCategory::Image}));
}

TEST_F(FileValidatorTest, NoExtension) {
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "noext", .category = core::FileCategory::Audio}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "noext", .category = core::FileCategory::Video}));
  EXPECT_FALSE(FileValidator::isValid(
      {.path = tmpDir / "noext", .category = core::FileCategory::Image}));
}
