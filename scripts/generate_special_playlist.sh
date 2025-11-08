#!/bin/bash

# Generate a special playlist JSON file from a media folder
# Usage: ./generate_special_playlist.sh <media_folder> <date> <time> <title> [output_file]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check arguments
if [ $# -lt 4 ]; then
    echo -e "${RED}Error: Missing required arguments${NC}"
    echo ""
    echo "Usage: $0 <media_folder> <date> <time> <title> [output_file]"
    echo ""
    echo "Arguments:"
    echo "  media_folder  - Path to folder containing media files"
    echo "  date          - Event date in YYYY-MM-DD format (e.g., 2025-11-10)"
    echo "  time          - Event time in HH:MM format (e.g., 09:05)"
    echo "  title         - Event title (use quotes if contains spaces)"
    echo "  output_file   - Optional: Output file path (default: data/special_playlist.json)"
    echo ""
    echo "Examples:"
    echo "  $0 data/media/special 2025-11-10 09:05 \"Ataturk Commemoration\""
    echo "  $0 data/media/newyear 2026-01-01 00:00 \"New Year\" data/newyear_playlist.json"
    exit 1
fi

MEDIA_FOLDER="$1"
EVENT_DATE="$2"
EVENT_TIME="$3"
EVENT_TITLE="$4"
OUTPUT_FILE="${5:-data/special_playlist.json}"

# Check if folder exists
if [ ! -d "$MEDIA_FOLDER" ]; then
    echo -e "${RED}Error: Folder '$MEDIA_FOLDER' does not exist!${NC}"
    exit 1
fi

# Validate date format
if ! echo "$EVENT_DATE" | grep -qE '^[0-9]{4}-[0-9]{2}-[0-9]{2}$'; then
    echo -e "${RED}Error: Invalid date format '$EVENT_DATE'. Expected YYYY-MM-DD (e.g., 2025-11-10)${NC}"
    exit 1
fi

# Validate time format
if ! echo "$EVENT_TIME" | grep -qE '^[0-9]{2}:[0-9]{2}$'; then
    echo -e "${RED}Error: Invalid time format '$EVENT_TIME'. Expected HH:MM (e.g., 09:05)${NC}"
    exit 1
fi

echo -e "${CYAN}Scanning folder: ${MEDIA_FOLDER}${NC}"

# Find media files
IMAGE_EXTS="jpg|jpeg|png|gif|bmp|webp"
VIDEO_EXTS="mp4|avi|mkv|mov|webm|flv|wmv"

# Get sorted list of files (properly handling filenames with spaces)
mapfile -t MEDIA_FILES < <(find "$MEDIA_FOLDER" -maxdepth 1 -type f \( -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o -iname "*.gif" -o -iname "*.bmp" -o -iname "*.webp" -o -iname "*.mp4" -o -iname "*.avi" -o -iname "*.mkv" -o -iname "*.mov" -o -iname "*.webm" -o -iname "*.flv" -o -iname "*.wmv" \) | sort)

if [ ${#MEDIA_FILES[@]} -eq 0 ]; then
    echo -e "${RED}Error: No media files found in '$MEDIA_FOLDER'${NC}"
    exit 1
fi

echo -e "${GREEN}Found ${#MEDIA_FILES[@]} media file(s):${NC}"

# Start building JSON
ITEMS=""
FIRST_ITEM=true
TOTAL_DURATION=0

while IFS= read -r FILE; do
    FILENAME=$(basename "$FILE")
    EXTENSION="${FILENAME##*.}"
    EXTENSION_LOWER=$(echo "$EXTENSION" | tr '[:upper:]' '[:lower:]')
    ABSOLUTE_PATH=$(realpath "$FILE")
    
    # Determine media type and defaults
    if echo "$EXTENSION_LOWER" | grep -qE "^($IMAGE_EXTS)$"; then
        MEDIA_TYPE="image"
        DURATION=180000  # 3 minutes
        MUTED="true"
        echo -e "  - ${YELLOW}[image]${NC} $FILENAME"
    elif echo "$EXTENSION_LOWER" | grep -qE "^($VIDEO_EXTS)$"; then
        MEDIA_TYPE="video"
        DURATION=300000  # 5 minutes (placeholder)
        MUTED="false"
        echo -e "  - ${YELLOW}[video]${NC} $FILENAME"
    else
        continue
    fi
    
    TOTAL_DURATION=$((TOTAL_DURATION + DURATION))
    
    # Set custom_time for first item
    if [ "$FIRST_ITEM" = true ]; then
        CUSTOM_TIME="$EVENT_TIME"
        FIRST_ITEM=false
    else
        CUSTOM_TIME="NA"
    fi
    
    # Add comma if not first item
    if [ -n "$ITEMS" ]; then
        ITEMS="$ITEMS,"
    fi
    
    # Build item JSON
    ITEMS="$ITEMS
    {
      \"type\": \"$MEDIA_TYPE\",
      \"custom_time\": \"$CUSTOM_TIME\",
      \"url\": \"file://$ABSOLUTE_PATH\",
      \"duration\": $DURATION,
      \"muted\": $MUTED
    }"
done < <(find "$MEDIA_FOLDER" -maxdepth 1 -type f \( -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o -iname "*.gif" -o -iname "*.bmp" -o -iname "*.webp" -o -iname "*.mp4" -o -iname "*.avi" -o -iname "*.mkv" -o -iname "*.mov" -o -iname "*.webm" -o -iname "*.flv" -o -iname "*.wmv" \) | sort)

# Build complete JSON
JSON_OUTPUT="{
  \"special\": true,
  \"title\": \"$EVENT_TITLE\",
  \"date\": \"$EVENT_DATE\",
  \"items\": [$ITEMS
  ]
}"

# Calculate total duration
TOTAL_SECONDS=$((TOTAL_DURATION / 1000))
TOTAL_MINUTES=$((TOTAL_SECONDS / 60))

# Display preview
echo ""
echo -e "${CYAN}============================================================${NC}"
echo -e "${CYAN}PLAYLIST PREVIEW:${NC}"
echo -e "${CYAN}============================================================${NC}"
echo "$JSON_OUTPUT"
echo -e "${CYAN}============================================================${NC}"
echo ""
echo -e "${GREEN}Total duration: ${TOTAL_MINUTES} minutes (${TOTAL_SECONDS} seconds)${NC}"
echo -e "${GREEN}Trigger date/time: ${EVENT_DATE} at ${EVENT_TIME}${NC}"
echo ""

# Confirm before writing
read -p "Write to '$OUTPUT_FILE'? [y/N]: " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Cancelled.${NC}"
    exit 0
fi

# Create output directory if needed
OUTPUT_DIR=$(dirname "$OUTPUT_FILE")
mkdir -p "$OUTPUT_DIR"

# Write to file
echo "$JSON_OUTPUT" > "$OUTPUT_FILE"

echo ""
echo -e "${GREEN}✓ Successfully created: ${OUTPUT_FILE}${NC}"
echo ""
echo -e "${CYAN}To test this special event, run:${NC}"
DATE_FORMATTED=$(echo "$EVENT_DATE" | sed 's/-/:/g')
echo -e "  ${YELLOW}./run.sh --date $DATE_FORMATTED --time $EVENT_TIME${NC}"
