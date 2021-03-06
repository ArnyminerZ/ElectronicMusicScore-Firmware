#!/usr/bin/env bash

echo "âšī¸ Checking prerequisites for building MX..."

# Check if cmake is installed
if ! command -v cmake &> /dev/null
then
    echo "đ cmake is not installed."
    echo "đ Installing dependencies..."

    # Check if running as root
    if [ "$EUID" -ne 0 ]
    then
        echo "đ Sudo permission is required."
        sudo apt-get update -y && sudo apt-get install -y cmake build-essential gdb libusb-1.0-0-dev
    else
        apt-get update -y && apt-get install -y cmake build-essential gdb libusb-1.0-0-dev
    fi
fi

# The directory where the script is stored at
BASEDIR=$(dirname "$0")

# Where to store the build files
BUILD_DIR="$BASEDIR/.build-mx"

if [ -d "$BUILD_DIR" ]; then
  # Take action if $DIR exists. #
  echo "âšī¸ Build directory already exists. Deleting..."
  rm -rf "$BUILD_DIR"
fi

echo "âšī¸ Creating build directory..."
mkdir "$BUILD_DIR"
echo "âšī¸ Accessing created directory..."
cd "$BUILD_DIR"

echo "đ§ Configuring build..."
cmake ../mx -DMX_BUILD_TESTS=off -DMX_BUILD_CORE_TESTS=off -DMX_BUILD_EXAMPLES=off

echo "đī¸ Building..."
make -j6

echo "â Build complete."

echo "đ Creating mx libs dir..."
mkdir -p ../../libs/mx

echo "đ Copying .a library..."
cp ../mx/build/libmx.a ../../libs/mx
