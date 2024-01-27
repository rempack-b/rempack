#CMake toolchain preset defining options for the remarkable tablet

set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_STRIP arm-linux-gnueabihf-strip)
set(CMAKE_LINKER arm-linux-gnueabihf-ld)
set(CMAKE_SYSROOT /opt/x-tools/arm-remarkable-linux-gnueabihf/arm-remarkable-linux-gnueabihf/sysroot)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread -fdata-sections -ffunction-sections -Wl,--gc-sections")