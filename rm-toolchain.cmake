#CMake toolchain preset defining options for the remarkable tablet
#basically just defining the compiler we want and a few gcc flags

set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_STRIP arm-linux-gnueabihf-strip)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread -fdata-sections -ffunction-sections -Wl,--gc-sections")