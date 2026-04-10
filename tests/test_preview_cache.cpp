#include "core/preview_cache.h"
#include "core/render_profile.h"

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include <gtest/gtest.h>

class PreviewCacheTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char arg0[] = "test";
      static char *argv[] = {arg0};
      new QCoreApplication(argc, argv);
    }
  }

  QString writeTempFile(const QString &name, const QByteArray &content) {
    QString path = tempDir_.filePath(name);
    QFile f(path);
    EXPECT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(content);
    f.close();
    return path;
  }

  QTemporaryDir tempDir_;
};

TEST_F(PreviewCacheTest, EmptyInputsGiveEmptyKey) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  core::PreviewRequest request;
  EXPECT_TRUE(cache.computeKey(request).isEmpty());
}

TEST_F(PreviewCacheTest, KeyIsStableForSameInputsAndProfile) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  QString v = writeTempFile("visual.png", QByteArray(128, 'a'));
  QString a = writeTempFile("audio.wav", QByteArray(256, 'b'));

  core::PreviewRequest request{
      v, a, core::renderProfileForPreset(core::QualityPreset::Standard)};

  QString k1 = cache.computeKey(request);
  QString k2 = cache.computeKey(request);
  EXPECT_FALSE(k1.isEmpty());
  EXPECT_EQ(k1, k2);
}

TEST_F(PreviewCacheTest, KeyChangesWithProfile) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  QString v = writeTempFile("visual.png", QByteArray(128, 'a'));
  QString a = writeTempFile("audio.wav", QByteArray(256, 'b'));

  core::PreviewRequest r1{
      v, a, core::renderProfileForPreset(core::QualityPreset::Standard)};
  core::PreviewRequest r2{
      v, a, core::renderProfileForPreset(core::QualityPreset::High)};

  EXPECT_NE(cache.computeKey(r1), cache.computeKey(r2));
}

TEST_F(PreviewCacheTest, KeyChangesWhenFileContentChanges) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  QString v = writeTempFile("visual.png", QByteArray(128, 'a'));
  QString a = writeTempFile("audio.wav", QByteArray(256, 'b'));

  core::PreviewRequest request{
      v, a, core::renderProfileForPreset(core::QualityPreset::Standard)};

  QString before = cache.computeKey(request);

  writeTempFile("audio.wav", QByteArray(512, 'c'));
  QString after = cache.computeKey(request);
  EXPECT_NE(before, after);
}

TEST_F(PreviewCacheTest, KeyChangesWithCustomCrfEdit) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  QString v = writeTempFile("visual.png", QByteArray(128, 'a'));
  QString a = writeTempFile("audio.wav", QByteArray(256, 'b'));

  auto profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  core::PreviewRequest r1{v, a, profile};

  profile.videoCrf += 1;
  core::PreviewRequest r2{v, a, profile};

  EXPECT_NE(cache.computeKey(r1), cache.computeKey(r2));
}

TEST_F(PreviewCacheTest, HasArtifactReflectsFilePresence) {
  core::PreviewCache cache(tempDir_.filePath("cache"));
  QString v = writeTempFile("visual.png", QByteArray(128, 'a'));
  QString a = writeTempFile("audio.wav", QByteArray(256, 'b'));

  core::PreviewRequest request{
      v, a, core::renderProfileForPreset(core::QualityPreset::Standard)};

  QString key = cache.computeKey(request);
  ASSERT_FALSE(key.isEmpty());

  EXPECT_FALSE(cache.hasArtifact(key));
  EXPECT_TRUE(cache.ensureCacheDirExists());

  QFile f(cache.pathForKey(key));
  ASSERT_TRUE(f.open(QIODevice::WriteOnly));
  f.write(QByteArray("fake preview"));
  f.close();

  EXPECT_TRUE(cache.hasArtifact(key));
  cache.removeArtifact(key);
  EXPECT_FALSE(cache.hasArtifact(key));
}
