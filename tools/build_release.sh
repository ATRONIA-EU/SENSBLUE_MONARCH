#!/usr/bin/env bash
# build_release.sh — assembles an Arduino-CLI-compatible platform tarball
# from a pinned arduino-esp32 upstream release, adds the SensBlue Monarch
# variant and boards.txt entries, then updates package_sensblue_index.json.
#
# Usage:
#   ./tools/build_release.sh <version> [<upstream_ref>]
#
# Example:
#   ./tools/build_release.sh 0.1.0 3.3.2
#
# Requires: git, tar, bzip2, sha256sum, jq, stat

set -euo pipefail

SB_VERSION="${1:-}"
UPSTREAM_REF="${2:-3.3.2}"

if [[ -z "$SB_VERSION" ]]; then
    echo "usage: $0 <version> [<upstream_ref>]" >&2
    exit 1
fi

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"
STAGE_DIR="$BUILD_DIR/stage"
DIST_DIR="$REPO_ROOT/dist"
ARCHIVE_NAME="sensblue-monarch-$SB_VERSION.tar.bz2"
JSON_FILE="$REPO_ROOT/package_sensblue_index.json"

echo "==> Cleaning..."
rm -rf "$BUILD_DIR"
mkdir -p "$STAGE_DIR" "$DIST_DIR"

echo "==> Fetching arduino-esp32 @ $UPSTREAM_REF..."
git clone --depth 1 --branch "$UPSTREAM_REF" \
    https://github.com/espressif/arduino-esp32.git "$STAGE_DIR"
rm -rf "$STAGE_DIR/.git" "$STAGE_DIR/.github"

echo "==> Injecting SensBlue Monarch variant..."
mkdir -p "$STAGE_DIR/variants/sensblue_monarch"
cp -v "$REPO_ROOT/variants/sensblue_monarch/pins_arduino.h" \
      "$STAGE_DIR/variants/sensblue_monarch/"

echo "==> Appending our boards.txt entries..."
{
    echo ""
    echo "########## SensBlue Monarch ##########"
    cat "$REPO_ROOT/boards_snippet.txt"
} > "$STAGE_DIR/boards.txt"

echo "==> Bundling on-board library..."
mkdir -p "$STAGE_DIR/libraries/SensBlueMonarch"
cp -r "$REPO_ROOT/libraries/SensBlueMonarch/"* \
      "$STAGE_DIR/libraries/SensBlueMonarch/"

echo "==> Building tarball..."
# The Arduino Board Manager expects a single top-level directory inside the
# archive, and that directory name must match the extracted output.
cd "$BUILD_DIR"
mv stage "sensblue-monarch-$SB_VERSION"
tar -cjf "$DIST_DIR/$ARCHIVE_NAME" "sensblue-monarch-$SB_VERSION"
cd - > /dev/null

CHECKSUM="SHA-256:$(sha256sum "$DIST_DIR/$ARCHIVE_NAME" | awk '{print $1}')"
SIZE="$(stat -c%s "$DIST_DIR/$ARCHIVE_NAME")"

echo "==> Updating $JSON_FILE..."
tmp="$(mktemp)"
jq \
    --arg version "$SB_VERSION" \
    --arg archive "$ARCHIVE_NAME" \
    --arg checksum "$CHECKSUM" \
    --arg size "$SIZE" \
    --arg url "https://github.com/sensblue-monarch/arduino-monarch/releases/download/v$SB_VERSION/$ARCHIVE_NAME" \
    '.packages[0].platforms[0].version = $version
     | .packages[0].platforms[0].url = $url
     | .packages[0].platforms[0].archiveFileName = $archive
     | .packages[0].platforms[0].checksum = $checksum
     | .packages[0].platforms[0].size = $size' \
    "$JSON_FILE" > "$tmp"
mv "$tmp" "$JSON_FILE"

echo ""
echo "OK."
echo "  archive:  $DIST_DIR/$ARCHIVE_NAME"
echo "  size:     $SIZE bytes"
echo "  checksum: $CHECKSUM"
echo ""
echo "Next steps:"
echo "  git add package_sensblue_index.json"
echo "  git commit -m \"Release v$SB_VERSION\""
echo "  git tag v$SB_VERSION"
echo "  git push && git push --tags"
echo "  Create a GitHub Release for tag v$SB_VERSION and upload"
echo "  $DIST_DIR/$ARCHIVE_NAME as the release asset."
