#!/bin/bash

set -e

# Arguments with default values
ARCH=${1:-arm64}
USE_LLVM=${2:-1}
BUILD_TYPE=${3:-release}
CUSTOM_NAME=${4:-sbc_4_5_1}

# Paths and variables
SYSROOT="/srv/chroot/ubuntu-arm64"
OUTPUT_DIR="bin"
BUILD_BIN="godot.sbc.template_${BUILD_TYPE}.${ARCH}.llvm"
[[ "$USE_LLVM" != "1" ]] && BUILD_BIN="godot.sbc.template_${BUILD_TYPE}.${ARCH}"
FINAL_BIN="${CUSTOM_NAME}"
DIST_DIR="dist"
SQUASHFS_NAME="${FINAL_BIN}.squashfs"
TAR_NAME="${FINAL_BIN}.tar.xz"
PKG_CONFIG_PATH="$SYSROOT/usr/lib/aarch64-linux-gnu/pkgconfig:$SYSROOT/usr/share/pkgconfig"

# Show configuration
echo "🧱 Starting build for SBC:"
echo "   ➤ Architecture: $ARCH"
echo "   ➤ Use LLVM: $USE_LLVM"
echo "   ➤ Build Type: $BUILD_TYPE"
echo "   ➤ Final Name: $FINAL_BIN"

# Clean output directory
echo "🧹 Cleaning directory '$DIST_DIR/'..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Flags for SCons
SCONS_FLAGS=(
  "platform=sbc"
  "arch=${ARCH}"
  "use_llvm=${USE_LLVM}"
  "use_static_cpp=yes"
  "vulkan=yes"
  "opengl3=yes"
  "target=template_${BUILD_TYPE}"
  "progress=yes"
  "-j$(nproc)"
)

# Export PKG_CONFIG_PATH if applicable
if [[ "$ARCH" == "arm64" && "$(uname -m)" != "aarch64" ]]; then
  export PKG_CONFIG_PATH
  echo "📦 PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
fi

# Compilation
echo "🚀 Compiling with SCons..."
scons "${SCONS_FLAGS[@]}"

# Verification of the generated binary
BIN_PATH="${OUTPUT_DIR}/${BUILD_BIN}"
if [[ ! -f "$BIN_PATH" ]]; then
  echo "❌ Binary not found: $BIN_PATH"
  exit 1
fi

# Copy and rename binary
cp "$BIN_PATH" "$DIST_DIR/$FINAL_BIN"
chmod +x "$DIST_DIR/$FINAL_BIN"
echo "📦 Binary copied to $DIST_DIR/$FINAL_BIN"

# Create squashfs inside dist
if command -v mksquashfs &>/dev/null; then
  echo "📦 Creating squashfs image at $DIST_DIR/$SQUASHFS_NAME"
  mksquashfs "$DIST_DIR" "$DIST_DIR/$SQUASHFS_NAME" -comp xz -noappend
else
  echo "⚠️ mksquashfs not available. Skipping squashfs."
fi

# Create tar.xz inside dist
# echo "📦 Creating tar.xz package at $DIST_DIR/$TAR_NAME"
# tar -caf "$DIST_DIR/$TAR_NAME" -C "$DIST_DIR" "$FINAL_BIN"

# Show result
echo "✅ Generated files:"
ls -lh "$DIST_DIR/$FINAL_BIN" "$DIST_DIR/$TAR_NAME" "$DIST_DIR/$SQUASHFS_NAME" 2>/dev/null || true

