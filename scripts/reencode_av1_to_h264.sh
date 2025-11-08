#!/bin/bash

# Re-encode AV1 videos to H.264 for hardware acceleration compatibility
# GTX 1650 Ti supports H.264/H.265 hardware decoding but not AV1

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

show_usage() {
    echo "AV1 to H.264 Video Re-encoder (In-Place Replacement)"
    echo ""
    echo "Usage: $0 <input_directory>"
    echo ""
    echo "Arguments:"
    echo "  input_directory   Directory containing AV1 videos to convert"
    echo ""
    echo "Options:"
    echo "  -h, --help        Show this help message"
    echo "  --crf VALUE       Set CRF quality (18-28, default: 23, lower=better)"
    echo "  --preset PRESET   Set encoding preset (ultrafast|fast|medium|slow, default: medium)"
    echo "  --hwaccel         Use NVIDIA hardware encoder (h264_nvenc) if available"
    echo "  --backup          Create .bak backup of original files"
    echo ""
    echo "Examples:"
    echo "  $0 data/media/special"
    echo "  $0 data/media/special --crf 20 --preset slow"
    echo "  $0 data/media/special --hwaccel --backup"
    echo ""
    echo "WARNING: This will REPLACE original AV1 files with H.264 versions!"
    echo "         Use --backup to keep a copy of originals with .bak extension."
}

# Default settings
CRF=23
PRESET="medium"
USE_HWACCEL=false
CREATE_BACKUP=false

# Parse arguments
INPUT_DIR=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            show_usage
            exit 0
            ;;
        --crf)
            CRF="$2"
            shift 2
            ;;
        --preset)
            PRESET="$2"
            shift 2
            ;;
        --hwaccel)
            USE_HWACCEL=true
            shift
            ;;
        --backup)
            CREATE_BACKUP=true
            shift
            ;;
        *)
            if [ -z "$INPUT_DIR" ]; then
                INPUT_DIR="$1"
            else
                print_error "Unknown argument: $1"
                show_usage
                exit 1
            fi
            shift
            ;;
    esac
done

# Validate input directory
if [ -z "$INPUT_DIR" ]; then
    print_error "Input directory is required"
    show_usage
    exit 1
fi

if [ ! -d "$INPUT_DIR" ]; then
    print_error "Input directory does not exist: $INPUT_DIR"
    exit 1
fi

print_warning "=========================================="
print_warning "WARNING: This will REPLACE original files!"
print_warning "=========================================="
print_info "Re-encoding AV1 videos to H.264 (in-place)"
print_info "Input directory: $INPUT_DIR"
print_info "CRF: $CRF (lower = better quality)"
print_info "Preset: $PRESET"
print_info "Backup originals: $([ "$CREATE_BACKUP" = true ] && echo "YES (.bak)" || echo "NO")"

if [ "$USE_HWACCEL" = true ]; then
    print_info "Hardware encoding: ENABLED (h264_nvenc)"
    ENCODER="h264_nvenc"
    ENCODER_OPTS="-preset $PRESET -cq $CRF"
else
    print_info "Software encoding: libopenh264"
    ENCODER="libopenh264"
    ENCODER_OPTS="-b:v 2M"  # libopenh264 doesn't support CRF, use bitrate
fi

# Check for ffmpeg
if ! command -v ffmpeg >/dev/null 2>&1; then
    print_error "ffmpeg is not installed. Please install it first."
    exit 1
fi

# Find all video files (common extensions)
print_info "Scanning for video files..."
file_count=0
converted_count=0
skipped_count=0
failed_count=0

while IFS= read -r -d $'\0' input_file; do
    ((file_count++)) || true
    
    filename=$(basename "$input_file")
    temp_file="${input_file}.h264_temp.mp4"
    
    print_info "Converting: $filename"
    
    # Check if input needs conversion (AV1 or VP9)
    codec=$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$input_file" 2>/dev/null || echo "unknown")
    
    if [ "$codec" != "av1" ] && [ "$codec" != "vp9" ]; then
        if [ "$codec" = "h264" ]; then
            print_warning "  Skipping: Already H.264"
        else
            print_warning "  Skipping: Not AV1/VP9 (codec: $codec)"
        fi
        ((skipped_count++)) || true
        continue
    fi
    
    print_info "  Source codec: $codec"
    
    # Re-encode with ffmpeg to temporary file
    if ffmpeg -y -hide_banner -loglevel error -stats \
        -i "$input_file" \
        -c:v $ENCODER $ENCODER_OPTS \
        -c:a copy \
        -movflags +faststart \
        "$temp_file"; then
        
        # Get file sizes
        input_size=$(du -h "$input_file" | cut -f1)
        output_size=$(du -h "$temp_file" | cut -f1)
        
        # Backup original if requested
        if [ "$CREATE_BACKUP" = true ]; then
            mv "$input_file" "${input_file}.bak"
            print_info "  Backed up: ${filename}.bak"
        fi
        
        # Replace original with converted file
        mv "$temp_file" "$input_file"
        
        print_success "  Converted: $filename ($input_size → $output_size)"
        ((converted_count++)) || true
    else
        print_error "  Failed to convert: $filename"
        # Clean up temp file on failure
        rm -f "$temp_file"
        ((failed_count++)) || true
    fi
    
done < <(find "$INPUT_DIR" -maxdepth 1 -type f \( -iname "*.mp4" -o -iname "*.mkv" -o -iname "*.webm" -o -iname "*.avi" \) -print0)

echo ""
print_info "========== Summary =========="
print_info "Total files scanned: $file_count"
print_success "Successfully converted: $converted_count"
print_warning "Skipped: $skipped_count"
if [ $failed_count -gt 0 ]; then
    print_error "Failed: $failed_count"
fi
echo ""

if [ $converted_count -gt 0 ]; then
    print_success "Files have been converted to H.264 in-place!"
    if [ "$CREATE_BACKUP" = true ]; then
        print_info "Original AV1 files backed up with .bak extension"
    fi
    print_info ""
    print_info "Next steps:"
    print_info "1. Test playback with: ./run.sh --date 10:11:2025 --time 09:05"
    print_info "2. Videos should now play with hardware acceleration"
    if [ "$CREATE_BACKUP" = true ]; then
        print_info "3. If satisfied, you can delete .bak files to save space"
    fi
fi

exit 0
