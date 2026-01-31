cmake_minimum_required(VERSION 3.16)
project(modufile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Separate sources
list(FILTER SOURCES EXCLUDE REGEX ".*cli_main.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*main.cpp$")

# GUI Application
if(WIN32)
    add_executable(modufile WIN32 src/main.cpp ${SOURCES} ${HEADERS})
elseif(APPLE)
    add_executable(modufile MACOSX_BUNDLE src/main.cpp ${SOURCES} ${HEADERS})
else()
    add_executable(modufile src/main.cpp ${SOURCES} ${HEADERS})
endif()

target_link_libraries(modufile PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

if(WIN32)
    set_target_properties(modufile PROPERTIES WIN32_EXECUTABLE ON)
endif()

# CLI Application
# Need to filter out MainWindow which depends on Widgets heavily
set(CLI_SOURCES ${SOURCES})
list(FILTER CLI_SOURCES EXCLUDE REGEX ".*mainwindow.*")

set(CLI_HEADERS ${HEADERS})
list(FILTER CLI_HEADERS EXCLUDE REGEX ".*mainwindow.*")

add_executable(modufile-cli src/cli_main.cpp ${CLI_SOURCES} ${CLI_HEADERS})
target_link_libraries(modufile-cli PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui) 

install(TARGETS modufile modufile-cli DESTINATION bin)
