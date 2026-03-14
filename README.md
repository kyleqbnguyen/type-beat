# TypeBeat

Combine visuals with audio to create videos.

## Demo
https://github.com/user-attachments/assets/db6dc716-caf1-459c-be69-d0699c532574

## Download

Download the latest release for your platform from the [Releases page](https://github.com/kyleqbnguyen/type-beat/releases).

| Platform | Download |
|----------|----------|
| Windows | `TypeBeat-x.x.x-win64.exe` |
| macOS | `TypeBeat-vx.x.x-macOS.dmg` |
| Linux | `TypeBeat-vx.x.x-x86_64.AppImage` |

## Installation

### Windows

1. Download the `.exe` installer
2. Double-click to run
3. If you see a SmartScreen warning ("Windows protected your PC"):
   - Click **"More info"**
   - Click **"Run anyway"**
4. Follow the installer wizard
5. Launch from the Start Menu

### macOS

1. Download the `.dmg` file
2. **Before opening**, run this command in Terminal to remove the quarantine:
   ```bash
   xattr -cr ~/Downloads/TypeBeat-*.dmg
   ```
3. Double-click the `.dmg` to mount it
4. Drag `TypeBeat.app` to your Applications folder
5. Double-click to launch

> **Note:** This app is not notarized with Apple. The `xattr` command is required because macOS blocks unsigned apps downloaded from the internet.

### Linux

1. Download the `.AppImage` file
2. Make it executable:
   ```bash
   chmod +x TypeBeat-*.AppImage
   ```
3. Double-click or run from terminal:
   ```bash
   ./TypeBeat-*.AppImage
   ```

## Requirements

FFmpeg is bundled with the app - no separate installation needed.

## License

GPL-3.0 - see [LICENSE](LICENSE)
