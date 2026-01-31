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

if(WIN32)
    add_executable(modufile WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(modufile ${SOURCES} ${HEADERS})
endif()

target_link_libraries(modufile PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

if(WIN32)
    set_target_properties(modufile PROPERTIES WIN32_EXECUTABLE ON)
endif()

install(TARGETS modufile DESTINATION bin)
