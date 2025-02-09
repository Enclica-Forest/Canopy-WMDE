# CanopyWM - A Modern X11 Window Manager

CanopyWM is a lightweight, feature-rich X11 window manager written in C. It's designed to be fast, customizable, and user-friendly while providing essential window management features.

## Features

### Window Management
- Window focus cycling (Alt + Tab)
- Window moving (Alt + Left Click & Drag)
- Window resizing (Alt + Right Click & Drag)
- Fullscreen toggle (Alt + F)
- Window closing (Alt + Shift + Q)
- Automatic window decorations
- Multi-monitor support

### System Controls
- Volume control
- Screen brightness adjustment
- Multi-display support with RandR
- Desktop notifications
- Wallpaper management

### Desktop Features
- Customizable window borders
- Window snapping
- Desktop notifications
- System tray support
- Cairo-based rendering

## Installation

### Dependencies
```bash
# Debian/Ubuntu
sudo apt install \
    build-essential \
    cmake \
    libx11-dev \
    libcairo2-dev \
    libxrandr-dev \
    libasound2-dev \
    libnotify-dev \
    libsystemd-dev \
    pkg-config

# Arch Linux
sudo pacman -S \
    base-devel \
    cmake \
    libx11 \
    cairo \
    libxrandr \
    alsa-lib \
    libnotify \
    systemd
```

### Building
```bash
git clone https://github.com/yourusername/canopywm.git
cd canopywm
mkdir build
cd build
cmake ..
make
sudo make install
```

## Configuration

CanopyWM looks for its configuration file in the following locations:
1. `$XDG_CONFIG_HOME/canopy/rc` (typically `~/.config/canopy/rc`)
2. `~/.config/canopy/rc`

Example configuration:
```ini
[window]
border_width = 2
border_color = 0x7799AA
focus_color = 0xAA9977
snap_distance = 10

[layout]
master_size = 50
master_count = 1
split_ratio = 0.55

[appearance]
wallpaper_path = /path/to/wallpaper.png
wallpaper_mode = stretch
background_color = 0x2E3440

[system]
volume_step = 5
brightness_step = 10
```

## Usage

### Starting CanopyWM

Add to your `~/.xinitrc`:
```bash
exec canopy-wm
```

Or for display managers, create `/usr/share/xsessions/canopy.desktop`:
```desktop
[Desktop Entry]
Name=Canopy
Comment=Canopy Window Manager
Exec=canopy-wm
Type=Application
```

### Default Keybindings

#### Window Management
- `Alt + Tab`: Cycle through windows
- `Alt + Shift + Q`: Close focused window
- `Alt + F`: Toggle fullscreen
- `Alt + Left Click`: Move window
- `Alt + Right Click`: Resize window

#### System Controls
- `Alt + Up`: Increase volume
- `Alt + Down`: Decrease volume
- `Alt + Shift + Up`: Increase brightness
- `Alt + Shift + Down`: Decrease brightness

### Mouse Controls
- Click to focus windows
- Alt + Left Click to move windows
- Alt + Right Click to resize windows
- Click on window titlebar to raise window

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- X11 for window system protocol
- Cairo for rendering
- systemd for system integration
- libnotify for notifications
- ALSA for audio control

## Known Issues

- Some applications might not respect window decorations
- Multi-monitor setup might need manual configuration
- Some window types might not be handled properly

## Troubleshooting

### Common Issues

1. **Windows not appearing:**
   - Check X11 permissions
   - Verify display connection

2. **No sound control:**
   - Verify ALSA configuration
   - Check user permissions

3. **Multi-monitor issues:**
   - Update RandR configuration
   - Check display arrangement

### Debug Mode

Run with debug output:
```bash
CANOPY_DEBUG=1 canopy-wm
```

## roadmap

- [ ] Workspace support
- [ ] Configuration GUI
- [ ] Plugin system
- [ ] Better multi-monitor support
- [ ] Window rules system