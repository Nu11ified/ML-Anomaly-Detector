#!/bin/bash

# Cross-platform build script for AnomDetect

set -e

echo "🚀 Building AnomDetect..."

# Detect platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    echo "📱 Detected platform: macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    echo "🐧 Detected platform: Linux"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="windows"
    echo "🪟 Detected platform: Windows"
else
    echo "❌ Unsupported platform: $OSTYPE"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "⚙️  Configuring CMake..."
cmake ..

# Build
echo "🔨 Building..."
if [[ "$PLATFORM" == "windows" ]]; then
    cmake --build . --config Release
else
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

echo "✅ Build complete!"
echo "📦 Executable location: build/bin/anom_detect_${PLATFORM}"

# Test run (optional)
if [[ "$1" == "--test" ]]; then
    echo "🧪 Running test..."
    if [[ "$PLATFORM" == "windows" ]]; then
        timeout 10s ./bin/Release/anom_detect_windows.exe || true
    else
        timeout 10s ./bin/anom_detect_${PLATFORM} || true
    fi
    echo "✅ Test complete!"
fi

echo ""
echo "🎉 Build successful! Run with:"
if [[ "$PLATFORM" == "windows" ]]; then
    echo "   ./build/bin/Release/anom_detect_windows.exe"
else
    echo "   ./build/bin/anom_detect_${PLATFORM}"
fi 