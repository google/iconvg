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

// ----

// IconVG ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// To use that single file as a "foo.c"-like implementation, instead of a
// "foo.h"-like header, #define ICONVG_IMPLEMENTATION before #include'ing or
// compiling it.

#include "./aaa_public.h"
#ifdef ICONVG_IMPLEMENTATION
#include "./aaa_private.h"
#include "./arc.c"
#include "./broken.c"
#include "./cairo.c"
#include "./color.c"
#include "./debug.c"
#include "./decoder.c"
#include "./error.c"
#include "./matrix.c"
#include "./paint.c"
#include "./rectangle.c"
#include "./skia.c"
#endif  // ICONVG_IMPLEMENTATION

#endif  // ICONVG_INCLUDE_GUARD
