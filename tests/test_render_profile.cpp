#include "core/render_profile.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class RenderProfileTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    if (QCoreApplication::instance() == nullptr) {
      static int argc = 1;
      static char arg0[] = "test";
      static char *argv[] = {arg0};
      new QCoreApplication(argc, argv);
    }
  }
};

TEST_F(RenderProfileTest, PresetsHaveExpectedDimensions) {
  auto draft = core::renderProfileForPreset(core::QualityPreset::Draft);
  EXPECT_EQ(draft.width, 1280);
  EXPECT_EQ(draft.height, 720);

  auto standard = core::renderProfileForPreset(core::QualityPreset::Standard);
  EXPECT_EQ(standard.width, 1920);
  EXPECT_EQ(standard.height, 1080);

  auto high = core::renderProfileForPreset(core::QualityPreset::High);
  EXPECT_EQ(high.width, 1920);
  EXPECT_EQ(high.height, 1080);

  auto master = core::renderProfileForPreset(core::QualityPreset::Master);
  EXPECT_EQ(master.width, 1920);
  EXPECT_EQ(master.height, 1080);
}

TEST_F(RenderProfileTest, PresetsAreOrdered) {
  auto draft = core::renderProfileForPreset(core::QualityPreset::Draft);
  auto standard = core::renderProfileForPreset(core::QualityPreset::Standard);
  auto high = core::renderProfileForPreset(core::QualityPreset::High);
  auto master = core::renderProfileForPreset(core::QualityPreset::Master);

  EXPECT_GT(draft.imageCrf, standard.imageCrf);
  EXPECT_GT(standard.imageCrf, high.imageCrf);
  EXPECT_GT(high.imageCrf, master.imageCrf);

  EXPECT_GT(draft.videoCrf, standard.videoCrf);
  EXPECT_GT(standard.videoCrf, high.videoCrf);
  EXPECT_GT(high.videoCrf, master.videoCrf);
}

TEST_F(RenderProfileTest, IsProfileEquivalentToPreset) {
  auto profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  EXPECT_TRUE(core::isProfileEquivalentToPreset(profile,
                                                core::QualityPreset::Standard));
  EXPECT_FALSE(
      core::isProfileEquivalentToPreset(profile, core::QualityPreset::Draft));
}

TEST_F(RenderProfileTest, DetectPresetFindsKnownPreset) {
  auto profile = core::renderProfileForPreset(core::QualityPreset::High);
  EXPECT_EQ(core::detectPreset(profile), core::QualityPreset::High);

  profile.videoCrf -= 1;
  EXPECT_EQ(core::detectPreset(profile), core::QualityPreset::Standard);
}

TEST_F(RenderProfileTest, ProfileToRenderConfigMapsAllFields) {
  auto profile = core::renderProfileForPreset(core::QualityPreset::High);
  auto config = profile.toRenderConfig();
  EXPECT_EQ(config.width, profile.width);
  EXPECT_EQ(config.height, profile.height);
  EXPECT_EQ(config.videoCodec, profile.videoCodec);
  EXPECT_EQ(config.audioCodec, profile.audioCodec);
  EXPECT_EQ(config.audioBitrate, profile.audioBitrate);
  EXPECT_EQ(config.preset, profile.encoderPreset);
  EXPECT_EQ(config.imageCrf, profile.imageCrf);
  EXPECT_EQ(config.videoCrf, profile.videoCrf);
}

TEST_F(RenderProfileTest, InvalidProfileIsRejected) {
  core::RenderProfile profile =
      core::renderProfileForPreset(core::QualityPreset::Standard);
  EXPECT_TRUE(profile.isValid());

  profile.width = -1;
  EXPECT_FALSE(profile.isValid());

  profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  profile.width = 1921;
  EXPECT_FALSE(profile.isValid());

  profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  profile.imageCrf = 100;
  EXPECT_FALSE(profile.isValid());

  profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  profile.encoderPreset = QStringLiteral("bogus");
  EXPECT_FALSE(profile.isValid());

  profile = core::renderProfileForPreset(core::QualityPreset::Standard);
  profile.audioBitrate = QStringLiteral("7k");
  EXPECT_FALSE(profile.isValid());
}

TEST_F(RenderProfileTest, PresetStringRoundTrip) {
  for (auto preset : {core::QualityPreset::Draft, core::QualityPreset::Standard,
                      core::QualityPreset::High, core::QualityPreset::Master}) {
    QString s = core::qualityPresetToString(preset);
    EXPECT_EQ(core::qualityPresetFromString(s), preset);
  }
}
