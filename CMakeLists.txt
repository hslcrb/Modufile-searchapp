cmake_minimum_required(VERSION 3.14)
project(modufile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv)
include(${wxWidgets_USE_FILE})

# Source files
file(GLOB SOURCES "src/*.cpp" "src/*.h")

# Executable
if (WIN32)
    add_executable(modufile WIN32 ${SOURCES})
else()
    add_executable(modufile ${SOURCES})
endif()

# Link libraries
target_link_libraries(modufile PRIVATE ${wxWidgets_LIBRARIES})

# Include directories
target_include_directories(modufile PRIVATE src)

# Installation
install(TARGETS modufile DESTINATION bin)
