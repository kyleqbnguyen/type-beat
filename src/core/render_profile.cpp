#include "core/render_profile.h"

namespace core {

ffmpeg::RenderConfig RenderProfile::toRenderConfig() const {
  ffmpeg::RenderConfig config;
  config.width = width;
  config.height = height;
  config.videoCodec = videoCodec;
  config.audioCodec = audioCodec;
  config.audioBitrate = audioBitrate;
  config.preset = encoderPreset;
  config.imageCrf = imageCrf;
  config.videoCrf = videoCrf;
  return config;
}

QString RenderProfile::validationError() const {
  if (width < kMinDimension || width > kMaxDimension) {
    return QStringLiteral("Width must be between %1 and %2")
        .arg(kMinDimension)
        .arg(kMaxDimension);
  }
  if (height < kMinDimension || height > kMaxDimension) {
    return QStringLiteral("Height must be between %1 and %2")
        .arg(kMinDimension)
        .arg(kMaxDimension);
  }
  if (width % 2 != 0 || height % 2 != 0) {
    return QStringLiteral(
        "Width and height must be even (yuv420p requires it)");
  }
  if (imageCrf < kMinCrf || imageCrf > kMaxCrf) {
    return QStringLiteral("Image CRF must be between %1 and %2")
        .arg(kMinCrf)
        .arg(kMaxCrf);
  }
  if (videoCrf < kMinCrf || videoCrf > kMaxCrf) {
    return QStringLiteral("Video CRF must be between %1 and %2")
        .arg(kMinCrf)
        .arg(kMaxCrf);
  }
  if (!supportedEncoderPresets().contains(encoderPreset)) {
    return QStringLiteral("Unknown encoder preset: %1").arg(encoderPreset);
  }
  if (!supportedAudioBitrates().contains(audioBitrate)) {
    return QStringLiteral("Unsupported audio bitrate: %1").arg(audioBitrate);
  }
  return QString();
}

bool RenderProfile::operator==(const RenderProfile &other) const {
  return width == other.width && height == other.height &&
         videoCodec == other.videoCodec && audioCodec == other.audioCodec &&
         audioBitrate == other.audioBitrate &&
         encoderPreset == other.encoderPreset && imageCrf == other.imageCrf &&
         videoCrf == other.videoCrf;
}

RenderProfile renderProfileForPreset(QualityPreset preset) {
  RenderProfile profile;
  switch (preset) {
  case QualityPreset::Draft:
    profile.width = 1280;
    profile.height = 720;
    profile.encoderPreset = QStringLiteral("ultrafast");
    profile.imageCrf = 28;
    profile.videoCrf = 30;
    profile.audioBitrate = QStringLiteral("128k");
    break;
  case QualityPreset::Standard:
    profile.width = 1920;
    profile.height = 1080;
    profile.encoderPreset = QStringLiteral("medium");
    profile.imageCrf = 20;
    profile.videoCrf = 23;
    profile.audioBitrate = QStringLiteral("192k");
    break;
  case QualityPreset::High:
    profile.width = 1920;
    profile.height = 1080;
    profile.encoderPreset = QStringLiteral("slow");
    profile.imageCrf = 18;
    profile.videoCrf = 20;
    profile.audioBitrate = QStringLiteral("256k");
    break;
  case QualityPreset::Master:
    profile.width = 1920;
    profile.height = 1080;
    profile.encoderPreset = QStringLiteral("veryslow");
    profile.imageCrf = 16;
    profile.videoCrf = 17;
    profile.audioBitrate = QStringLiteral("320k");
    break;
  }
  return profile;
}

bool isProfileEquivalentToPreset(const RenderProfile &profile,
                                 QualityPreset preset) {
  return profile == renderProfileForPreset(preset);
}

QualityPreset detectPreset(const RenderProfile &profile) {
  for (auto preset : {QualityPreset::Draft, QualityPreset::Standard,
                      QualityPreset::High, QualityPreset::Master}) {
    if (isProfileEquivalentToPreset(profile, preset)) {
      return preset;
    }
  }
  return QualityPreset::Standard;
}

QString qualityPresetToString(QualityPreset preset) {
  switch (preset) {
  case QualityPreset::Draft:
    return QStringLiteral("draft");
  case QualityPreset::Standard:
    return QStringLiteral("standard");
  case QualityPreset::High:
    return QStringLiteral("high");
  case QualityPreset::Master:
    return QStringLiteral("master");
  }
  return QStringLiteral("standard");
}

QualityPreset qualityPresetFromString(const QString &str) {
  const QString lower = str.toLower();
  if (lower == QStringLiteral("draft")) {
    return QualityPreset::Draft;
  }
  if (lower == QStringLiteral("high")) {
    return QualityPreset::High;
  }
  if (lower == QStringLiteral("master")) {
    return QualityPreset::Master;
  }
  return QualityPreset::Standard;
}

QString qualityPresetDisplayName(QualityPreset preset) {
  switch (preset) {
  case QualityPreset::Draft:
    return QStringLiteral("Draft");
  case QualityPreset::Standard:
    return QStringLiteral("Standard");
  case QualityPreset::High:
    return QStringLiteral("High");
  case QualityPreset::Master:
    return QStringLiteral("Master");
  }
  return QStringLiteral("Standard");
}

QStringList supportedEncoderPresets() {
  return {
      QStringLiteral("ultrafast"), QStringLiteral("superfast"),
      QStringLiteral("veryfast"),  QStringLiteral("faster"),
      QStringLiteral("fast"),      QStringLiteral("medium"),
      QStringLiteral("slow"),      QStringLiteral("slower"),
      QStringLiteral("veryslow"),
  };
}

QStringList supportedAudioBitrates() {
  return {
      QStringLiteral("96k"),  QStringLiteral("128k"), QStringLiteral("160k"),
      QStringLiteral("192k"), QStringLiteral("256k"), QStringLiteral("320k"),
  };
}

} // namespace core
