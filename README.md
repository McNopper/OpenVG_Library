OpenVG_Library
==============

[![Build](https://github.com/McNopper/OpenVG_Library/actions/workflows/build.yml/badge.svg)](https://github.com/McNopper/OpenVG_Library/actions/workflows/build.yml)

OpenVG 1.1 for [McNopper/EGL](https://github.com/McNopper/EGL): a software
OpenVG implementation (the `vg*` API) whose platform side is played by
McNopper/EGL through a thin provider glue.

The implementation derives from the OpenVG 1.1 Reference Implementation by
The Khronos Group (https://www.khronos.org/openvg/). Its functionality is
preserved; outdated or buggy code is fixed - for example the 32-bit handle
typedef (it did not build for x64) and an MSVC6-era `for` macro that broke
modern toolchains. Provenance and license are documented in `THIRD-PARTY.md`.

Use it on platforms that do not provide their own EGL/OpenVG pair - desktop
Windows and Linux. On platforms that do provide both (for example embedded
GPUs such as the Vivante GPU of the STM32MP1), link the platform's own
libraries instead and leave this library out - that is what the
[McNopper/OpenVG](https://github.com/McNopper/OpenVG) examples do.

Building
--------

        cmake -S . -B build
        cmake --build build

This produces the `OpenVG` static library. Requirements:

- **McNopper/EGL** - its OpenVG seam (`EGL_ENABLE_OPENVG`) is switched on by
  this library's build. A sibling `../EGL` checkout is preferred; otherwise the
  `v1.0.14` release is fetched.
- A C++14 compiler (the reference implementation is MSVC6-era C++).

Usage
-----

Link the `OpenVG` target (it pulls in `EGL`) and include `<VG/openvg.h>`:

        target_link_libraries(my_app OpenVG)

The provider registers itself with EGL at startup, so there is no
initialization code to write. EGL hands the implementation a `VG_sARGB_8888`
framebuffer (see EGL's `src/eglvg.h`) and presents it on `eglSwapBuffers`.

Together with [GLUS](https://github.com/McNopper/GLUS) (`OpenGL=VG` flavor)
this is the full stack behind the McNopper/OpenVG examples.

License
-------

The glue and build environment are MIT (see `LICENSE`). The RI-derived sources
are (c) 2007 The Khronos Group under their own permissive license - see
`THIRD-PARTY.md` and `RI/license.txt`, and
https://www.khronos.org/openvg/.
