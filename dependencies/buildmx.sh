#!/usr/bin/env bash

echo "ℹ️ Checking prerequisites for building MX..."

# Check if cmake is installed
if ! command -v cmake &> /dev/null
then
    echo "🆘 cmake is not installed."
    echo "🔃 Installing dependencies..."

    # Check if running as root
    if [ "$EUID" -ne 0 ]
    then
        echo "🔐 Sudo permission is required."
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
  echo "ℹ️ Build directory already exists. Deleting..."
  rm -rf "$BUILD_DIR"
fi

echo "ℹ️ Creating build directory..."
mkdir "$BUILD_DIR"
echo "ℹ️ Accessing created directory..."
cd "$BUILD_DIR"

echo "🔧 Configuring build..."
# cmake ../mx -DMX_BUILD_TESTS=off -DMX_BUILD_CORE_TESTS=off -DMX_BUILD_EXAMPLES=off

echo "🏗️ Building..."
# make -j6

echo "✅ Build complete."

rm -rf ../../include/mx/
mkdir ../../include/mx/

echo "🚚 Copying header files..."
cp -R ../mx/Sourcecode/include/mx/api/* ../../include/mx/

echo "🚚 Copying mx/api files..."
cp -R ../mx/Sourcecode/private/mx/api/* ../../include/mx/

echo "🚚 Copying mx/ezxml files..."
cp -R ../mx/Sourcecode/private/mx/ezxml/src/include/ezxml/* ../../include/mx/
cp -R ../mx/Sourcecode/private/mx/ezxml/src/private/private/* ../../include/mx/

echo "🚚 Copying mx/core files..."
cp -R ../mx/Sourcecode/private/mx/core/elements/* ../../include/mx/
cp -R ../mx/Sourcecode/private/mx/core/* ../../include/mx/
rm -rf ../../include/mx/elements

echo "🚚 Copying mx/impl files..."
cp -R ../mx/Sourcecode/private/mx/impl/* ../../include/mx/

echo "🚚 Copying mx/utility files..."
cp -R ../mx/Sourcecode/private/mx/utility/* ../../include/mx/
