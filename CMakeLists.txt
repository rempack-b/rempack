cmake_minimum_required(VERSION 3.25)

#set your project name here. this will be used throughout the build system
project(rempack)

set(CMAKE_CXX_STANDARD 17)

#user@host same as an SSH command
#host string should be an IP address or a hostname, if configured
set(REMARKABLE_HOST "root@remarkable")

#to enable local emulation, install resim () and set the path here
#do not include 'resim' at the end of the path, only the directory (with trailing slash)
#may be omitted if the resim binary is in the PATH variable for this environment
set(RESIM_PATH "/home/brant/.local/bin/")

#TODO: try fetching rmkit.h from https://build.rmkit.dev/master/latest/rm/rmkit.h

add_executable(${PROJECT_NAME}-dev main.cpp
        ${CMAKE_SOURCE_DIR}/rmkit/src/build/stb.x86.o)
target_compile_options(${PROJECT_NAME}-dev -D"DEV=1" -D"REMARKABLE_IMPLEMENTATION")


#g++ -c ../vendor/stb/stb.cpp -o ../build/stb.x86.o -fPIC -Os
add_library(stb.x86.o ${CMAKE_SOURCE_DIR}/rmkit/src/vendor/stb/stb.cpp)
target_compile_options(stb.x86.o PRIVATE -fPIC -Os -g)

add_dependencies(${PROJECT_NAME}-dev stb.x86.o)

#TODO: this PATH match will never work, figure out how to probe for the executable more directly
if(RESIM_PATH OR $ENV{PATH} MATCHES "resim")
    message(NOTICE "Resim path looks good")
    add_custom_target(Simulate
            COMMAND ${RESIM_PATH}resim ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME}
            VERBATIM)
    add_dependencies(Simulate build_native)
else()
    message(NOTICE "Resim not found in PATH, Simulate target disabled!")
endif()

#TODO: find a way to verify that docker is installed and available
#docker must be installed to build for the actual hardware
add_custom_target(build_native
        COMMAND docker run -i -v /home/brant/git/rempack:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 cmake . -B artefacts
        VERBATIM)

add_custom_target(Install
        COMMAND scp ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME} ${REMARKABLE_HOST}:/root/${PROJECT_NAME}
        VERBATIM)
add_dependencies(Install build_native)
