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
set(LIBXLSXWRITER_VERSION 1.2.2)
set(ZLIB_VERSION 1.3.1)
set(LIBZIP_VERSION 1.11.4)
set(UTF8CPP_VERSION 4.0.6)

message(STATUS "Downloading required libraries")

# raylib
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
    set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(MBEDTLS_CONFIG_FILE ${CMAKE_SOURCE_DIR}/include/my_mbedtls_config.h CACHE FILEPATH "" FORCE)
    FetchContent_Declare(
        mbedtls
        URL https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v${MBEDTLS_VERSION}.zip
    )
    FetchContent_MakeAvailable(mbedtls)
    set(MBEDTLS_INCLUDE_DIR ${mbedtls_SOURCE_DIR}/include CACHE PATH "" FORCE)
    set(MBEDTLS_LIBRARY ${mbedtls_BINARY_DIR}/library/libmbedtls.a CACHE FILEPATH "" FORCE)
    set(MBEDX509_LIBRARY ${mbedtls_BINARY_DIR}/library/libmbedx509.a CACHE FILEPATH "" FORCE)
    set(MBEDCRYPTO_LIBRARY ${mbedtls_BINARY_DIR}/library/libmbedcrypto.a CACHE FILEPATH "" FORCE)

    # libpsl
    set(LIBPSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/libpsl-install)
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
    else()
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

    set(LIBCURL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/libcurl-install)
    ExternalProject_Add(
        libcurl
        URL https://github.com/curl/curl/archive/curl-${LIBCURL_VERSION}.zip
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libcurl
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${LIBCURL_INSTALL_DIR}
            -DCURL_USE_MBEDTLS=ON
            -DMBEDTLS_INCLUDE_DIR=${mbedtls_SOURCE_DIR}/include
            -DMBEDTLS_LIBRARY=${mbedtls_BINARY_DIR}/library/libmbedtls.a
            -DMBEDX509_LIBRARY=${mbedtls_BINARY_DIR}/library/libmbedx509.a
            -DMBEDCRYPTO_LIBRARY=${mbedtls_BINARY_DIR}/library/libmbedcrypto.a
            -DCURL_USE_LIBPSL=ON
            -DLIBPSL_INCLUDE_DIR=${LIBPSL_INSTALL_DIR}/include
            -DLIBPSL_LIBRARY=${LIBPSL_INSTALL_DIR}/lib/libpsl.a
            -DCURL_STATICLIB=ON
            -DBUILD_SHARED_LIBS=OFF
            -DBUILD_CURL_EXE=OFF
        DEPENDS mbedtls libpsl
    )
endif()

# zlib
FetchContent_Declare(
    zlib
    URL https://github.com/madler/zlib/archive/v${ZLIB_VERSION}.zip
)
FetchContent_MakeAvailable(zlib)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE PATH "" FORCE)
if (WIN32)
    set(ZLIB_LIBRARY ${zlib_BINARY_DIR}/libzlibstatic.a CACHE FILEPATH "" FORCE)
else()
    set(ZLIB_LIBRARY ${zlib_BINARY_DIR}/libz.a CACHE FILEPATH "" FORCE)
endif()

# libxlsxwriter
FetchContent_Declare(
    xlsxwriter
    URL https://github.com/jmcnamara/libxlsxwriter/archive/v${LIBXLSXWRITER_VERSION}.zip
)
FetchContent_MakeAvailable(xlsxwriter)

# libzip
FetchContent_Declare(
    libzip
    URL https://github.com/nih-at/libzip/archive/v${LIBZIP_VERSION}.zip
)
FetchContent_MakeAvailable(libzip)

# utf8cpp
FetchContent_Declare(
    utf8cpp
    URL https://github.com/nemtrif/utfcpp/archive/v${UTF8CPP_VERSION}.zip
)
FetchContent_MakeAvailable(utf8cpp)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib imgui rlimgui zlib xlsxwriter zip utf8cpp)
if (NOT TARGET CURL::libcurl)
    add_dependencies(${PROJECT_NAME} libpsl libcurl)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${LIBCURL_INSTALL_DIR}/include
        ${LIBPSL_INSTALL_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC
        ${LIBCURL_INSTALL_DIR}/lib/libcurl.a
        mbedtls
        ${LIBPSL_INSTALL_DIR}/lib/libpsl.a
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
endif()
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32 secur32 user32 advapi32 wldap32 bcrypt)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
