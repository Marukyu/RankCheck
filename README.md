# RankCheck

Automatic leaderboard lookup tool for Awesomenauts.

**[Download the latest release here!](https://rankcheck.marukyu.de/)**

RankCheck can be run alongside Awesomenauts and detects when a match begins. Leaderboard information for all players in
the match is automatically requested from a web server and displayed in the RankCheck window. It replaces the process
of manually looking up players on external leaderboard websites. It also allows streamers to overlay player information
on their Awesomenauts streams.

## Features

* Automatic detection of online, custom and bot matches
* Leaderboard lookup (database is refreshed every 30 minutes)
* Nickname tracker (shows each player's most commonly used name below their current name, if they differ)
* Encounter counter (tracks how often a player was encountered on the same or opposing team)
* Rating gain/loss history tracker
* Country lookup
* Streamer friendly (can be used as a color-keyed overlay, player info can be set to disappear after some time)
* Player card layout is completely customizable using a live-reloaded config file

## Screenshot

![RankCheck v1.4](https://i.imgur.com/ZCttB1v.png)

## Installation

You can download the latest release from the [RankCheck website](https://rankcheck.marukyu.de/).

### Windows

Extract the Windows release archive to your desired installation folder and open `RankCheck.exe`.

If any errors messages show up when you start RankCheck, please report the bug by opening an issue on RankCheck's issue
tracker on GitHub.

### Linux

A recent version (GLIBCXX >= 3.4.21) of the GNU C++ runtime library (libstdc++6) is required.

Extract the Linux release archive to your desired installation path and run `./RankCheck` in the installation path.

Only 64-bit Linux binaries are provided. To use RankCheck on 32-bit Linux, a build from source is required.

### Building from source

Clone this repository using Git and use [CMake](https://cmake.org) to build RankCheck from source.

```sh
git clone https://github.com/Marukyu/RankCheck.git
cd RankCheck
git pull --recurse-submodules
mkdir build
cd build
cmake ..
make
```

Library and header files for [SFML 2.3 or newer](https://sfml-dev.org) and [Poco Foundation 1.9 or
newer](https://pocoproject.org/), as well as their respective dependencies, must be installed.

A C++11 compliant compiler such as GCC 5 or newer is required to build RankCheck. Visual Studio has not been tested.

A copy of [`rankcheck.dat`](https://rankcheck.marukyu.de/files/rankcheck.dat) is required for RankCheck to run. The
file must be placed in the RankCheck working directory. For license reasons, this file or its contents are not included 
in the repository. When using the `Debug` build type, or when the macro `WOS_DEBUG` is defined, the RankCheck menu
contains options to extract and repack `rankcheck.dat`.

## Contributors

* [Marukyu](https://github.com/Marukyu) (main development of RankCheck)
* [Forty-Bot](https://github.com/Forty-Bot) (complete build system overhaul, data storage reworks, bugfixes,
[AUR package](https://aur.archlinux.org/packages/rankcheck/))
* [Blatoy](https://github.com/Blatoy) (username back-end)

## Dependencies

* [SFML](https://sfml-dev.org)
* [Poco](https://pocoproject.org/)
* [zlib](https://www.zlib.net/)
* [PugiXML](https://pugixml.org/) (included)
* [mingw-std-threads](https://github.com/meganz/mingw-std-threads) (included)
* [RapidJSON](http://rapidjson.org/) (included)
* [Tiny File Dialogs](https://sourceforge.net/projects/tinyfiledialogs/) (included)

## License (MIT License)

Copyright (c) 2018 Marukyu

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the 
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
