#!/bin/bash
# VideoTimeline with Hardware Acceleration
# For dual-GPU laptop: NVIDIA GTX 1650 Ti + AMD Renoir

echo "=== VideoTimeline Hardware Acceleration Setup ==="

# First, install required packages if not present
if ! command -v vainfo &> /dev/null; then
    echo "[INFO] Installing VAAPI tools and drivers..."
    echo "Run: sudo dnf install libva-utils libva-vdpau-driver nvidia-vaapi-driver"
    echo ""
fi

# Detect which GPU to use
if [ "$1" == "--nvidia" ]; then
    echo "[INFO] Using NVIDIA GTX 1650 Ti"
    export __NV_PRIME_RENDER_OFFLOAD=1
    export __GLX_VENDOR_LIBRARY_NAME=nvidia
    export VDPAU_DRIVER=nvidia
    export QT_MEDIA_BACKEND=ffmpeg
elif [ "$1" == "--amd" ]; then
    echo "[INFO] Using AMD Renoir (Integrated)"
    export DRI_PRIME=1
    export LIBVA_DRIVER_NAME=radeonsi
    export QT_MEDIA_BACKEND=ffmpeg
else
    echo "[INFO] Auto-detecting GPU (will try AMD with GStreamer)"
    # Use GStreamer backend for better VAAPI support
    export LIBVA_DRIVER_NAME=radeonsi
    export QT_MEDIA_BACKEND=gstreamer
    export GST_VAAPI_ALL_DRIVERS=1
fi

# Enable Qt multimedia debugging (optional)
if [ "$2" == "--debug" ]; then
    export QT_LOGGING_RULES="qt.multimedia*=true"
    echo "[DEBUG] Qt multimedia logging enabled"
fi

echo "[INFO] Environment configured:"
echo "  QT_MEDIA_BACKEND: $QT_MEDIA_BACKEND"
echo "  LIBVA_DRIVER_NAME: $LIBVA_DRIVER_NAME"
echo "  VDPAU_DRIVER: $VDPAU_DRIVER"
echo "  GST_VAAPI_ALL_DRIVERS: $GST_VAAPI_ALL_DRIVERS"
echo ""

# Collect remaining arguments to pass to run.sh
REMAINING_ARGS=()
SKIP_NEXT=false

for arg in "$@"; do
    if [ "$SKIP_NEXT" = true ]; then
        SKIP_NEXT=false
        continue
    fi
    
    # Skip GPU selection flags
    if [ "$arg" == "--nvidia" ] || [ "$arg" == "--amd" ] || [ "$arg" == "--debug" ]; then
        continue
    fi
    
    REMAINING_ARGS+=("$arg")
done

# Run the application with remaining arguments
echo "[INFO] Launching VideoTimeline..."
if [ ${#REMAINING_ARGS[@]} -gt 0 ]; then
    echo "[INFO] Passing arguments: ${REMAINING_ARGS[@]}"
    ./run.sh "${REMAINING_ARGS[@]}"
else
    ./run.sh
fi
