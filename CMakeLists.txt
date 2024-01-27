cmake_minimum_required(VERSION 3.25)

#set your project name here. this will be used throughout the build system
project(rempack)

set(CMAKE_CXX_STANDARD 17)

#sanity checks for local toolchain setup

#to enable local emulation, install resim (link here)
#if resim is not available in the PATH environment variable, you may specify it here
#set(RESIM_ALTERNATE_PATH "~/.local/bin/")

find_program(RESIM_PATH resim
        "$ENV{PATH}" "${RESIM_ALTERNATE_PATH}")

if(NOT RESIM_PATH)
    message(WARNING "Resim is not found in any search path. Local emulation disabled!")
endif ()

#docker is required to build native images
find_program(DOCKER_PATH docker
            "$ENV{PATH}" REQUIRED)
if(NOT DOCKER_PATH)
    message(WARNING "Docker is not available! You won't be able to deploy to real hardware!")
endif ()

#user@host same as an SSH command
#host string should be an IP address or a hostname, if configured
set(REMARKABLE_HOST "root@remarkable")

#fetch rmkit.h from the build server
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/rmkit)

set(RMKIT_DIR "${CMAKE_BINARY_DIR}/include/rmkit")
set(HASH_FILE "${RMKIT_DIR}/sha256sum.txt")
set(RMKIT_FILE "${RMKIT_DIR}/rmkit.h")

file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/rmkit.h "${RMKIT_FILE}")
#verify downloaded file against posted hashes
file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/sha256sum.txt "${HASH_FILE}")
file(READ "${HASH_FILE}" HASH_LINES)
#extract the hash from the line ending with rmkit.h
string(REGEX MATCH "([A-Fa-f0-9]+)[\\t\\ ]+[a-zA-Z0-9/\\]+rmkit\\.h" MATCHERS "${HASH_LINES}")
set(HASH_VALUE "${CMAKE_MATCH_1}")
#check the hashes
file(READ "${RMKIT_FILE}" RMKIT_CONTENT)
string(SHA256 RMKIT_CONTENT_HASH "${RMKIT_CONTENT}")

if(NOT "${RMKIT_CONTENT_HASH}" STREQUAL "${HASH_VALUE}")
    message(FATAL_ERROR "Hash verification failed for rmkit.h! Claim: ${HASH_VALUE} Computed: ${RMKIT_CONTENT_HASH}")
else()
    message("Hash verification successful for rmkit.h!")
endif()

include_directories("${RMKIT_DIR}")
#TODO: font embed include directory
include_directories("${CMAKE_SOURCE_DIR}/include")
add_executable(${PROJECT_NAME}-dev main.cpp stb.x86.o)
target_compile_options(${PROJECT_NAME}-dev PUBLIC -O0 -g)#-D DEV=1 -D REMARKABLE_IMPLEMENTATION)

#if(RESIM_PATH)
    message(NOTICE "Resim found at ${RESIM_PATH}. Setting up target...")
    add_custom_target(Simulate
            COMMAND resim ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-dev
            VERBATIM)
#else()
    message(NOTICE "Resim path not found, Simulate target disabled!")
#endif()

add_custom_target(build_native
        COMMAND docker run -i -v /home/brant/git/rempack:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 cmake . -B artefacts
        VERBATIM)

#TODO: we can probably make this an external .sh script?
add_custom_target(Install
        COMMAND scp ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME} ${REMARKABLE_HOST}:/root/${PROJECT_NAME}
        VERBATIM)
add_dependencies(Install build_native)
