#!/bin/bash
# Install.sh
#
# This script installs the Canopy desktop environment components:
#   - Builds CanopyWM and CanopyDE using CMake.
#   - Creates the XSession desktop file (/usr/share/xsessions/canopy.desktop)
#   - Installs the startup script (/usr/local/bin/start-canopy)
#   - Creates symlinks for CanopyWM and CanopyDE from their build locations.
#
# Usage:
#   sudo ./Install.sh
#

# Check if running as root.
if [ "$(id -u)" -ne 0 ]; then
    zenity --error --text="This script must be run as root (or with sudo)."
    exit 1
fi

# Define file locations.
XSESSION_DIR="/usr/share/xsessions"
XSESSION_FILE="${XSESSION_DIR}/canopy.desktop"
START_SCRIPT="/usr/local/bin/start-canopy"
CANOPY_WM_LINK="/usr/local/bin/CanopyWM"
CANOPY_DE_LINK="/usr/local/bin/CanopyDE"

# Build directory.
BUILD_DIR="$(pwd)/build"

# Create necessary directories if they don't exist.
mkdir -p "$XSESSION_DIR"
mkdir -p "/usr/local/bin"
mkdir -p "$BUILD_DIR"

###############################################################################
# 1. Build CanopyWM and CanopyDE using CMake.
###############################################################################
zenity --info --text="Starting build process for CanopyWM and CanopyDE."

cd "$BUILD_DIR" || { zenity --error --text="Failed to navigate to build directory."; exit 1; }

cmake .. && make
if [ $? -eq 0 ]; then
    zenity --info --text="Build successful for CanopyWM and CanopyDE."
else
    zenity --error --text="Build failed. Please check the build logs."
    exit 1
fi

###############################################################################
# 2. Create the XSession file for the desktop environment.
###############################################################################
cat > "$XSESSION_FILE" <<EOF
[Desktop Entry]
Name=Canopy Desktop
Comment=Canopy Window Manager with Desktop Environment
Exec=/usr/local/bin/start-canopy
Type=XSession
EOF

if [ $? -eq 0 ]; then
    zenity --info --text="Created XSession file: $XSESSION_FILE"
else
    zenity --error --text="Error: Could not create $XSESSION_FILE"
    exit 1
fi

###############################################################################
# 3. Create the start-canopy startup script.
###############################################################################
cat > "$START_SCRIPT" <<'EOF'
#!/bin/bash
# start-canopy
# This script launches the Canopy Window Manager and Desktop Environment.

# Start the window manager.
CanopyWM &

# Wait a moment for the window manager to initialize.
sleep 1

# Start the desktop environment.
CanopyDE &

# Wait for all background processes.
wait
EOF

# Make the startup script executable.
chmod +x "$START_SCRIPT"
if [ $? -eq 0 ]; then
    zenity --info --text="Created and set executable: $START_SCRIPT"
else
    zenity --error --text="Error: Could not set executable permission on $START_SCRIPT"
    exit 1
fi

###############################################################################
# 4. Create symlinks for CanopyWM and CanopyDE.
###############################################################################
if [ -f "$BUILD_DIR/CanopyWM/CanopyWM" ]; then
    ln -sf "$BUILD_DIR/CanopyWM/CanopyWM" "$CANOPY_WM_LINK"
    chmod +x "$CANOPY_WM_LINK"
    zenity --info --text="Symlink created: $CANOPY_WM_LINK -> $BUILD_DIR/CanopyWM/CanopyWM"
else
    zenity --warning --text="'CanopyWM' not found in the build directory."
fi

if [ -f "$BUILD_DIR/CanopyDE/canopy-de" ]; then
    ln -sf "$BUILD_DIR/CanopyDE/canopy-de" "$CANOPY_DE_LINK"
    chmod +x "$CANOPY_DE_LINK"
    zenity --info --text="Symlink created: $CANOPY_DE_LINK -> $BUILD_DIR/CanopyDE/canopy-de"
else
    zenity --warning --text="'CanopyDE' not found in the build directory."
fi

zenity --info --text="Installation complete. You can now select 'Canopy Desktop' from your login screen."

exit 0
