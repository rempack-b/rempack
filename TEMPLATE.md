# RMKIT CMAKE TEMPLATE

This repo contains a template project for the ReMarkable series of tablets targeting rmkit with CMake.

This was developed specifically for CLion on Linux, but it should generally work in other environments. This was also targeted at the ReMarkable 2, but it should work fine on an RM1, and will probably work on Kobo tablets. If you successfully use this template for other systems, let me know!

## How to use

If you don't already know the basics of CMake, I recommend you stick with the `make` system provided by rmkit. If you don't know how to install and run software on your tablet, please read through the [wiki]() carefully, and [read the warning]().

At bare minimum, you need docker installed. Windows users should see the [docker website](). Linux users know what to do.

Edit `CMakeLists.txt`. There are several options you should change, but the only required setting is the project name at the top of the file. The file is thoroughly documented and explains all the helpful things it does for you. I promise it's worth reading the entire thing.

Build the project. Make sure the hello world project compiles in *both* Debug and Release modes. Generally, you'd do something like:
```shell
cd /path/to/project
cmake . -B debug_output_dir --preset Debug
cmake --build debug_output_dir
cmake --build debug_output_dir --target docker_release
```

You can open the app on your tablet with
```shell
REMARKABLE_HOST="192.168.1.100" cmake --build debug_output_dir --target install_docker
```

Press ctrl-c in your terminal to close the app and go back to the remarkable home screen.

If the connection is interrupted, the tablet UI may freeze. You can ssh in and reboot, or hold down the power button until it reboots.

From here you're free to develop your app with all the utilities provided by rmkit. See rmkit's documentation [here]()

## Native cross-compilation

This template supports native cross-compilation with the `Release` preset. You'll need to install `arm-linux-gnueabihf-g++` or equivalent for your platform.

Note: this is not recommended, but the option exists if you're so inclined.

## CLion Setup

When you open the project directory, CLion should detect `CMakePresets.json`