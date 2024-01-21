# Docs

Just some notes during my discovery of this system

Build system is built around the Toltec docker images. You can cross-compile natively, but this requires the CPY preprocessor, which I'm trying to avoid. Python makes me scared and confused, especially where it touches my C. Fortunately we can just brush all of that under the rug and only deal with gcc.

The docker environment is set up for native cross-compilation with all of the gross toolchain preinstalled.

General order of operations:
- Build rmkit with the docker image
- Link current project against rmkit output headers
- Build with docker image
  - we can produce x86 and native images for emulation and local debug
- Deploy to remote device or emulator

rmkit has makefile options to automate docker builds, but it deletes some files that I'd rather keep. It's easy enough to bypass that and take control of build more directly in the docker image. The big thing was the `stb.x86.o`. I haven't looked at exactly what that is doing, but it's important.

There are also some environment variables that need to be defined. One is #REMARKABLE_IMPLEMENTATION, the other is a path to some asset file, which can probably just point to a blank file. Need to look into what the hell that REMARKABLE_IMPLEMENTATION flag is doing.

We also need to look at the asset builder. We'll need it eventually.

# Docker
Basically, just run `docker run -i -v /path/to/project/root:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 <some command>`
This mounts your project to `/mnt/project` inside the container, then you can just work out of there. `-w` is for `w`orking directory. 

I don't recommend running `make` or `cmake` without specifying a new output folder. If you just blindly make in the root, you'll clobber a bunch of file permissions from the docker user's UID. Or something like that anyway. Just set your output directory to something like `/mnt/project/docker-build` and it'll be fine. 
 
The Toltec images are from https://github.com/toltec-dev/toolchain

tl;dr there are multiple images with different dependencies preloaded. We're mainly concerned with `toolchain` and `python`. See the readme at the github repo.

rmkit needs the `python` flavor, and you have to `pip install okp` before building.

CMake was a CMistake.

At some point I need to look at CLion's docker support. I'm not really sure what it's actually for?