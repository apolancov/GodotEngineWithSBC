#!/bin/bash

set -e

# Argumentos con valores por defecto
ARCH=${1:-arm64}
USE_LLVM=${2:-1}
BUILD_TYPE=${3:-release}
CUSTOM_NAME=${4:-sbc_4_4_1}

# Rutas y variables
SYSROOT="/srv/chroot/ubuntu-arm64"
OUTPUT_DIR="bin"
BUILD_BIN="godot.sbc.template_${BUILD_TYPE}.${ARCH}.llvm"
[[ "$USE_LLVM" != "1" ]] && BUILD_BIN="godot.sbc.template_${BUILD_TYPE}.${ARCH}"
FINAL_BIN="${CUSTOM_NAME}"
DIST_DIR="dist"
SQUASHFS_NAME="${FINAL_BIN}.squashfs"
TAR_NAME="${FINAL_BIN}.tar.xz"
PKG_CONFIG_PATH="$SYSROOT/usr/lib/aarch64-linux-gnu/pkgconfig:$SYSROOT/usr/share/pkgconfig"

# Mostrar configuración
echo "🧱 Starting build for SBC:"
echo "   ➤ Architecture: $ARCH"
echo "   ➤ Use LLVM: $USE_LLVM"
echo "   ➤ Build Type: $BUILD_TYPE"
echo "   ➤ Final Name: $FINAL_BIN"

# Limpiar carpeta de salida
echo "🧹 Limpiando carpeta '$DIST_DIR/'..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Flags para SCons
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

# Exportar PKG_CONFIG_PATH si aplica
if [[ "$ARCH" == "arm64" && "$(uname -m)" != "aarch64" ]]; then
  export PKG_CONFIG_PATH
  echo "📦 PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
fi

# Compilación
echo "🚀 Compilando con SCons..."
scons "${SCONS_FLAGS[@]}"

# Verificación del binario generado
BIN_PATH="${OUTPUT_DIR}/${BUILD_BIN}"
if [[ ! -f "$BIN_PATH" ]]; then
  echo "❌ Binario no encontrado: $BIN_PATH"
  exit 1
fi

# Copiar y renombrar binario
cp "$BIN_PATH" "$DIST_DIR/$FINAL_BIN"
chmod +x "$DIST_DIR/$FINAL_BIN"
echo "📦 Binario copiado a $DIST_DIR/$FINAL_BIN"

# Crear squashfs dentro de dist
if command -v mksquashfs &>/dev/null; then
  echo "📦 Creando imagen squashfs en $DIST_DIR/$SQUASHFS_NAME"
  mksquashfs "$DIST_DIR" "$DIST_DIR/$SQUASHFS_NAME" -comp xz -noappend
else
  echo "⚠️ mksquashfs no disponible. Saltando squashfs."
fi

# Crear tar.xz dentro de dist
# echo "📦 Creando paquete tar.xz en $DIST_DIR/$TAR_NAME"
# tar -caf "$DIST_DIR/$TAR_NAME" -C "$DIST_DIR" "$FINAL_BIN"

# Mostrar resultado
echo "✅ Archivos generados:"
ls -lh "$DIST_DIR/$FINAL_BIN" "$DIST_DIR/$TAR_NAME" "$DIST_DIR/$SQUASHFS_NAME" 2>/dev/null || true

