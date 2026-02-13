cmake_minimum_required(VERSION 3.14)
project(TimetableGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Turn off all warning-related errors when compiling third-party code
add_compile_options(-Wno-error)

# raylib
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/raylib ${CMAKE_BINARY_DIR}/_deps/raylib-build SYSTEM)

# imgui
add_library(imgui)
file(GLOB imgui-src ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp ${CMAKE_SOURCE_DIR}/thirdparty/imgui/misc/cpp/*.cpp)
target_sources(imgui PRIVATE ${imgui-src})
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/imgui)

# rlImGui
add_library(rlimgui)
file(GLOB rlimgui-src ${CMAKE_SOURCE_DIR}/thirdparty/rlImGui/*.cpp)
target_sources(rlimgui PRIVATE ${rlimgui-src})
target_include_directories(rlimgui PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/rlImGui)
target_link_libraries(rlimgui raylib imgui)

find_package(CURL QUIET)
if (NOT TARGET CURL::libcurl)
    # mbedtls
    set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(MBEDTLS_CONFIG_FILE ${CMAKE_SOURCE_DIR}/include/my_mbedtls_config.h CACHE FILEPATH "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/mbedtls ${CMAKE_BINARY_DIR}/_deps/mbedtls-build SYSTEM)
    set(MBEDTLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/mbedtls/include CACHE PATH "" FORCE)
    set(MBEDTLS_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedtls.a CACHE FILEPATH "" FORCE)
    set(MBEDX509_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedx509.a CACHE FILEPATH "" FORCE)
    set(MBEDCRYPTO_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedcrypto.a CACHE FILEPATH "" FORCE)

    # curl
    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(CURL_USE_MBEDTLS ON CACHE BOOL "" FORCE)
    set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/curl ${CMAKE_BINARY_DIR}/_deps/curl-build SYSTEM)
endif()

# zlib
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/zlib ${CMAKE_BINARY_DIR}/_deps/zlib-build SYSTEM)
set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/zlib;${CMAKE_BINARY_DIR}/_deps/zlib-build" CACHE PATH "" FORCE)
if (WIN32)
    set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a CACHE FILEPATH "" FORCE)
else()
    set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/_deps/zlib-build/libz.a CACHE FILEPATH "" FORCE)
endif()

# libxlsxwriter
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libxlsxwriter ${CMAKE_BINARY_DIR}/_deps/libxlsxwriter-build SYSTEM)

# libzip
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(ENABLE_BZIP2 OFF CACHE BOOL "" FORCE)
set(ENABLE_ZSTD OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libzip ${CMAKE_BINARY_DIR}/_deps/libzip-build SYSTEM)

# utf8cpp
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/utfcpp ${CMAKE_BINARY_DIR}/_deps/utfcpp SYSTEM)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra>
)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib imgui rlimgui CURL::libcurl xlsxwriter zip utf8cpp)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32 secur32 user32 advapi32 wldap32 bcrypt)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()
