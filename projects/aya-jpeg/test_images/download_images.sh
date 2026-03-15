#!/usr/bin/env zsh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

ENTRIES=(
    "baboon  https://sipi.usc.edu/database/download.php?vol=misc&img=4.2.03"
    "lake   https://sipi.usc.edu/database/download.php?vol=misc&img=4.2.06"
    "peppers     https://sipi.usc.edu/database/download.php?vol=misc&img=4.2.07"
    "splash https://sipi.usc.edu/database/download.php?vol=misc&img=4.2.01"
)

# Detect ImageMagick binary
if command -v magick &>/dev/null; then
    CONVERT="magick"
elif command -v convert &>/dev/null; then
    CONVERT="convert"
else
    echo "ERROR: ImageMagick not found. Install it:"
    echo "  macOS:  brew install imagemagick"
    echo "  Ubuntu: sudo apt-get install imagemagick"
    exit 1
fi

echo "Using ImageMagick: $CONVERT"

for entry in "${ENTRIES[@]}"; do
    name="${entry%% *}"
    url="${entry##* }"
    tiff_file="${name}.tiff"
    png_file="${name}.png"

    if [ -f "$png_file" ]; then
        echo "  $png_file already exists, skipping."
        continue
    fi

    echo "  Downloading $name..."
    curl -sL "$url" -o "$tiff_file"

    if [ ! -s "$tiff_file" ]; then
        echo "  WARNING: Download failed for $name, skipping."
        rm -f "$tiff_file"
        continue
    fi

    echo "  Converting $tiff_file -> $png_file..."
    $CONVERT "$tiff_file" "$png_file"
    rm -f "$tiff_file"
    echo "  Done: $png_file"
done

echo "All images ready."
