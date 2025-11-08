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

for file in "${files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Skipping (not found): $file"
        continue
    fi
    
    echo "Converting: $file"
    
    if ffmpeg -y -hide_banner -loglevel error -stats \
        -i "$file" \
        -c:v h264_nvenc -preset medium -cq 23 \
        -c:a copy \
        -movflags +faststart \
        "temp_convert.mp4"; then
        
        mv "$file" "${file}.bak"
        mv "temp_convert.mp4" "$file"
        echo "✓ Converted: $file"
    else
        echo "✗ Failed: $file"
        rm -f "temp_convert.mp4"
    fi
    echo ""
done

echo "Done! All files converted."
