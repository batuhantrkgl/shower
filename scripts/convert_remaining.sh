#!/bin/bash
# Quick script to convert remaining files without bash history expansion issues

set +H  # Disable history expansion
cd "$(dirname "$0")/../data/media/special" || exit 1

echo "Converting remaining AV1 and VP9 files..."

# Array of files to convert
files=(
    "10 Kasım ｜ Milletin Seni Hep #yaşATAcak!.mp4"
    "10 Kasım ｜ #SenHepBuradasın.mp4"
    "Hiç Ölmedin.mp4"
    "Minnettarız.mp4"
)

# Determine encoder (prefer h264_nvenc if working, fallback to libx264)
ENCODER="libx264"
ENCODER_OPTS="-preset medium -crf 23"
if ffmpeg -encoders 2>/dev/null | grep -q "h264_nvenc"; then
    if ffmpeg -f lavfi -i color=c=black:s=64x64:d=0.04 -c:v h264_nvenc -f null - 2>/dev/null; then
        ENCODER="h264_nvenc"
        ENCODER_OPTS="-preset medium -cq 23"
    fi
fi
echo "Using video encoder: $ENCODER"

for file in "${files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Skipping (not found): $file"
        continue
    fi
    
    echo "Converting: $file"
    temp_file="${file}.tmp.$$.mp4"
    
    if ffmpeg -y -hide_banner -loglevel error -stats \
        -i "$file" \
        -c:v $ENCODER $ENCODER_OPTS \
        -c:a aac -b:a 192k \
        -movflags +faststart \
        "$temp_file"; then
        
        mv "$file" "${file}.bak"
        mv "$temp_file" "$file"
        echo "✓ Converted: $file"
    else
        echo "✗ Failed: $file"
        rm -f "$temp_file"
    fi
    echo ""
done

echo "Done! All files converted."
