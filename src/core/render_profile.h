#pragma once

#include "core/ffmpeg.h"

#include <QString>
#include <QStringList>

namespace core {

enum class QualityPreset {
  Draft,
  Standard,
  High,
  Master,
};

struct RenderProfile {
  int width = 1920;
  int height = 1080;
  QString videoCodec = QStringLiteral("libx264");
  QString audioCodec = QStringLiteral("aac");
  QString audioBitrate = QStringLiteral("192k");
  QString encoderPreset = QStringLiteral("medium");
  int imageCrf = 20;
  int videoCrf = 23;

  [[nodiscard]] ffmpeg::RenderConfig toRenderConfig() const;
  [[nodiscard]] QString validationError() const;
  [[nodiscard]] bool isValid() const { return validationError().isEmpty(); }

  bool operator==(const RenderProfile &other) const;
  bool operator!=(const RenderProfile &other) const {
    return !(*this == other);
  }
};

[[nodiscard]] RenderProfile renderProfileForPreset(QualityPreset preset);
[[nodiscard]] bool isProfileEquivalentToPreset(const RenderProfile &profile,
                                               QualityPreset preset);
[[nodiscard]] QualityPreset detectPreset(const RenderProfile &profile);

[[nodiscard]] QString qualityPresetToString(QualityPreset preset);
[[nodiscard]] QualityPreset qualityPresetFromString(const QString &str);
[[nodiscard]] QString qualityPresetDisplayName(QualityPreset preset);

[[nodiscard]] QStringList supportedEncoderPresets();
[[nodiscard]] QStringList supportedAudioBitrates();

inline constexpr int kMinCrf = 0;
inline constexpr int kMaxCrf = 51;
inline constexpr int kMinDimension = 16;
inline constexpr int kMaxDimension = 7680;

inline constexpr int kRenderPipelineVersion = 1;

} // namespace core
