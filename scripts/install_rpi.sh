#!/bin/bash
# VideoTimeline Raspberry Pi Installation Script

set -e

echo "VideoTimeline Raspberry Pi Installation"
echo "======================================"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script as root (use sudo)"
    exit 1
fi

# Get the user who called sudo
SUDO_USER=${SUDO_USER:-pi}
USER_HOME=$(eval echo ~$SUDO_USER)

echo "Installing for user: $SUDO_USER"
echo "Home directory: $USER_HOME"

# Install dependencies
echo "Installing dependencies..."
apt-get update
apt-get install -y \
    qt6-base-dev \
    qt6-multimedia-dev \
    libqt6multimedia6 \
    libqt6multimediawidgets6 \
    libqt6network6 \
    xorg \
    openbox \
    lightdm \
    unclutter \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-alsa \
    gstreamer1.0-gl \
    gstreamer1.0-omx \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    ffmpeg \
    libavcodec-extra

# Create application directory
APP_DIR="$USER_HOME/VideoTimeline"
echo "Creating application directory: $APP_DIR"
mkdir -p "$APP_DIR"
mkdir -p "$APP_DIR/data/media"

# Copy application files from build directory
if [ -f "build/bin/VideoTimeline" ]; then
    echo "Copying VideoTimeline executable..."
    cp build/bin/VideoTimeline "$APP_DIR/"
    chmod +x "$APP_DIR/VideoTimeline"
else
    echo "Error: VideoTimeline executable not found in build/bin/"
    echo "Please run ./scripts/build.sh first"
    exit 1
fi

# Copy server executable
if [ -f "build/bin/server" ]; then
    echo "Copying server executable..."
    cp build/bin/server "$APP_DIR/"
    chmod +x "$APP_DIR/server"
else
    echo "Error: server executable not found in build/bin/"
    echo "Please run ./scripts/build.sh first"
    exit 1
fi

# Copy data files
if [ -d "data" ]; then
    echo "Copying data files..."
    cp -r data/* "$APP_DIR/data/"
else
    echo "Warning: data directory not found"
fi

# Set ownership
chown -R $SUDO_USER:$SUDO_USER "$APP_DIR"

# Configure auto-login for display
echo "Configuring auto-login..."
mkdir -p /etc/lightdm/lightdm.conf.d/
cat > /etc/lightdm/lightdm.conf.d/50-videotimeline.conf << EOF
[SeatDefaults]
autologin-user=$SUDO_USER
autologin-user-timeout=0
user-session=openbox
EOF

# Create openbox autostart for fullscreen
mkdir -p "$USER_HOME/.config/openbox"
cat > "$USER_HOME/.config/openbox/autostart" << EOF
# Hide cursor after 1 second of inactivity
unclutter -idle 1 &

# Disable screen saver
xset s off
xset -dpms
xset s noblank

# Wait for system to be ready
sleep 3

# Start VideoTimeline in fullscreen
$APP_DIR/VideoTimeline &
EOF

chmod +x "$USER_HOME/.config/openbox/autostart"
chown -R $SUDO_USER:$SUDO_USER "$USER_HOME/.config"

# Create server systemd service
cat > /etc/systemd/system/videotimeline-server.service << EOF
[Unit]
Description=VideoTimeline Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=$SUDO_USER
Group=$SUDO_USER
WorkingDirectory=$APP_DIR
ExecStart=$APP_DIR/server
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable videotimeline-server.service

echo ""
echo "Installation complete!"
echo "===================="
echo ""
echo "The VideoTimeline client will start automatically when the system boots"
echo "and logs in to the graphical session."
echo ""
echo "To start the server manually:"
echo "  sudo systemctl start videotimeline-server"
echo ""
echo "To check server status:"
echo "  sudo systemctl status videotimeline-server"
echo ""
echo "The server will automatically start on boot."
echo ""
echo "Server will be available at: http://$(hostname -I | awk '{print $1}'):8080"
echo ""
echo "Reboot the system to start everything automatically:"
echo "  sudo reboot"
