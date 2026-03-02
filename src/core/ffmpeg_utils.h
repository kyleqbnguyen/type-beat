#pragma once

#include "render_config.h"
#include "encoder_detection.h"

#include <string>
#include <vector>

namespace FFmpegUtils {

double getAudioDuration(RenderConfig::Config path);

std::vector<std::string> buildCommand(RenderConfig::Config config,
                                      EncoderDetection::Encoder encoder);

// TODO: function trackProgress

} // namespace FFmpegUtils
