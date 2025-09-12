# FFmpeg C++ CMake Template

A practical starter to build C++ applications with FFmpeg using CMake.

## Why this exists

Linking FFmpeg correctly (headers, libraries, and link order) can be tricky
across platforms and toolchains (I am saying this from experience). This
template shows a straightforward CMake setup and a tiny example so you can
focus on writing your media code, not your build system.

## Prerequisites

- nasm (install on Ubuntu with `sudo apt-get install nasm`)
- CMake
- a build system (like make or ninja)
- a c and c++ compiler

## Start

Run the following commands to build the example program and run it on the video file:

```bash
git clone https://github.com/agokule/ffmpeg-template.git --recurse-submodules
cd ffmpeg-template
cmake --preset=vcpkg
cmake --build build
./build/HelloWorld test.mp4
```

> Note: on Windows, vcpkg might try to build ffmpeg with a different compiler, if that is the case,
> you might need to add `-DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic` or `x64-mingw-static` or `x64-windows`
> to the `cmake --preset=vcpkg` command

If it compiled successfully, you should see the following message:

```
=== File Information ===
Filename: test.mp4
Format: QuickTime / MOV
Duration: 30 seconds
Bitrate: 2018246 bps
Number of streams: 1

=== Stream 0 ===
Type: Video
Codec: H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
Resolution: 1280x720
Pixel Format: yuv420p
Bitrate: 2015361 bps
Frame Rate: 25 fps
```


## Contributing

Issues and PRs to improve the starter are welcome, especially fixes for
platform quirks and better discovery of FFmpeg on different systems.


## Acknowledgments

- The FFmpeg project and community: https://ffmpeg.org
- Everyone who has shared examples and best practices for CMake + FFmpeg

---

Again, this repo exists purely to help people start developing with FFmpeg.
Feel free to copy it, tweak it, and build something awesome!

