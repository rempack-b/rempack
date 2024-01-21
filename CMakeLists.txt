cmake_minimum_required(VERSION 3.25)

#set your project name here. this will be used throughout the build system
project(rempack)

set(CMAKE_CXX_STANDARD 17)

#sanity checks for local toolchain setup

#to enable local emulation, install resim ()
#if resim is not available in the PATH environment variable, you may specify it here
#set(RESIM_ALTERNATE_PATH "/home/brant/.local/bin/")

find_program(RESIM_PATH resim
        "$ENV{PATH}" "${RESIM_ALTERNATE_PATH}")

if(NOT RESIM_PATH)
    message(WARNING "Resim is not found in any search path. Local emulation disabled!")
endif ()

#docker is required to build native images
find_program(DOCKER_PATH docker
            "$ENV{PATH}" REQUIRED)


#user@host same as an SSH command
#host string should be an IP address or a hostname, if configured
set(REMARKABLE_HOST "root@remarkable")

set(RMKIT_PATH "${CMAKE_SOURCE_DIR}/include/rmkit")

#TODO: try fetching rmkit.h from https://build.rmkit.dev/master/latest/rm/rmkit.h

add_executable(${PROJECT_NAME}-dev main.cpp)
#target_compile_options(${PROJECT_NAME}-dev -D DEV=1 -D REMARKABLE_IMPLEMENTATION)

if(RESIM_PATH)
    message(NOTICE "Resim found at ${RESIM_PATH}. Setting up target...")
    add_custom_target(Simulate
            COMMAND ${RESIM_PATH}resim ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME}
            VERBATIM)
    add_dependencies(Simulate build_native)
else()
    message(NOTICE "Resim path not found, Simulate target disabled!")
endif()

add_custom_target(build_native
        COMMAND docker run -i -v /home/brant/git/rempack:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 cmake . -B artefacts
        VERBATIM)

#TODO: we can probably make this an external .sh script?
add_custom_target(Install
        COMMAND scp ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME} ${REMARKABLE_HOST}:/root/${PROJECT_NAME}
        VERBATIM)
add_dependencies(Install build_native)

add_custom_target(rmkit_docker
        COMMAND cd ${RMKIT_PATH} && make docker
        VERBATIM)