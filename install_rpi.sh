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
    python3 \
    python3-pip \
    xorg \
    openbox \
    lightdm

# Create application directory
APP_DIR="$USER_HOME/VideoTimeline"
echo "Creating application directory: $APP_DIR"
mkdir -p "$APP_DIR"

# Copy application files (assuming they're in current directory)
if [ -f "VideoTimeline" ]; then
    echo "Copying VideoTimeline executable..."
    cp VideoTimeline "$APP_DIR/"
    chmod +x "$APP_DIR/VideoTimeline"
else
    echo "Warning: VideoTimeline executable not found in current directory"
fi

# Copy server files
if [ -d "server" ]; then
    echo "Copying server files..."
    cp -r server "$APP_DIR/"
    chmod +x "$APP_DIR/server/server.py"
else
    echo "Warning: server directory not found"
fi

# Set ownership
chown -R $SUDO_USER:$SUDO_USER "$APP_DIR"

# Install systemd service for client
echo "Installing systemd service for VideoTimeline client..."
cp videotimeline.service /etc/systemd/system/
# Update paths in service file
sed -i "s|/home/pi|$USER_HOME|g" /etc/systemd/system/videotimeline.service
sed -i "s|User=pi|User=$SUDO_USER|g" /etc/systemd/system/videotimeline.service
sed -i "s|Group=pi|Group=$SUDO_USER|g" /etc/systemd/system/videotimeline.service

# Enable the service
systemctl daemon-reload
systemctl enable videotimeline.service

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

# Start VideoTimeline
$APP_DIR/VideoTimeline &
EOF

chown -R $SUDO_USER:$SUDO_USER "$USER_HOME/.config"

# Create server start script
cat > "$APP_DIR/start_server.sh" << 'EOF'
#!/bin/bash
cd "$(dirname "$0")/server"
python3 server.py
EOF

chmod +x "$APP_DIR/start_server.sh"
chown $SUDO_USER:$SUDO_USER "$APP_DIR/start_server.sh"

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
WorkingDirectory=$APP_DIR/server
ExecStart=/usr/bin/python3 server.py
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
echo "To start the server manually:"
echo "  sudo systemctl start videotimeline-server"
echo ""
echo "To start the client manually:"
echo "  sudo systemctl start videotimeline"
echo ""
echo "The system will automatically start both services on boot."
echo ""
echo "Server will be available at: http://$(hostname -I | awk '{print $1}'):8080"
echo ""
echo "Reboot the system to start everything automatically:"
echo "  sudo reboot"
