// feat(TODO): per-device encoder selection

#include <cstdint>
#include <string>
#include <unordered_map>

namespace EncoderDetection {

enum class Encoder : int8_t {
  libx264,
  h264_nvenc,
  h264_qsv,
  h264_videotoolbox,
  h264_amf,
};

const std::unordered_map<Encoder, std::string> encoderSettings;

} // namespace EncoderDetection
