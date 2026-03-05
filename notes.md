# Design considerations
- how output is saved?
  - default system Downloads 
  - user configurable and updates on each export
- Bundle ffmpeg w/ app
- Errors: log in backend and simple message in UI
- Overwrting: 
  - index default name if alr exist
  - prompt overwrite

# Image
ffmpeg -loop 1 -framerate 1 -i image.png -i audio.mp3 \
  -c:v libx264 -tune stillimage -preset ultrafast -crf 18 \
  -c:a aac -b:a 192k \
  -pix_fmt yuv420p -r 1 -shortest \
  -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
  output.mp4

# Video
## Software (faster render time, larger file size)
  ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3 \
    -map 0:v -map 1:a \
    -c:v libx264 -preset ultrafast -crf 23 \
    -c:a aac -b:a 192k \
    -pix_fmt yuv420p -t <audio_duration> \
    -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
    output.mp4

# Hardware Encoding (slower render time, smaller file size)
## NVIDIA GPU
  ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3 \
    -map 0:v -map 1:a \
    -c:v h264_nvenc -preset p1 -cq 23 \
    -c:a aac -b:a 192k \
    -pix_fmt yuv420p -t <audio_duration> \
    -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
    output.mp4

## Intel QSV
  ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3 \
    -map 0:v -map 1:a \
    -c:v h264_qsv -preset veryfast -global_quality 23 \
    -c:a aac -b:a 192k \
    -pix_fmt yuv420p -t <audio_duration> \
    -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
    output.mp4

## macOS VideoToolbox
  ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3 \
    -map 0:v -map 1:a \
    -c:v h264_videotoolbox -q:v 65 \
    -c:a aac -b:a 192k \
    -pix_fmt yuv420p -t <audio_duration> \
    -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
    output.mp4

## AMD AMF
  ffmpeg -stream_loop -1 -i video.mp4 -i audio.mp3 \
    -map 0:v -map 1:a \
    -c:v h264_amf -quality speed -rc cqp -qp_i 23 -qp_p 23 \
    -c:a aac -b:a 192k \
    -pix_fmt yuv420p -t <audio_duration> \
    -vf "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2" \
    output.mp4
