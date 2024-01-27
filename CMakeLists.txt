cmake_minimum_required(VERSION 3.21)

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

#user@host same as an SSH command
#host string should be an IP address or a hostname, if configured
#you can also simply set this variable as an argument passed to CMake: -DREMARKABLE_HOST="root@192.168.1.102"
if(NOT REMARKABLE_HOST)
    set(REMARKABLE_HOST "root@remarkable")
endif()

set(RMKIT_DIR "${CMAKE_BINARY_DIR}/include/rmkit")
set(HASH_FILE "${RMKIT_DIR}/sha256sum.txt")
set(RMKIT_FILE "${RMKIT_DIR}/rmkit.h")

#fetch rmkit.h from the build server
if(NOT EXISTS ${RMKIT_FILE})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/rmkit)
    file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/rmkit.h "${RMKIT_FILE}")
    #verify downloaded file against posted hashes
    file(DOWNLOAD https://build.rmkit.dev/master/latest/rm/sha256sum.txt "${HASH_FILE}")
    file(READ "${HASH_FILE}" HASH_LINES)
    #extract the hash from the line ending with rmkit.h
    string(REGEX MATCH "([A-Fa-f0-9]+)[\\t\\ ]+[a-zA-Z0-9/\\]+rmkit\\.h" _ "${HASH_LINES}")
    set(HASH_VALUE "${CMAKE_MATCH_1}")
    #check the hashes
    file(READ "${RMKIT_FILE}" RMKIT_CONTENT)
    string(SHA256 RMKIT_CONTENT_HASH "${RMKIT_CONTENT}")
    file(REMOVE ${HASH_FILE})

    if(NOT "${RMKIT_CONTENT_HASH}" STREQUAL "${HASH_VALUE}")
        message(FATAL_ERROR "Hash verification failed for rmkit.h! Claim: ${HASH_VALUE} Computed: ${RMKIT_CONTENT_HASH}")
        file(REMOVE ${RKIT_FILE})
    else()
        message("Hash verification successful for rmkit.h!")
    endif()
endif()

include_directories(SYSTEM "${RMKIT_DIR}")
#for font_embed.h
include_directories("${CMAKE_SOURCE_DIR}/include")

#set(CMAKE_SYSTEM_NAME "Linux")
#set(CMAKE_SYSTEM_NAME "Arm")
#set(CMAKE_SYSTEM_NAME "Remarkable")
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
#set(STRIP_BIN arm-linux-gnueabihf-strip)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_options(${PROJECT_NAME}
        PUBLIC -pthread -lpthread -fdata-sections -ffunction-sections -Wl,--gc-sections --verbose
        #PUBLIC -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++
        PUBLIC $<$<CONFIG:Debug>:-O0>
        PUBLIC $<$<CONFIG:Release>:-O3>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        RMKIT_IMPLEMENTATION
        STB_IMAGE_IMPLEMENTATION
        STB_IMAGE_RESIZE_IMPLEMENTATION
        STB_IMAGE_WRITE_IMPLEMENTATION
        STB_TRUETYPE_IMPLEMENTATION
        REMARKABLE=1
        #$<$<CONFIG:Release>:REMARKABLE>
        #$<$<CONFIG:Debug>:FONT_EMBED_H="font_embed.h">
        )

if(RESIM_PATH)
#    message(NOTICE "Resim found at ${RESIM_PATH}. Setting up target...")
    add_custom_target(Simulate
            COMMAND ${RESIM_PATH} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
            VERBATIM)
    add_dependencies(Simulate ${PROJECT_NAME})
#else()
#    message(NOTICE "Resim path not found, Simulate target disabled!")
endif()

add_custom_target(Install
        COMMAND scp ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME} ${REMARKABLE_HOST}:/root/${PROJECT_NAME}
        VERBATIM)
add_dependencies(Install ${PROJECT_NAME})

add_custom_target(build_docker
        COMMAND docker run -i -v ${CMAKE_SOURCE_DIR}:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 sh stupid_build_script.sh
        VERBATIM)

add_custom_target(install_docker
        COMMAND sh run_remote.sh rempack
        VERBATIM)
add_dependencies(install_docker build_docker)