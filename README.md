# Windows Power Manager (WPM) v1.3

Console-based power plan manager for Windows with brightness control.

**Version:** 1.3 | [Download Releases](https://github.com/Lonewolf239/WPM/releases)

## Features

- Switch between three power plans:
  - High Performance (`8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c`)
  - Balanced (`381b4222-f694-41f0-9685-ff5bb260df2e`) 
  - Power Saver (`a1841308-3541-4fab-bc81-f71556f20b4a`)
- Automatic brightness adjustment per power plan
- Real-time brightness display with progress bar
- Settings persistence (settings.txt)
- Color support detection (Windows 10+)
- Animated title with dynamic RGB colors
- Centered console window positioning
- Mutex-protected single instance execution

## Usage

1. Run as Administrator (required for power plan changes)
2. Use numeric keys 1-3 to switch power plans
3. Press 4 to access Settings (brightness control must be available)
4. Press ESC to exit

## Default Brightness Settings

- High Performance: 100%
- Balanced: 70%
- Power Saver: 35%

## Settings Menu

- Toggle display brightness on/off
- Adjust brightness values for each power plan (0-100%)
- Settings automatically saved to `settings.txt`

## Requirements

- Windows 10+ (for full color support)
- Administrator privileges
- WMI services enabled (for brightness control)

## Compilation

```bash
g++ main.cpp -o WPM.exe -lole32 -loleaut32 -lwbemuuid -lpdh
```

## Credits
**Author:** [Lonewolf239](https://github.com/Lonewolf239)

**Changelogs:** https://github.com/Lonewolf239/WPM/releases
