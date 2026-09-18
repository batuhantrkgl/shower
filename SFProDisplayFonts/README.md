# SF Pro Display Fonts

System-wide and application font assets for VideoTimeline and desktop systems.

## Linux Installation (All Distributions)

Run the universal font installer script:

```bash
# System-wide installation (requires sudo/root):
sudo ./install_fonts.sh

# Or install for the current user only:
./install_fonts.sh --user
```

Supported distributions:
- Ubuntu / Debian / Raspberry Pi OS / Linux Mint (`apt`)
- Fedora / RHEL / CentOS / Rocky / Alma (`dnf` / `yum`)
- Arch Linux / Manjaro (`pacman`)
- openSUSE / SLES (`zypper`)
- Alpine Linux (`apk`)
- Void Linux (`xbps`)

> Note: `install_fedora.sh` is preserved as a backward-compatible wrapper that calls `install_fonts.sh`.

## Windows 11 Installation

To use SF Pro Display as system font on Windows 11:
1. Select all `.otf` and `.ttf` files in this folder, right-click and choose **Install for all users** (or **Install**).
2. Follow the registry tweaks provided if replacing the Windows shell font.
