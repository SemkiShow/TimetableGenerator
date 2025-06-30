cmake_minimum_required(VERSION 3.14)
project(TimetableGenerator)

if (UNIX)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland support")
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support")
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
include(ExternalProject)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 5.5)
set(IMGUI_VERSION 1.91.9b)

message(STATUS "Downloading required libraries...")

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(raylib)

# Imgui
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}-docking.zip
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

# Curl
find_package(CURL QUIET)

if (NOT TARGET CURL::libcurl)
    set(LIBCURL_VERSION 8_14_1)
    set(LIBPSL_VERSION 0.21.5)
    set(MBEDTLS_VERSION 3.6.3.1)

    # MbedTLS
    set(MBEDTLS_INSTALL_DIR ${CMAKE_BINARY_DIR}/mbedtls-install)
    ExternalProject_Add(
        mbedtls
        URL https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v${MBEDTLS_VERSION}.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/_deps/mbedtls
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${MBEDTLS_INSTALL_DIR}
            -DENABLE_TESTING=OFF
            -DENABLE_PROGRAMS=OFF
            -DMBEDTLS_CONFIG_FILE=${CMAKE_SOURCE_DIR}/include/my_mbedtls_config.h
    )

    # Libpsl
    set(LIBPSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/libpsl-install)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        ExternalProject_Add(
            libpsl
            URL https://github.com/rockdaboot/libpsl/archive/${LIBPSL_VERSION}.zip
            PREFIX ${CMAKE_BINARY_DIR}/_deps/libpsl
            CONFIGURE_COMMAND ./autogen.sh && ./configure --host=x86_64-w64-mingw32 --prefix=${LIBPSL_INSTALL_DIR} --disable-runtime --disable-shared --enable-static
            BUILD_COMMAND curl -o list/public_suffix_list.dat https://publicsuffix.org/list/public_suffix_list.dat && make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
        )
    elseif (UNIX)
        ExternalProject_Add(
            libpsl
            URL https://github.com/rockdaboot/libpsl/archive/${LIBPSL_VERSION}.zip
            PREFIX ${CMAKE_BINARY_DIR}/_deps/libpsl
            CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=${LIBPSL_INSTALL_DIR} --disable-runtime --disable-shared --enable-static
            BUILD_COMMAND curl -o list/public_suffix_list.dat https://publicsuffix.org/list/public_suffix_list.dat && make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
        )
    endif()

    set(LIBCURL_INSTALL_DIR ${CMAKE_BINARY_DIR}/libcurl-install)
    ExternalProject_Add(libcurl
        URL https://github.com/curl/curl/archive/curl-${LIBCURL_VERSION}.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libcurl
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${LIBCURL_INSTALL_DIR}
            -DCURL_USE_MBEDTLS=ON
            -DMBEDTLS_INCLUDE_DIR=${MBEDTLS_INSTALL_DIR}/include
            -DMBEDTLS_LIBRARY=${MBEDTLS_INSTALL_DIR}/lib/libmbedtls.a
            -DMBEDX509_LIBRARY=${MBEDTLS_INSTALL_DIR}/lib/libmbedx509.a
            -DMBEDCRYPTO_LIBRARY=${MBEDTLS_INSTALL_DIR}/lib/libmbedcrypto.a
            -DCURL_USE_LIBPSL=ON
            -DLIBPSL_INCLUDE_DIR=${LIBPSL_INSTALL_DIR}/include
            -DLIBPSL_LIBRARY=${LIBPSL_INSTALL_DIR}/lib/libpsl.a
            -DCURL_STATICLIB=ON
            -DBUILD_SHARED_LIBS=OFF
            -DBUILD_CURL_EXE=OFF
        DEPENDS mbedtls libpsl
    )
endif()

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(main ${SOURCES})
target_include_directories(main PRIVATE include/ ${httplib_SOURCE_DIR})
target_compile_features(main PRIVATE cxx_std_17)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(main PRIVATE raylib imgui rlimgui)
if (NOT TARGET CURL::libcurl)
    add_dependencies(main libcurl)
    target_include_directories(main PRIVATE
        ${LIBCURL_INSTALL_DIR}/include
        ${MBEDTLS_INSTALL_DIR}/include
        ${LIBPSL_INSTALL_DIR}/include)
    target_link_libraries(main PUBLIC
        ${LIBCURL_INSTALL_DIR}/lib/libcurl.a
        ${MBEDTLS_INSTALL_DIR}/lib/libmbedtls.a
        ${MBEDTLS_INSTALL_DIR}/lib/libmbedcrypto.a
        ${MBEDTLS_INSTALL_DIR}/lib/libmbedx509.a
        ${LIBPSL_INSTALL_DIR}/lib/libpsl.a)
else()
    target_link_libraries(main PRIVATE CURL::libcurl)
endif()
if (UNIX)
    target_link_libraries(main PRIVATE pthread dl)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(main PRIVATE ws2_32 crypt32 secur32 user32 advapi32 wldap32 bcrypt)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(main "-framework IOKit")
    target_link_libraries(main "-framework Cocoa")
    target_link_libraries(main "-framework OpenGL")
endif()
