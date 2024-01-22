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

There are also some environment variables that need to be defined. One is `RMKIT_IMPLEMENTATION`, the other is `font_embed.h` (see below). Need to look into what the hell that REMARKABLE_IMPLEMENTATION flag is doing.

Update: it doesn't seem to be anything significant. The flag just disables a bunch of what appears to be boilerplate. I really don't get why it's there? Maybe this is an exclusive or to `REMARKABLE`, which flags for a native build. Anyway, it seems to be required at all times, so we'll go with that.

We also need to look at the asset builder. We'll need it eventually.

# Docker
Basically, just run `docker run -i -v /path/to/project/root:/mnt/project --pull missing -w /mnt/project ghcr.io/toltec-dev/toolchain:v3.1 <some command>`
This mounts your project to `/mnt/project` inside the container, then you can just work out of there. `-w` is for `w`orking directory. 

I don't recommend running `make` or `cmake` without specifying a new output folder. If you just blindly make in the root, you'll clobber a bunch of file permissions from the docker user's UID. Or something like that anyway. Just set your output directory to something like `/mnt/project/docker-build` and it'll be fine. 
 
The Toltec images are from https://github.com/toltec-dev/toolchain

tl;dr there are multiple images with different dependencies preloaded. We're mainly concerned with `toolchain` and `python`. See the readme at the github repo.

rmkit needs the `python` flavor, and you have to `pip install okp` before building.

CMake was a CMistake.

At some point I need to look at CLion's docker support. It seems to just keep the docker container in the background and uses it instead of calling the native build tools? 

# Build server

rmkit has an automated build server that posts compiled binaries, including stb.arm.o

This isn't enough. We have the full rmkit.h file, but we're missing two things: font_embed.h and stb.x86.o

font_embed is a free font serialized to a byte array with some metadata. It's in the rmkit repo at `src/rmkit/`. You have to `#define FONT_EMBED_H "rel/path/to/font_embed.h"` somewhere to tell rmkit where to look. The path can be excluded if CMake includes the path for you.

The stb library is an image parser of some description. It's available as source, but rmkit optimized their build system by compiling stb once and sharing the binary with all projects. So we need the .o file to build against. This is available at `src/vendor/stb` and we can compile it with the normal tools to target x86.

So, we need to grab the release of `rmkit.h` from the server, and also clone the git repo so we can build `stb` and to also nab `font_embed.h`. Honestly might just copy or recreate the font and distribute it directly. Would save a step.