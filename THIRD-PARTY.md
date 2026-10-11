# Third-Party Licenses

This file aggregates the third-party licenses and copyright notices that apply
to the **McNopper/OpenVG_Library** project.

The library's own code (the EGL glue in `src/` and the build environment) is
distributed under the **MIT License** - see [`LICENSE`](LICENSE).

## Khronos provenance: OpenVG 1.1 Reference Implementation

**The implementation in `RI/` derives from the OpenVG 1.1 Reference
Implementation by The Khronos Group.** Its functionality is preserved;
outdated or buggy code is fixed in this repository (the changes are listed
below). The upstream license terms apply to those sources unchanged:

- **Library:** OpenVG 1.1 Reference Implementation (the "Materials")
- **Upstream:** https://www.khronos.org/openvg/
- **Copyright:** Copyright (c) 2007 The Khronos Group Inc.
- **License:** the Khronos permissive license shipped with the reference
  implementation (full text below and in [`RI/license.txt`](RI/license.txt))

> OpenVG 1.1 Reference Implementation
> -----------------------------------
>
> Copyright (c) 2007 The Khronos Group Inc.
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and /or associated documentation files (the "Materials "),
> to deal in the Materials without restriction, including without limitation
> the rights to use, copy, modify, merge, publish, distribute, sublicense,
> and/or sell copies of the Materials, and to permit persons to whom the
> Materials are furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of the Materials.
>
> THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS IN
> THE MATERIALS.

The canonical copy of the upstream license, including the notes on the tiger
sample data of the reference implementation (extracted from `tiger.eps` of
**GPL Ghostscript** - that data is not part of this library, but the note
ships with the reference implementation), is kept in [`RI/license.txt`](RI/license.txt)
together with [`RI/readme.txt`](RI/readme.txt).

### What was stripped from `RI/`

The upstream sources ship their own mini-EGL and OS layers (`riMiniEGL.cpp`,
the `riEGLOS.cpp` platform variants and the bundled `include/EGL/egl.h`). They
are **not part of this repository** - that role is played by
[McNopper/EGL](https://github.com/McNopper/EGL) (MIT), reached through the
provider seam in [`src/eglvg_glue.cpp`](src/eglvg_glue.cpp).

### Modifications to `RI/`

The OpenVG 1.1 functionality is preserved; outdated or buggy code is fixed.
So far the changes are:

- [`RI/include/VG/openvg.h`](RI/include/VG/openvg.h): `VGHandle` is
  pointer-sized (`uintptr_t`) instead of the 32-bit `VGuint` - the
  implementation casts object pointers to handles, which loses precision on
  64-bit platforms (it did not build for x64).
- [`RI/src/riDefs.h`](RI/src/riDefs.h): the MSVC6 for-scope workaround
  (`#define for if(getFalse()); else for`) is removed. Modern compilers scope
  for-init variables correctly, and the macro broke every standard header
  included after it.
- [`RI/src/riApi.cpp`](RI/src/riApi.cpp): the unused `egl.h` include is
  removed. The EGL seam only uses `void*` and `bool` declarations, and the
  platform headers a public EGL header pulls in collided with the
  implementation's own types.

## Dependency: McNopper/EGL

- **Library:** McNopper/EGL (portable EGL 1.5 with HDR support)
- **Upstream:** https://github.com/McNopper/EGL
- **License:** MIT
- **Copyright:** Copyright (c) since 2014 Norbert Nopper

EGL's OpenVG seam (`EGL_ENABLE_OPENVG`, off by default there) is switched on
by OpenVG_Library's build; its license and notices are maintained in the EGL
repository:

https://github.com/McNopper/EGL/blob/main/THIRD-PARTY.md
