#include "core/file_utils.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <gtest/gtest.h>

using core::file::generateOutputPath;

class FileUtilsTest : public ::testing::Test {
protected:
  QTemporaryDir tempDir;
};

TEST_F(FileUtilsTest, GeneratesCorrectBaseName) {
  auto result = generateOutputPath("/fake/path/my_song.mp3", tempDir.path());
  ASSERT_TRUE(result.ok());
  EXPECT_TRUE(result.path.endsWith("my_song_type_beat.mp4"));
}

TEST_F(FileUtilsTest, CreatesOutputDirectory) {
  QString subDir = tempDir.path() + "/new_subdir";
  auto result = generateOutputPath("/fake/path/song.mp3", subDir);
  ASSERT_TRUE(result.ok());
  EXPECT_TRUE(QDir(subDir).exists());
}

TEST_F(FileUtilsTest, AvoidsCollisions) {
  // Create the first file so it exists
  auto result1 = generateOutputPath("/fake/path/song.mp3", tempDir.path());
  ASSERT_TRUE(result1.ok());
  QFile file1(result1.path);
  ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
  file1.close();

  // Second call should produce a _1 suffix
  auto result2 = generateOutputPath("/fake/path/song.mp3", tempDir.path());
  ASSERT_TRUE(result2.ok());
  EXPECT_TRUE(result2.path.contains("_1.mp4"));
  EXPECT_NE(result1.path, result2.path);
}

TEST_F(FileUtilsTest, ReturnsErrorOnFailedDirCreation) {
  // Try to create dir in a path that doesn't exist and can't be created
  auto result =
      generateOutputPath("/fake/path/song.mp3", "/nonexistent/deep/path/dir");
  EXPECT_FALSE(result.ok());
  EXPECT_FALSE(result.error.isEmpty());
}

TEST_F(FileUtilsTest, HandlesComplexFileName) {
  auto result =
      generateOutputPath("/path/to/my.complex.name.wav", tempDir.path());
  ASSERT_TRUE(result.ok());
  EXPECT_TRUE(result.path.endsWith("my.complex.name_type_beat.mp4"));
}
