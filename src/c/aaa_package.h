#ifndef ICONVG_INCLUDE_GUARD
#define ICONVG_INCLUDE_GUARD

// Copyright 2021 The IconVG Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// ----------------

// IconVG ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// To use that single file as a "foo.c"-like implementation, instead of a
// "foo.h"-like header, #define ICONVG_IMPLEMENTATION before #include'ing or
// compiling it.

// ---------------- Version

// This section deals with library versions (also known as API versions), which
// are different from file format versions (FFVs). For example, library
// versions 3.0.1 and 4.2.0 could have incompatible API but still speak the
// same file format.

// ICONVG_LIBRARY_VERSION is major.minor.patch, as per https://semver.org/, as
// a uint64_t. The major number is the high 32 bits. The minor number is the
// middle 16 bits. The patch number is the low 16 bits. The pre-release label
// and build metadata are part of the string representation (such as
// "1.2.3-beta+456.20181231") but not the uint64_t representation.
//
// ICONVG_LIBRARY_VERSION_PRE_RELEASE_LABEL (such as "", "beta" or "rc.1")
// being non-empty denotes a developer preview, not a release version, and has
// no backwards or forwards compatibility guarantees.
//
// ICONVG_LIBRARY_VERSION_BUILD_METADATA_XXX, if non-zero, are the number of
// commits and the last commit date in the repository used to build this
// library. Within each major.minor branch, the commit count should increase
// monotonically.
//
// Some code generation programs can override ICONVG_LIBRARY_VERSION.
#define ICONVG_LIBRARY_VERSION 0
#define ICONVG_LIBRARY_VERSION_MAJOR 0
#define ICONVG_LIBRARY_VERSION_MINOR 0
#define ICONVG_LIBRARY_VERSION_PATCH 0
#define ICONVG_LIBRARY_VERSION_PRE_RELEASE_LABEL "unsupported.snapshot"
#define ICONVG_LIBRARY_VERSION_BUILD_METADATA_COMMIT_COUNT 0
#define ICONVG_LIBRARY_VERSION_BUILD_METADATA_COMMIT_DATE 0
#define ICONVG_LIBRARY_VERSION_STRING "0.0.0+0.00000000"

#include "./aaa_public.h"
#ifdef ICONVG_IMPLEMENTATION
#include "./aaa_private.h"
#include "./colors.c"
#include "./debug.c"
#include "./decoder.c"
#include "./errors.c"
#include "./paint.c"
#include "./rectangle.c"
#endif  // ICONVG_IMPLEMENTATION

#endif  // ICONVG_INCLUDE_GUARD
