@echo off
TITLE Modufile Native C++ Launcher

echo Checking build environment...

:: 1. Build directory check
if not exist build (
    echo Creating build directory...
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
    if %errorlevel% neq 0 (
        echo Error: CMake configuration failed.
        pause
        exit /b %errorlevel%
    )
)

:: 2. Build the application
echo Building Modufile...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Error: Build failed.
    pause
    exit /b %errorlevel%
)

:: 3. Run the executable
if exist build\Release\modufile.exe (
    echo Launching Modufile...
    start "" "build\Release\modufile.exe"
) else if exist build\modufile.exe (
    echo Launching Modufile...
    start "" "build\modufile.exe"
) else (
    echo Error: Could not find modufile.exe
    pause
    exit /b 1
)
