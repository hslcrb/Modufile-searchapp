cmake_minimum_required(VERSION 3.14)
project(modufile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)

# WebView
FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview.git
    GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(webview)

# Pthread (for Linux)
if(UNIX AND NOT APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Executable
if (WIN32)
    add_executable(modufile WIN32 ${SOURCES})
else()
    add_executable(modufile ${SOURCES})
endif()

# Include directories
target_include_directories(modufile PRIVATE ${webview_SOURCE_DIR})

# Link libraries
target_link_libraries(modufile PRIVATE webview)
if(UNIX AND NOT APPLE)
    target_link_libraries(modufile PRIVATE Threads::Threads)
endif()
if(APPLE)
    target_link_libraries(modufile PRIVATE "-framework WebKit" "-framework Cocoa")
endif()

# Installation
install(TARGETS modufile DESTINATION bin)
install(DIRECTORY ui DESTINATION share/modufile)
