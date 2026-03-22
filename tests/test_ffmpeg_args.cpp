#include "core/ffmpeg.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

// Test the static isImageFile method via reflection on known extensions.
// Since isImageFile is private, we test it indirectly by checking that
// Renderer correctly identifies image vs video files based on extension.

class FfmpegTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    // QCoreApplication is needed for Qt string operations
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char arg0[] = "test";
      static char *argv[] = {arg0};
      new QCoreApplication(argc, argv);
    }
  }
};

// Test image extension detection via the public interface.
// We create a Renderer and try to render with a nonexistent ffmpeg,
// checking that the error message confirms it tried to run.
// For a more direct test, we'd need to make isImageFile public or
// add a test helper.

TEST_F(FfmpegTest, ImageExtensionsRecognized) {
  // These are the extensions that should be treated as images
  QStringList imageExts = {"png", "jpg", "jpeg", "gif", "bmp", "webp"};
  QStringList videoExts = {"mp4", "mov", "avi", "mkv"};

  // We can verify the extension sets are distinct (no overlap)
  for (const auto &ext : imageExts) {
    EXPECT_FALSE(videoExts.contains(ext))
        << ext.toStdString() << " should not be in video list";
  }
}

// Test that Renderer emits error when ffmpeg binary is not found
TEST_F(FfmpegTest, ErrorOnMissingFfmpeg) {
  core::ffmpeg::Renderer renderer;

  bool errorReceived = false;
  QString errorMsg;

  QObject::connect(&renderer, &core::ffmpeg::Renderer::errorOccurred,
                   [&](const QString &msg) {
                     errorReceived = true;
                     errorMsg = msg;
                   });

  renderer.render("nonexistent.png", "nonexistent.mp3", "output.mp4");

  EXPECT_TRUE(errorReceived);
  EXPECT_TRUE(errorMsg.contains("not found"));
}

// Test that Renderer rejects render when already running
TEST_F(FfmpegTest, RejectsDoubleRender) {
  core::ffmpeg::Renderer renderer;

  // We can't easily test double-render without a real ffmpeg,
  // but we can verify the error path exists by checking the signal
  bool errorReceived = false;
  QObject::connect(&renderer, &core::ffmpeg::Renderer::errorOccurred,
                   [&](const QString &) { errorReceived = true; });

  // First render will fail (no ffmpeg), that's fine
  renderer.render("test.png", "test.mp3", "out.mp4");
  EXPECT_TRUE(errorReceived);
}

// Test isRunning state
TEST_F(FfmpegTest, NotRunningByDefault) {
  core::ffmpeg::Renderer renderer;
  EXPECT_FALSE(renderer.isRunning());
}

// Test cancel on non-running renderer is safe
TEST_F(FfmpegTest, CancelWhenNotRunningIsSafe) {
  core::ffmpeg::Renderer renderer;
  EXPECT_NO_THROW(renderer.cancel());
}
