#!/bin/bash

# Modufile Native C++ - One-click Launcher for Linux/macOS

set -e

# 1. Check for build directory
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
fi

# 2. Build the application
echo "Building Modufile..."
cmake --build build --config Release

# 3. Locate and run the executable
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    APP_PATH="build/modufile.app"
    if [ -d "$APP_PATH" ]; then
        echo "Launching Modufile App..."
        open "$APP_PATH"
    else
        echo "Error: Application bundle not found."
        exit 1
    fi
else
    # Linux
    EXE_PATH="build/modufile"
    if [ -f "$EXE_PATH" ]; then
        echo "Launching Modufile..."
        ./"$EXE_PATH" &
    else
        echo "Error: Executable not found."
        exit 1
    fi
fi
