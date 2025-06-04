cmake_minimum_required(VERSION 3.28)
project(TimetableGenerator)

if (UNIX)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland support")
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support")
endif()

include(FetchContent)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.5)
set(IMGUI_VERSION 1.91.9b)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(raylib)

# Imgui
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip
)
FetchContent_MakeAvailable(imgui)
add_library(imgui)
file(GLOB imgui-src ${imgui_SOURCE_DIR}/*.cpp ${imgui_SOURCE_DIR}/misc/cpp/*.cpp)
target_sources(imgui PRIVATE ${imgui-src})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# rlImGui
FetchContent_Declare(
    rlimgui
    URL https://github.com/raylib-extras/rlImGui/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(rlimgui)
add_library(rlimgui)
file(GLOB rlimgui-src ${rlimgui_SOURCE_DIR}/*.cpp)
target_sources(rlimgui PRIVATE ${rlimgui-src})
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui raylib imgui)

# Our Project
file(GLOB SOURCES src/*.cpp)

add_executable(main ${SOURCES})
target_include_directories(main PRIVATE include/)
target_compile_features(main PRIVATE cxx_std_17)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(main raylib imgui rlimgui)

# Web Configurations
if ("${PLATFORM}" STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(main PROPERTIES SUFFIX ".html")
    target_link_options(main PUBLIC
        "-sASSERTIONS=1"
        "-sFORCE_FILESYSTEM"
        "-sALLOW_MEMORY_GROWTH"
        "-sUSE_GLFW=3"
        "-sEXPORTED_RUNTIME_METHODS=['HEAPF32']"
        "--preload-file=${CMAKE_SOURCE_DIR}/resources@resources"
        "--preload-file=${CMAKE_SOURCE_DIR}/settings.txt@settings.txt"
    )
    if ("${SHELL}" STREQUAL "Minimal")
        target_link_options(main PUBLIC "--shell-file" "${CMAKE_SOURCE_DIR}/src/shell_minimal.html")
    endif()
endif()


# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(main "-framework IOKit")
    target_link_libraries(main "-framework Cocoa")
    target_link_libraries(main "-framework OpenGL")
endif()
