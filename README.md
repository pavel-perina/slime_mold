```
 ____  _ _                __  __       _     _ 
/ ___|| (_)_ __ ___   ___|  \/  | ___ | | __| |
\___ \| | | '_ ` _ \ / _ \ |\/| |/ _ \| |/ _` |
 ___) | | | | | | | |  __/ |  | | (_) | | (_| |
|____/|_|_|_| |_| |_|\___|_|  |_|\___/|_|\__,_|
```

This is basically AI generated slime mold simulator. I never used ImGui or SDL libraries, but it evolved this way.
I just randomly asked ChatGPT to create it, use SDL cause I wanted to try it for other project anyways, then it
was upgraded to SDL3 and ImGui was added.

Optimizations were done by me. 

More is in my [blog article](https://www.pavelp.cz/posts/eng-random-chatgpt-code/) with initial, more simple code.

![Screenshot](slime_mold.png)

## Build instructions (Windows, Conan)

```
# Clone this repo
git clone https://github.com/pavel-perina/slime_mold.git
# Go to directory
cd .\slime_mold\
# Update submodule (and perform initial fetch - this clones [ImGui](https://github.com/ocornut/imgui) repository)
git submodule update --init
# Install/build dependencies (SDL3)
.\conan_install.bat
```
* Open folder in Visual Studio 2022
  * CMake will run automatically
* Choose build type `conan-relwithdebinfo` or `conan-debug`
* Select startup item such as `Relwithdebinfo\slime_mold.exe`
* Build it (build all or just `slime_mold.exe`)
* Debug -> Start (or start without debugging)
* Enjoy :-)
