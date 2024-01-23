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

include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME} main.cpp)
#target_compile_options(${PROJECT_NAME}-dev PUBLIC -O0 -g)#-D DEV=1 -D REMARKABLE_IMPLEMENTATION)
target_compile_options(${PROJECT_NAME} PUBLIC
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        RMKIT_IMPLEMENTATION
        STB_IMAGE_IMPLEMENTATION
        STB_IMAGE_RESIZE_IMPLEMENTATION
        STB_IMAGE_WRITE_IMPLEMENTATION
        STB_TRUETYPE_IMPLEMENTATION
        $<$<CONFIG:Release>:REMARKABLE>
        )

#if(RESIM_PATH)
    message(NOTICE "Resim found at ${RESIM_PATH}. Setting up target...")
    add_custom_target(Simulate
            COMMAND resim ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-dev
            VERBATIM)
    add_dependencies(Simulate ${PROJECT_NAME})
#else()
    message(NOTICE "Resim path not found, Simulate target disabled!")
#endif()

add_custom_target(Install
        COMMAND scp ${CMAKE_SOURCE_DIR}/artefacts/${PROJECT_NAME} ${REMARKABLE_HOST}:/root/${PROJECT_NAME}
        VERBATIM)
add_dependencies(Install ${PROJECT_NAME})
