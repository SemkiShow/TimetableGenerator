cmake_minimum_required(VERSION 3.14)
project(TimetableGenerator)

if (UNIX)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland support")
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support")
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Turn off all warning-related errors when compiling third-party code
add_compile_options(-Wno-error)

# raylib
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/raylib ${CMAKE_BINARY_DIR}/_deps/raylib-build)

# Imgui
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

# Curl
find_package(CURL QUIET)

if (NOT TARGET CURL::libcurl)
    # MbedTLS
    set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(MBEDTLS_CONFIG_FILE ${CMAKE_SOURCE_DIR}/include/my_mbedtls_config.h CACHE FILEPATH "" FORCE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/mbedtls ${CMAKE_BINARY_DIR}/_deps/mbedtls-build)
    set(MBEDTLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/mbedtls/include CACHE PATH "" FORCE)
    set(MBEDTLS_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedtls.a CACHE FILEPATH "" FORCE)
    set(MBEDX509_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedx509.a CACHE FILEPATH "" FORCE)
    set(MBEDCRYPTO_LIBRARY ${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedcrypto.a CACHE FILEPATH "" FORCE)

    # libpsl
    set(LIBPSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/libpsl-install)
    file(COPY ${CMAKE_SOURCE_DIR}/thirdparty/libpsl DESTINATION ${CMAKE_BINARY_DIR}/_deps)
    ExternalProject_Add(
        libpsl
        SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/libpsl
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libpsl
        CONFIGURE_COMMAND curl -o list/public_suffix_list.dat https://publicsuffix.org/list/public_suffix_list.dat &&
            ./autogen.sh && ./configure --prefix=${LIBPSL_INSTALL_DIR} --disable-runtime --disable-shared --enable-static $<$<AND:$<BOOL:${CMAKE_CROSSCOMPILING}>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>>:--host=x86_64-w64-mingw32>
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
    )

    set(LIBCURL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/libcurl-install)
    ExternalProject_Add(
        libcurl
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/curl
        BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/libcurl-build
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libcurl
        CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${LIBCURL_INSTALL_DIR}
            -DCURL_USE_MBEDTLS=ON
            -DMBEDTLS_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/thirdparty/mbedtls/include
            -DMBEDTLS_LIBRARY=${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedtls.a
            -DMBEDX509_LIBRARY=${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedx509.a
            -DMBEDCRYPTO_LIBRARY=${CMAKE_BINARY_DIR}/_deps/mbedtls-build/library/libmbedcrypto.a
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
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/zlib ${CMAKE_BINARY_DIR}/_deps/zlib-build)
set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/zlib;${CMAKE_BINARY_DIR}/_deps/zlib-build" CACHE PATH "" FORCE)
if (WIN32)
    set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a CACHE FILEPATH "" FORCE)
else()
    set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/_deps/zlib-build/libz.a CACHE FILEPATH "" FORCE)
endif()

# libxlsxwriter
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libxlsxwriter ${CMAKE_BINARY_DIR}/_deps/libxlsxwriter-build)

# libzip
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libzip ${CMAKE_BINARY_DIR}/_deps/libzip-build)
add_dependencies(zip zlibstatic)

# utf8cpp
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/utfcpp ${CMAKE_BINARY_DIR}/_deps/utfcpp)

add_compile_options(-Wall -Wextra)
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib imgui rlimgui zlibstatic xlsxwriter zip utf8cpp)
if (NOT TARGET CURL::libcurl)
    add_dependencies(${PROJECT_NAME} libpsl libcurl)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${LIBCURL_INSTALL_DIR}/include
        ${LIBPSL_INSTALL_DIR}/include
    )
    target_link_directories(${PROJECT_NAME} PRIVATE ${LIBCURL_INSTALL_DIR}/lib ${LIBCURL_INSTALL_DIR}/lib64)
    target_link_directories(${PROJECT_NAME} PRIVATE ${LIBPSL_INSTALL_DIR}/lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE curl mbedtls psl)
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
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()
