# FFmpeg Checksums

These files contain SHA256 checksums for the FFmpeg binaries downloaded during CI release builds.

**To update checksums when upgrading FFmpeg:**

1. Download the new FFmpeg binaries manually
2. Compute SHA256: `shasum -a 256 <file>` (macOS/Linux) or `Get-FileHash <file> -Algorithm SHA256` (PowerShell)
3. Replace the hash in the corresponding `.sha256` file (lowercase hex, no filename suffix)

Files:
- `ffmpeg-windows.sha256` — hash for `ffmpeg-release-essentials.zip` from gyan.dev
- `ffmpeg-macos.sha256` — hash for `ffmpeg.7z` from evermeet.cx
- `ffprobe-macos.sha256` — hash for `ffprobe.7z` from evermeet.cx
- `ffmpeg-linux.sha256` — hash for `ffmpeg-release-amd64-static.tar.xz` from johnvansickle.com
