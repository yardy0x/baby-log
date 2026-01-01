# Baby Log v0.1

A Flipper Zero application for tracking baby activities - feeding, sleeping, and diaper changes.

P.S.: all these phone trackers are so clumsy, I don't want to unlock my phone in the middle of the night to find an app and start "feeding" and the unlock it again to finish it. Just click one button on your Flipper and it started logging your feeding/sleeping/diaper etc. 

![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-External%20App-orange)
![Version](https://img.shields.io/badge/version-0.1-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

### Quick Actions (Main Screen)
- **Feed** (↑ button) - Start/stop feeding timer
- **Sleep** (↓ button) - Start/stop sleep timer  
- **Diaper** (← button) - Log diaper change
- **Menu** (Back button) - Open menu

### Animated Bird Companion
A cute animated bird shows the current state:
- 🐦 **Awake** - Bird standing, blinking occasionally
- 🍽️ **Feeding** - Bird pecking at food
- 💤 **Sleeping** - Bird with floating "Zzz" animation
- 💕 **Diaper logged** - Happy bird with hearts

### Activity Tracking
- Automatic duration tracking for feeding and sleep
- Mutual exclusivity - starting one activity stops the other
- Visual feedback when logging diaper changes
- Timestamps for all events

### Child Profile
- Set child's name (up to 12 characters)
- Set birth date
- Automatic age calculation (displayed as "Xm Yd")

### Recent Log
- View last 10 events
- Shows time and event type
- Scrollable list

### Data Management
- Delete last event (soft delete)
- Clear all logs
- Persistent storage on SD card

### Multi-Language Support
- English
- Russian (Latin transliteration)
- Spanish

## Installation

### Building from Source

1. Clone the Flipper Zero firmware:
```bash
git clone https://github.com/flipperdevices/flipperzero-firmware.git
cd flipperzero-firmware
```

2. Copy the `baby_log` folder to `applications_user/`:
```bash
cp -r baby_log applications_user/
```

3. Build the app:
```bash
./fbt fap_baby_log
```

4. The compiled `.fap` file will be at:
```
build/f7-firmware-D/.extapps/baby_log.fap
```

### Installing on Flipper Zero

Copy `baby_log.fap` to your Flipper Zero's SD card:
```
/ext/apps/Tools/baby_log.fap
```

Or use qFlipper to transfer the file.

## Usage

### Main Screen Controls
| Button | Action |
|--------|--------|
| ↑ UP | Toggle feeding |
| ↓ DOWN | Toggle sleep |
| ← LEFT | Log diaper |
| ◀ BACK | Open menu |

### Menu Options
1. **Child profile** - View/edit name and birth date
2. **Recent log** - View recent events
3. **Delete last** - Remove last logged event
4. **Clear all logs** - Delete all event history
5. **Language** - Change app language
6. **Exit** - Close the app

### Edit Name Screen
| Button | Action |
|--------|--------|
| ↑ UP | Next letter (A→Z→Space) |
| ↓ DOWN | Previous letter |
| ← LEFT | Move cursor left |
| → RIGHT | Move cursor right |
| ◀ BACK | Delete character / Save & exit |
| ○ OK | Confirm name |

### Edit Birth Date Screen
| Button | Action |
|--------|--------|
| ↑ UP | Increase value |
| ↓ DOWN | Decrease value |
| → RIGHT / OK | Next field |
| ◀ BACK | Save & exit |

## File Structure

The app stores data on the SD card at `/ext/apps/baby_log/`:

```
/ext/apps/baby_log/
├── config.json    # Child name, birth date, language
├── state.json     # Active timers (restored on restart)
├── log.jsonl      # Event log (JSON lines format)
├── deleted.jsonl  # Soft-deleted event IDs
└── debug.log      # Debug information (for troubleshooting)
```

### Log Entry Format
```json
{"ts":"2026-01-01T12:30:00","type":"feeding_start","id":"abc123"}
{"ts":"2026-01-01T12:45:00","type":"feeding_stop","id":"def456","dur":900}
{"ts":"2026-01-01T13:00:00","type":"diaper","id":"ghi789"}
```

## Technical Details

- **Platform**: Flipper Zero
- **SDK**: Furi
- **Language**: C
- **Screen**: 128x64 pixels monochrome
- **Memory**: No dynamic allocation (malloc-free)
- **Storage**: JSON/JSONL files on SD card

## Troubleshooting

### App crashes when viewing logs
Check the `debug.log` file on SD card for the last successful operation. This helps identify where the crash occurred.

### Events not saving
Ensure the SD card is properly inserted and has free space.

### Language shows strange characters
Russian uses Latin transliteration because Flipper Zero fonts don't support Cyrillic characters.

## Version History

### v0.1 (Current)
- Initial release
- Quick actions for feeding, sleeping, diaper
- Animated bird companion
- Child profile with name and birth date
- Recent log viewer
- Delete last event / Clear all logs
- Multi-language support (EN, RU, ES)
- Age display in months and days
- Mutual exclusivity for feeding/sleeping
- Visual feedback for diaper logging
- Debug logging for troubleshooting

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

MIT License - See LICENSE file for details.

## Credits

Created for parents who want a simple, offline way to track their baby's activities using Flipper Zero.

---

**Made with ❤️ for tired parents everywhere**

