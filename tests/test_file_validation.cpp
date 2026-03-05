#include "core/file_utils.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

class FileValidationTest : public ::testing::Test {
protected:
  std::filesystem::path tmpDir;

  void SetUp() override {
    tmpDir = std::filesystem::temp_directory_path() / "file_validation_test";
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

TEST_F(FileValidationTest, ValidAudio) {
  EXPECT_TRUE(core::file::isValid({.path = tmpDir / "test.mp3",
                                   .category = core::file::Category::Audio,
                                   .duration{}}));

  EXPECT_TRUE(core::file::isValid({.path = tmpDir / "test.wav",
                                   .category = core::file::Category::Audio,
                                   .duration{}}));
}

TEST_F(FileValidationTest, ValidImage) {
  EXPECT_TRUE(core::file::isValid({.path = tmpDir / "test.png",
                                   .category = core::file::Category::Image,
                                   .duration{}}));
  EXPECT_TRUE(core::file::isValid({.path = tmpDir / "test.jpg",
                                   .category = core::file::Category::Image,
                                   .duration{}}));
  EXPECT_TRUE(core::file::isValid({.path = tmpDir / "test.jpeg",
                                   .category = core::file::Category::Image,
                                   .duration{}}));
}

TEST_F(FileValidationTest, WrongCategory) {
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.mp3",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.png",
                                    .category = core::file::Category::Audio,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.mp4",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
}

TEST_F(FileValidationTest, InvalidExtension) {
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.txt",
                                    .category = core::file::Category::Audio,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.txt",
                                    .category = core::file::Category::Video,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.txt",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
}

TEST_F(FileValidationTest, NonExistentFile) {
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "nonexistent.mp3",
                                    .category = core::file::Category::Audio,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "nonexistent.mp3",
                                    .category = core::file::Category::Video,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "nonexistent.mp3",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
}

TEST_F(FileValidationTest, CaseSensitivity) {
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.MP3",
                                    .category = core::file::Category::Audio,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.MP4",
                                    .category = core::file::Category::Video,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "test.JPG",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
}

TEST_F(FileValidationTest, NoExtension) {
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "noext",
                                    .category = core::file::Category::Audio,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "noext",
                                    .category = core::file::Category::Video,
                                    .duration{}}));
  EXPECT_FALSE(core::file::isValid({.path = tmpDir / "noext",
                                    .category = core::file::Category::Image,
                                    .duration{}}));
}
