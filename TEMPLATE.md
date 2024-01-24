# RMKIT CMAKE TEMPLATE

This repo contains a template project for the ReMarkable series of tablets targeting rmkit with CMake.

This was developed specifically for CLion on Linux, but it should generally work in other environments. This was also targeted at the ReMarkable 2, but it should work fine on an RM1, and will probably work on Kobo tablets. If you successfully use this template for other systems, let me know!

## How to use

If you don't already know the basics of CMake, I recommend you stick with the `make` system provided by rmkit. If you don't know how to install and run software on your tablet, please read through the [wiki](https://remarkable.guide/index.html) carefully, and [**read the warnings**](https://toltec-dev.org/#install-toltec).

At bare minimum, you need Docker installed. Windows users should see the [docker website](https://docs.docker.com/desktop/install/windows-install/). Linux users know what to do.

(This is currently untested on Windows. The deploy script may not work correctly under Windows, but it should be trivial to work around)

Edit `CMakeLists.txt`. You must set your project name at the top of the file, and optionally provide an IP address for your tablet. The CMake file actually does a lot of work for you, I encourage you to read it all, it's not long.

Build the project. Make sure the hello world project compiles in *both* Debug and Release modes. Generally, you'd do something like:
```shell
cd /path/to/project
cmake . -B debug_output_dir --preset Debug
cmake --build debug_output_dir
cmake --build debug_output_dir --target docker_release
```

You can open the app on your tablet with
```shell
REMARKABLE_HOST="192.168.1.100" cmake --build debug_output_dir --target run_app
```

Press ctrl-c in your terminal to close the app and go back to the remarkable home screen.

If the connection is interrupted, the tablet UI may freeze. You can ssh in and reboot, or hold down the power button until it reboots.

From here you're free to develop your app with all the utilities provided by rmkit. See the [rmkit github](https://github.com/rmkit-dev/rmkit) for documentation.

## Native cross-compilation

This template supports native cross-compilation with the `Release` preset. You'll need to install `arm-linux-gnueabihf-g++` or equivalent for your platform as well as the RM toolchain. Please see the [Remarkable guide](https://remarkable.guide/devel/toolchains.html) for more information.

Note: this is not recommended, but the option exists if you're so inclined. It's best to stick with the docker toolchains.

## CLion Setup

When you open the project directory, CLion should detect `CMakePresets.json`. It will prompt you to import the preset build profiles. Unless you are set up for native cross-compiling, leave Release disabled and enable Debug.

You can optionally set up Docker integration with CLion.

First, open a terminal and 
```shell
docker pull ghcr.io/toltec-dev/toolchain:latest
```

Once that finishes, go back to CLion, go to `Settings -> Build, Execution, Deployment -> Toolchains`. Click `+` to add a new toolchain, select `Docker`, then under `Image` select the toolchain image you just pulled. You can change the name if you like.

Go to `CMake` in the settings panel, make a *copy* of the Release profile, name it Docker-Release or whatever you like. In that profile, change the `Toolchain` option to the docker toolchain you just created. Make sure that `Build Type` is set to `Release`

Now, when building in Debug mode, the system builds x86 binaries locally. In Release mode, it uses Docker behind the scenes to run the Release preset against the full set of RM toolchains.

## Next Steps

You'll probably want to publish your package in the [toltec](https://toltec-dev.org/) repositories. See their [contribution guide](https://github.com/toltec-dev/toltec/blob/stable/docs/contributing.md) for details.

Also check out the [ReMarkable dev discord](https://discord.gg/ATqQGfu)
