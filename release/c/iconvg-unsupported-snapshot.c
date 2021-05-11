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

// -------------------------------- #include "./aaa_public.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

// ----

// Functions that return a "const char*" typically use that to denote success
// (returning NULL) or failure (returning non-NULL). On failure, that C string
// is a human-readable but non-localized error message. It can also be compared
// (by the == operator, not just by strcmp) to an iconvg_error_etc constant.
//
// bad_etc indicates a file format error. The source bytes are not IconVG.
//
// Other errors (invalid_etc, null_etc, unsupported_etc) are programming errors
// instead of file format errors.

extern const char iconvg_error_bad_color[];
extern const char iconvg_error_bad_coordinate[];
extern const char iconvg_error_bad_drawing_opcode[];
extern const char iconvg_error_bad_magic_identifier[];
extern const char iconvg_error_bad_metadata[];
extern const char iconvg_error_bad_metadata_id_order[];
extern const char iconvg_error_bad_metadata_suggested_palette[];
extern const char iconvg_error_bad_metadata_viewbox[];
extern const char iconvg_error_bad_number[];
extern const char iconvg_error_bad_path_unfinished[];
extern const char iconvg_error_bad_styling_opcode[];

extern const char iconvg_error_invalid_backend_not_enabled[];
extern const char iconvg_error_invalid_constructor_argument[];
extern const char iconvg_error_null_vtable[];
extern const char iconvg_error_unsupported_vtable[];

// ----

// iconvg_rectangle_f32 is an axis-aligned rectangle with float32 co-ordinates.
//
// It is valid for a minimum co-ordinate to be greater than or equal to the
// corresponding maximum, or for any co-ordinate to be NaN, in which case the
// rectangle is empty. There are multiple ways to represent an empty rectangle
// but the canonical representation has all fields set to positive zero.
typedef struct iconvg_rectangle_f32_struct {
  float min_x;
  float min_y;
  float max_x;
  float max_y;
} iconvg_rectangle_f32;

// ----

#define ICONVG_RGBA_INDEX___RED 0
#define ICONVG_RGBA_INDEX__BLUE 1
#define ICONVG_RGBA_INDEX_GREEN 2
#define ICONVG_RGBA_INDEX_ALPHA 3

// iconvg_nonpremul_color is an non-alpha-premultiplied RGBA color. Non-alpha-
// premultiplication means that {0x00, 0xFF, 0x00, 0xC0} represents a
// 75%-opaque, fully saturated green.
typedef struct iconvg_nonpremul_color_struct {
  uint8_t rgba[4];
} iconvg_nonpremul_color;

// iconvg_premul_color is an alpha-premultiplied RGBA color. Alpha-
// premultiplication means that {0x00, 0xC0, 0x00, 0xC0} represents a
// 75%-opaque, fully saturated green.
typedef struct iconvg_premul_color_struct {
  uint8_t rgba[4];
} iconvg_premul_color;

// iconvg_palette is a list of 64 alpha-premultiplied RGBA colors.
typedef struct iconvg_palette_struct {
  iconvg_premul_color colors[64];
} iconvg_palette;

// ----

struct iconvg_paint_struct;

// iconvg_paint is an opaque data structure passed to iconvg_canvas_vtable's
// paint method.
typedef struct iconvg_paint_struct iconvg_paint;

// ----

// iconvg_decode_options holds the optional arguments to iconvg_decode.
typedef struct iconvg_decode_options_struct {
  // sizeof__iconvg_decode_options should be set to the sizeof this data
  // structure. An explicit value allows different library versions to work
  // together when dynamically linked. Newer library versions will only append
  // fields, never remove or re-arrange old fields. If the caller has a newer
  // library version, newer fields will be ignored. If the callee has a newer
  // library version, missing fields will assume the implicit default values.
  size_t sizeof__iconvg_decode_options;

  // palette, if non-NULL, is the custom palette used for rendering. If NULL,
  // the IconVG file's suggested palette is used instead.
  iconvg_palette* palette;
} iconvg_decode_options;

// iconvg_make_decode_options_ffv1 returns an iconvg_decode_options suitable
// for FFV (file format version) 1.
static inline iconvg_decode_options  //
iconvg_make_decode_options_ffv1(iconvg_palette* palette) {
  iconvg_decode_options o = {0};
  o.sizeof__iconvg_decode_options = sizeof(iconvg_decode_options);
  o.palette = palette;
  return o;
}

// ----

// iconvg_canvas is conceptually a 'virtual super-class' with e.g. Cairo-backed
// or Skia-backed 'sub-classes'.
//
// This is like C++'s class mechanism, simplified (no multiple inheritance, all
// 'sub-classes' have the same sizeof), but implemented by explicit code
// instead of by the language. This library is implemented in C, not C++.
//
// Most users won't need to know about the details of the iconvg_canvas and
// iconvg_canvas_vtable types. Only that iconvg_make_etc_canvas creates a
// canvas and the iconvg_canvas__etc methods take a canvas as an argument.

struct iconvg_canvas_struct;

typedef struct iconvg_canvas_vtable_struct {
  size_t sizeof__iconvg_canvas_vtable;
  const char* (*begin_decode)(struct iconvg_canvas_struct* c);
  const char* (*end_decode)(struct iconvg_canvas_struct* c,
                            const char* err_msg,
                            size_t num_bytes_consumed,
                            size_t num_bytes_remaining);
  const char* (*begin_drawing)(struct iconvg_canvas_struct* c);
  const char* (*end_drawing)(struct iconvg_canvas_struct* c,
                             const iconvg_paint* p);
  const char* (*begin_path)(struct iconvg_canvas_struct* c, float x0, float y0);
  const char* (*end_path)(struct iconvg_canvas_struct* c);
  const char* (*path_line_to)(struct iconvg_canvas_struct* c,
                              float x1,
                              float y1);
  const char* (*path_quad_to)(struct iconvg_canvas_struct* c,
                              float x1,
                              float y1,
                              float x2,
                              float y2);
  const char* (*path_cube_to)(struct iconvg_canvas_struct* c,
                              float x1,
                              float y1,
                              float x2,
                              float y2,
                              float x3,
                              float y3);
  const char* (*path_arc_to)(struct iconvg_canvas_struct* c,
                             float radius_x,
                             float radius_y,
                             float x_axis_rotation,
                             bool large_arc,
                             bool sweep,
                             float final_x,
                             float final_y);
  const char* (*on_metadata_viewbox)(struct iconvg_canvas_struct* c,
                                     iconvg_rectangle_f32 viewbox);
  const char* (*on_metadata_suggested_palette)(
      struct iconvg_canvas_struct* c,
      const iconvg_palette* suggested_palette);
} iconvg_canvas_vtable;

typedef struct iconvg_canvas_struct {
  // vtable defines what 'sub-class' we have.
  const iconvg_canvas_vtable* vtable;

  // context_etc semantics depend on the 'sub-class' and should be considered
  // private implementation details. For built-in 'sub-classes', as returned by
  // the library's iconvg_make_etc_canvas functions, users should not read or
  // write these fields directly and their semantics may change between minor
  // library releases.
  void* context_nonconst_ptr0;
  void* context_nonconst_ptr1;
  const void* context_const_ptr;
  size_t context_extra;
} iconvg_canvas;

// ----

#ifdef __cplusplus
extern "C" {
#endif

// iconvg_error_is_file_format_error returns whether err_msg is one of the
// built-in iconvg_error_bad_etc constants.
bool  //
iconvg_error_is_file_format_error(const char* err_msg);

// ----

// iconvg_make_broken_canvas returns an iconvg_canvas whose callbacks all do
// nothing other than return err_msg.
iconvg_canvas  //
iconvg_make_broken_canvas(const char* err_msg);

// iconvg_make_debug_canvas returns an iconvg_canvas that logs vtable calls to
// f before forwarding the call on to the wrapped iconvg_canvas. Log messages
// are prefixed by message_prefix.
//
// f may be NULL, in which case nothing is logged.
//
// message_prefix may be NULL, equivalent to an empty prefix.
//
// wrapped may be NULL, in which case the iconvg_canvas vtable calls always
// return success (a NULL error message) except that end_decode returns its
// (possibly non-NULL) err_msg argument unchanged.
//
// If any of the pointer-typed arguments are non-NULL then the caller of this
// function is responsible for ensuring that the pointers remain valid while
// the returned iconvg_canvas is in use.
iconvg_canvas  //
iconvg_make_debug_canvas(FILE* f,
                         const char* message_prefix,
                         iconvg_canvas* wrapped);

// ----

typedef struct _cairo cairo_t;

// iconvg_make_cairo_canvas returns an iconvg_canvas that is backed by the
// Cairo graphics library, if the ICONVG_CONFIG__ENABLE_CAIRO_BACKEND macro
// was defined when the IconVG library was built.
//
// If that macro was not defined then the returned value will be broken (with
// iconvg_error_invalid_backend_not_enabled).
//
// If cr is NULL then the returned value will be broken (with
// iconvg_error_invalid_constructor_argument).
iconvg_canvas  //
iconvg_make_cairo_canvas(cairo_t* cr);

// ----

// iconvg_decode decodes the src IconVG-formatted data, calling dst_canvas's
// callbacks (vtable functions) to paint the decoded vector graphic.
//
// The call sequence always begins with exactly one begin_decode call and ends
// with exactly one end_decode call. If src holds well-formed IconVG data and
// none of the callbacks returns an error then the err_msg argument to
// end_decode will be NULL. Otherwise, the call sequence stops as soon as a
// non-NULL error is encountered, whether a file format error or a callback
// error. This non-NULL error becomes the err_msg argument to end_decode and
// this function, iconvg_decode, returns whatever end_decode returns.
//
// options may be NULL, in which case default values will be used.
const char*  //
iconvg_decode(iconvg_canvas* dst_canvas,
              const uint8_t* src_ptr,
              size_t src_len,
              const iconvg_decode_options* options);

// iconvg_decode_viewbox sets *dst_viewbox to the ViewBox Metadata from the src
// IconVG-formatted data.
//
// An explicit ViewBox is optional in the IconVG file format. If not present in
// src, *dst_viewbox will be set to the default ViewBox: {-32, -32, +32, +32}.
//
// dst_viewbox may be NULL, in which case the function merely validates src's
// ViewBox.
const char*  //
iconvg_decode_viewbox(iconvg_rectangle_f32* dst_viewbox,
                      const uint8_t* src_ptr,
                      size_t src_len);

// ----

// iconvg_paint__is_flat_color returns whether self is a flat color (as opposed
// to a gradient).
bool  //
iconvg_paint__is_flat_color(const iconvg_paint* self);

// iconvg_paint__flat_color_as_nonpremul_color returns self's color (as non-
// alpha-premultiplied), assuming that self is a flat color.
//
// If self is not a flat color than the result may be a non-sensical color.
iconvg_nonpremul_color  //
iconvg_paint__flat_color_as_nonpremul_color(const iconvg_paint* self);

// iconvg_paint__flat_color_as_premul_color returns self's color (as alpha-
// premultiplied), assuming that self is a flat color.
//
// If self is not a flat color than the result may be a non-sensical color.
iconvg_premul_color  //
iconvg_paint__flat_color_as_premul_color(const iconvg_paint* self);

// ----

// iconvg_rectangle_f32__width returns self's width.
float  //
iconvg_rectangle_f32__width(const iconvg_rectangle_f32* self);

// iconvg_rectangle_f32__height returns self's height.
float  //
iconvg_rectangle_f32__height(const iconvg_rectangle_f32* self);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef ICONVG_IMPLEMENTATION
// -------------------------------- #include "./aaa_private.h"

#define ICONVG_PRIVATE_TRY(err_msg)                   \
  do {                                                \
    const char* iconvg_private_try_err_msg = err_msg; \
    if (iconvg_private_try_err_msg) {                 \
      return iconvg_private_try_err_msg;              \
    }                                                 \
  } while (false)

// ----

extern const char iconvg_private_internal_error_unreachable[];

// ----

static inline uint16_t  //
iconvg_private_peek_u16le(const uint8_t* p) {
  return (uint16_t)(((uint16_t)(p[0]) << 0) | ((uint16_t)(p[1]) << 8));
}

static inline uint32_t  //
iconvg_private_peek_u32le(const uint8_t* p) {
  return ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

static inline void  //
iconvg_private_poke_u32le(uint8_t* p, uint32_t x) {
  p[0] = (uint8_t)(x >> 24);
  p[1] = (uint8_t)(x >> 16);
  p[2] = (uint8_t)(x >> 8);
  p[3] = (uint8_t)(x >> 0);
}

static inline float  //
iconvg_private_reinterpret_from_u32_to_f32(uint32_t u) {
  float f = 0;
  if (sizeof(uint32_t) == sizeof(float)) {
    memcpy(&f, &u, sizeof(uint32_t));
  }
  return f;
}

// ----

static inline size_t  //
iconvg_private_canvas_sizeof_vtable(iconvg_canvas* c) {
  if (c && c->vtable) {
    return c->vtable->sizeof__iconvg_canvas_vtable;
  }
  return 0;
}

// ----

static inline iconvg_rectangle_f32  //
iconvg_private_default_viewbox() {
  iconvg_rectangle_f32 r;
  r.min_x = -32.0f;
  r.min_y = -32.0f;
  r.max_x = +32.0f;
  r.max_y = +32.0f;
  return r;
}

// ----

typedef struct iconvg_private_decoder_struct {
  const uint8_t* ptr;
  size_t len;
} iconvg_private_decoder;

// ----

extern const uint8_t iconvg_private_one_byte_colors[512];
extern const iconvg_palette iconvg_private_default_palette;

static inline void  //
iconvg_private_set_one_byte_color(uint8_t* dst,
                                  const iconvg_palette* custom_palette,
                                  const iconvg_palette* creg,
                                  uint8_t u) {
  if (u < 0x80) {
    iconvg_private_poke_u32le(
        dst, iconvg_private_peek_u32le(
                 &iconvg_private_one_byte_colors[4 * ((size_t)u)]));
  } else if (u < 0xC0) {
    iconvg_private_poke_u32le(
        dst,
        iconvg_private_peek_u32le(&custom_palette->colors[u & 0x3F].rgba[0]));
  } else {
    iconvg_private_poke_u32le(
        dst, iconvg_private_peek_u32le(&creg->colors[u & 0x3F].rgba[0]));
  }
}

// ----

static inline int  //
iconvg_private_last_color_that_isnt_opaque_black(
    const iconvg_palette* palette) {
  int i = 63;
  for (; i >= 0; i--) {
    if (iconvg_private_peek_u32le(&palette->colors[i].rgba[0]) != 0xFF000000u) {
      break;
    }
  }
  return i;
}

// ----

struct iconvg_paint_struct {
  uint8_t paint_rgba[4];
  iconvg_palette custom_palette;
  iconvg_palette creg;
  float nreg[64];
};

// -------------------------------- #include "./broken.c"

static const char*  //
iconvg_private_broken_canvas__begin_decode(iconvg_canvas* c) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_decode(iconvg_canvas* c,
                                         const char* err_msg,
                                         size_t num_bytes_consumed,
                                         size_t num_bytes_remaining) {
  return err_msg ? err_msg : ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__begin_drawing(iconvg_canvas* c) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_drawing(iconvg_canvas* c,
                                          const iconvg_paint* p) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_path(iconvg_canvas* c) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_line_to(iconvg_canvas* c,
                                           float x1,
                                           float y1) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_quad_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_cube_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2,
                                           float x3,
                                           float y3) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_arc_to(iconvg_canvas* c,
                                          float radius_x,
                                          float radius_y,
                                          float x_axis_rotation,
                                          bool large_arc,
                                          bool sweep,
                                          float final_x,
                                          float final_y) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_viewbox(
    iconvg_canvas* c,
    iconvg_rectangle_f32 viewbox) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return ((const char*)(c->context_const_ptr));
}

static const iconvg_canvas_vtable  //
    iconvg_private_broken_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_broken_canvas__begin_decode,
        &iconvg_private_broken_canvas__end_decode,
        &iconvg_private_broken_canvas__begin_drawing,
        &iconvg_private_broken_canvas__end_drawing,
        &iconvg_private_broken_canvas__begin_path,
        &iconvg_private_broken_canvas__end_path,
        &iconvg_private_broken_canvas__path_line_to,
        &iconvg_private_broken_canvas__path_quad_to,
        &iconvg_private_broken_canvas__path_cube_to,
        &iconvg_private_broken_canvas__path_arc_to,
        &iconvg_private_broken_canvas__on_metadata_viewbox,
        &iconvg_private_broken_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_broken_canvas(const char* err_msg) {
  iconvg_canvas c;
  c.vtable = &iconvg_private_broken_canvas_vtable;
  c.context_nonconst_ptr0 = NULL;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = err_msg;
  c.context_extra = 0;
  return c;
}

// -------------------------------- #include "./cairo.c"

#if !defined(ICONVG_CONFIG__ENABLE_CAIRO_BACKEND)

iconvg_canvas  //
iconvg_make_cairo_canvas(cairo_t* cr) {
  return iconvg_make_broken_canvas(iconvg_error_invalid_backend_not_enabled);
}

#else  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND

#include <cairo/cairo.h>

static const char*  //
iconvg_private_cairo_canvas__begin_decode(iconvg_canvas* c) {
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  return err_msg;
}

static const char*  //
iconvg_private_cairo_canvas__begin_drawing(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_new_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  if (iconvg_paint__is_flat_color(p)) {
    iconvg_nonpremul_color k = iconvg_paint__flat_color_as_nonpremul_color(p);
    cairo_set_source_rgba(cr, k.rgba[0] / 255.0, k.rgba[1] / 255.0,
                          k.rgba[2] / 255.0, k.rgba[3] / 255.0);
  } else {
    // TODO: gradients.
    cairo_set_source_rgb(cr, 1, 1, 1);
  }
  cairo_fill(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_move_to(cr, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_path(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_close_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_line_to(cr, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  // Cairo doesn't have explicit support for quadratic Bézier curves, only
  // linear and cubic ones. However, a "Bézier curve of degree n can be
  // converted into a Bézier curve of degree n + 1 with the same shape", per
  // https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Degree_elevation
  //
  // Here, we perform "degree elevation" from [x0, x1, x2] to [X0, X1, X2, X3]
  // = [x0, ((⅓ * x0) + (⅔ * x1)), ((⅔ * x1) + (⅓ * x2)), c2] and likewise for
  // the y dimension.
  double X0;
  double Y0;
  cairo_get_current_point(cr, &X0, &Y0);
  double twice_x1 = ((double)x1) * 2;
  double twice_y1 = ((double)y1) * 2;
  double X3 = ((double)x2);
  double Y3 = ((double)y2);
  double X1 = (X0 + twice_x1) / 3;
  double Y1 = (Y0 + twice_y1) / 3;
  double X2 = (X3 + twice_x1) / 3;
  double Y2 = (Y3 + twice_y1) / 3;
  cairo_curve_to(cr, X1, Y1, X2, Y2, X3, Y3);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_cube_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2,
                                          float x3,
                                          float y3) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_arc_to(iconvg_canvas* c,
                                         float radius_x,
                                         float radius_y,
                                         float x_axis_rotation,
                                         bool large_arc,
                                         bool sweep,
                                         float final_x,
                                         float final_y) {
  // TODO: convert from SVG's parameterization to Cairo's. Until then, we
  // substitute in a placeholder cairo_line_to.
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_line_to(cr, final_x, final_y);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                 iconvg_rectangle_f32 viewbox) {
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return NULL;
}

static const iconvg_canvas_vtable  //
    iconvg_private_cairo_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_cairo_canvas__begin_decode,
        &iconvg_private_cairo_canvas__end_decode,
        &iconvg_private_cairo_canvas__begin_drawing,
        &iconvg_private_cairo_canvas__end_drawing,
        &iconvg_private_cairo_canvas__begin_path,
        &iconvg_private_cairo_canvas__end_path,
        &iconvg_private_cairo_canvas__path_line_to,
        &iconvg_private_cairo_canvas__path_quad_to,
        &iconvg_private_cairo_canvas__path_cube_to,
        &iconvg_private_cairo_canvas__path_arc_to,
        &iconvg_private_cairo_canvas__on_metadata_viewbox,
        &iconvg_private_cairo_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_cairo_canvas(cairo_t* cr) {
  if (!cr) {
    return iconvg_make_broken_canvas(iconvg_error_invalid_constructor_argument);
  }
  iconvg_canvas c;
  c.vtable = &iconvg_private_cairo_canvas_vtable;
  c.context_nonconst_ptr0 = cr;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = NULL;
  c.context_extra = 0;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND

// -------------------------------- #include "./colors.c"

const uint8_t iconvg_private_one_byte_colors[512] = {
    0x00, 0x00, 0x00, 0xFF,  //
    0x00, 0x00, 0x40, 0xFF,  //
    0x00, 0x00, 0x80, 0xFF,  //
    0x00, 0x00, 0xC0, 0xFF,  //
    0x00, 0x00, 0xFF, 0xFF,  //
    0x00, 0x40, 0x00, 0xFF,  //
    0x00, 0x40, 0x40, 0xFF,  //
    0x00, 0x40, 0x80, 0xFF,  //
    0x00, 0x40, 0xC0, 0xFF,  //
    0x00, 0x40, 0xFF, 0xFF,  //
    0x00, 0x80, 0x00, 0xFF,  //
    0x00, 0x80, 0x40, 0xFF,  //
    0x00, 0x80, 0x80, 0xFF,  //
    0x00, 0x80, 0xC0, 0xFF,  //
    0x00, 0x80, 0xFF, 0xFF,  //
    0x00, 0xC0, 0x00, 0xFF,  //
    0x00, 0xC0, 0x40, 0xFF,  //
    0x00, 0xC0, 0x80, 0xFF,  //
    0x00, 0xC0, 0xC0, 0xFF,  //
    0x00, 0xC0, 0xFF, 0xFF,  //
    0x00, 0xFF, 0x00, 0xFF,  //
    0x00, 0xFF, 0x40, 0xFF,  //
    0x00, 0xFF, 0x80, 0xFF,  //
    0x00, 0xFF, 0xC0, 0xFF,  //
    0x00, 0xFF, 0xFF, 0xFF,  //
    0x40, 0x00, 0x00, 0xFF,  //
    0x40, 0x00, 0x40, 0xFF,  //
    0x40, 0x00, 0x80, 0xFF,  //
    0x40, 0x00, 0xC0, 0xFF,  //
    0x40, 0x00, 0xFF, 0xFF,  //
    0x40, 0x40, 0x00, 0xFF,  //
    0x40, 0x40, 0x40, 0xFF,  //
    0x40, 0x40, 0x80, 0xFF,  //
    0x40, 0x40, 0xC0, 0xFF,  //
    0x40, 0x40, 0xFF, 0xFF,  //
    0x40, 0x80, 0x00, 0xFF,  //
    0x40, 0x80, 0x40, 0xFF,  //
    0x40, 0x80, 0x80, 0xFF,  //
    0x40, 0x80, 0xC0, 0xFF,  //
    0x40, 0x80, 0xFF, 0xFF,  //
    0x40, 0xC0, 0x00, 0xFF,  //
    0x40, 0xC0, 0x40, 0xFF,  //
    0x40, 0xC0, 0x80, 0xFF,  //
    0x40, 0xC0, 0xC0, 0xFF,  //
    0x40, 0xC0, 0xFF, 0xFF,  //
    0x40, 0xFF, 0x00, 0xFF,  //
    0x40, 0xFF, 0x40, 0xFF,  //
    0x40, 0xFF, 0x80, 0xFF,  //
    0x40, 0xFF, 0xC0, 0xFF,  //
    0x40, 0xFF, 0xFF, 0xFF,  //
    0x80, 0x00, 0x00, 0xFF,  //
    0x80, 0x00, 0x40, 0xFF,  //
    0x80, 0x00, 0x80, 0xFF,  //
    0x80, 0x00, 0xC0, 0xFF,  //
    0x80, 0x00, 0xFF, 0xFF,  //
    0x80, 0x40, 0x00, 0xFF,  //
    0x80, 0x40, 0x40, 0xFF,  //
    0x80, 0x40, 0x80, 0xFF,  //
    0x80, 0x40, 0xC0, 0xFF,  //
    0x80, 0x40, 0xFF, 0xFF,  //
    0x80, 0x80, 0x00, 0xFF,  //
    0x80, 0x80, 0x40, 0xFF,  //
    0x80, 0x80, 0x80, 0xFF,  //
    0x80, 0x80, 0xC0, 0xFF,  //
    0x80, 0x80, 0xFF, 0xFF,  //
    0x80, 0xC0, 0x00, 0xFF,  //
    0x80, 0xC0, 0x40, 0xFF,  //
    0x80, 0xC0, 0x80, 0xFF,  //
    0x80, 0xC0, 0xC0, 0xFF,  //
    0x80, 0xC0, 0xFF, 0xFF,  //
    0x80, 0xFF, 0x00, 0xFF,  //
    0x80, 0xFF, 0x40, 0xFF,  //
    0x80, 0xFF, 0x80, 0xFF,  //
    0x80, 0xFF, 0xC0, 0xFF,  //
    0x80, 0xFF, 0xFF, 0xFF,  //
    0xC0, 0x00, 0x00, 0xFF,  //
    0xC0, 0x00, 0x40, 0xFF,  //
    0xC0, 0x00, 0x80, 0xFF,  //
    0xC0, 0x00, 0xC0, 0xFF,  //
    0xC0, 0x00, 0xFF, 0xFF,  //
    0xC0, 0x40, 0x00, 0xFF,  //
    0xC0, 0x40, 0x40, 0xFF,  //
    0xC0, 0x40, 0x80, 0xFF,  //
    0xC0, 0x40, 0xC0, 0xFF,  //
    0xC0, 0x40, 0xFF, 0xFF,  //
    0xC0, 0x80, 0x00, 0xFF,  //
    0xC0, 0x80, 0x40, 0xFF,  //
    0xC0, 0x80, 0x80, 0xFF,  //
    0xC0, 0x80, 0xC0, 0xFF,  //
    0xC0, 0x80, 0xFF, 0xFF,  //
    0xC0, 0xC0, 0x00, 0xFF,  //
    0xC0, 0xC0, 0x40, 0xFF,  //
    0xC0, 0xC0, 0x80, 0xFF,  //
    0xC0, 0xC0, 0xC0, 0xFF,  //
    0xC0, 0xC0, 0xFF, 0xFF,  //
    0xC0, 0xFF, 0x00, 0xFF,  //
    0xC0, 0xFF, 0x40, 0xFF,  //
    0xC0, 0xFF, 0x80, 0xFF,  //
    0xC0, 0xFF, 0xC0, 0xFF,  //
    0xC0, 0xFF, 0xFF, 0xFF,  //
    0xFF, 0x00, 0x00, 0xFF,  //
    0xFF, 0x00, 0x40, 0xFF,  //
    0xFF, 0x00, 0x80, 0xFF,  //
    0xFF, 0x00, 0xC0, 0xFF,  //
    0xFF, 0x00, 0xFF, 0xFF,  //
    0xFF, 0x40, 0x00, 0xFF,  //
    0xFF, 0x40, 0x40, 0xFF,  //
    0xFF, 0x40, 0x80, 0xFF,  //
    0xFF, 0x40, 0xC0, 0xFF,  //
    0xFF, 0x40, 0xFF, 0xFF,  //
    0xFF, 0x80, 0x00, 0xFF,  //
    0xFF, 0x80, 0x40, 0xFF,  //
    0xFF, 0x80, 0x80, 0xFF,  //
    0xFF, 0x80, 0xC0, 0xFF,  //
    0xFF, 0x80, 0xFF, 0xFF,  //
    0xFF, 0xC0, 0x00, 0xFF,  //
    0xFF, 0xC0, 0x40, 0xFF,  //
    0xFF, 0xC0, 0x80, 0xFF,  //
    0xFF, 0xC0, 0xC0, 0xFF,  //
    0xFF, 0xC0, 0xFF, 0xFF,  //
    0xFF, 0xFF, 0x00, 0xFF,  //
    0xFF, 0xFF, 0x40, 0xFF,  //
    0xFF, 0xFF, 0x80, 0xFF,  //
    0xFF, 0xFF, 0xC0, 0xFF,  //
    0xFF, 0xFF, 0xFF, 0xFF,  //
    0xC0, 0xC0, 0xC0, 0xC0,  //
    0x80, 0x80, 0x80, 0x80,  //
    0x00, 0x00, 0x00, 0x00,  //
};

const iconvg_palette iconvg_private_default_palette = {{
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
    {{0x00, 0x00, 0x00, 0xFF}},  //
}};

// -------------------------------- #include "./debug.c"

static const char*  //
iconvg_private_debug_canvas__begin_decode(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_decode()\n", ((const char*)(c->context_const_ptr)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_decode)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    const char* quote = err_msg ? "\"" : "";
    fprintf(f, "%send_decode(%s%s%s, %zu, %zu)\n",
            ((const char*)(c->context_const_ptr)), quote,
            err_msg ? err_msg : "NULL", quote, num_bytes_consumed,
            num_bytes_remaining);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return err_msg;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_decode)(wrapped, err_msg, num_bytes_consumed,
                                        num_bytes_remaining);
}

static const char*  //
iconvg_private_debug_canvas__begin_drawing(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_drawing()\n", ((const char*)(c->context_const_ptr)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_drawing)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    if (iconvg_paint__is_flat_color(p)) {
      iconvg_premul_color k = iconvg_paint__flat_color_as_premul_color(p);
      fprintf(f, "%send_drawing(flat_color{%02X:%02X:%02X:%02X})\n",
              ((const char*)(c->context_const_ptr)), ((int)(k.rgba[0])),
              ((int)(k.rgba[1])), ((int)(k.rgba[2])), ((int)(k.rgba[3])));
    } else {
      // TODO: a more informative printf message.
      fprintf(f, "%send_drawing(gradient{...})\n",
              ((const char*)(c->context_const_ptr)));
    }
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_drawing)(wrapped, p);
}

static const char*  //
iconvg_private_debug_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_path(%g, %g)\n", ((const char*)(c->context_const_ptr)),
            x0, y0);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_path)(wrapped, x0, y0);
}

static const char*  //
iconvg_private_debug_canvas__end_path(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%send_path()\n", ((const char*)(c->context_const_ptr)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_path)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_line_to(%g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_line_to)(wrapped, x1, y1);
}

static const char*  //
iconvg_private_debug_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_quad_to(%g, %g, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1, x2, y2);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_quad_to)(wrapped, x1, y1, x2, y2);
}

static const char*  //
iconvg_private_debug_canvas__path_cube_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2,
                                          float x3,
                                          float y3) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_cube_to(%g, %g, %g, %g, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1, x2, y2, x3, y3);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_cube_to)(wrapped, x1, y1, x2, y2, x3, y3);
}

static const char*  //
iconvg_private_debug_canvas__path_arc_to(iconvg_canvas* c,
                                         float radius_x,
                                         float radius_y,
                                         float x_axis_rotation,
                                         bool large_arc,
                                         bool sweep,
                                         float final_x,
                                         float final_y) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_arc_to(%g, %g, %g, %d, %d, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), radius_x, radius_y,
            x_axis_rotation, (int)large_arc, (int)sweep, final_x, final_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_arc_to)(wrapped, radius_x, radius_y,
                                         x_axis_rotation, large_arc, sweep,
                                         final_x, final_y);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                 iconvg_rectangle_f32 viewbox) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%son_metadata_viewbox({%g, %g, %g, %g})\n",
            ((const char*)(c->context_const_ptr)), viewbox.min_x, viewbox.min_y,
            viewbox.max_x, viewbox.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->on_metadata_viewbox)(wrapped, viewbox);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    int j = iconvg_private_last_color_that_isnt_opaque_black(suggested_palette);
    if (j < 0) {
      fprintf(f, "%son_metadata_suggested_palette(...)\n",
              ((const char*)(c->context_const_ptr)));
    } else {
      fprintf(f, "%son_metadata_suggested_palette(",
              ((const char*)(c->context_const_ptr)));
      for (int i = 0; i <= j; i++) {
        fprintf(f, "%02X:%02X:%02X:%02X%s",
                ((int)(suggested_palette->colors[i].rgba[0])),
                ((int)(suggested_palette->colors[i].rgba[1])),
                ((int)(suggested_palette->colors[i].rgba[2])),
                ((int)(suggested_palette->colors[i].rgba[3])),
                (i < 63) ? ", " : ")\n");
      }
      if (j < 63) {
        fprintf(f, "...)\n");
      } else {
        fprintf(f, ")\n");
      }
    }
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->on_metadata_suggested_palette)(wrapped,
                                                           suggested_palette);
}

static const iconvg_canvas_vtable  //
    iconvg_private_debug_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_debug_canvas__begin_decode,
        &iconvg_private_debug_canvas__end_decode,
        &iconvg_private_debug_canvas__begin_drawing,
        &iconvg_private_debug_canvas__end_drawing,
        &iconvg_private_debug_canvas__begin_path,
        &iconvg_private_debug_canvas__end_path,
        &iconvg_private_debug_canvas__path_line_to,
        &iconvg_private_debug_canvas__path_quad_to,
        &iconvg_private_debug_canvas__path_cube_to,
        &iconvg_private_debug_canvas__path_arc_to,
        &iconvg_private_debug_canvas__on_metadata_viewbox,
        &iconvg_private_debug_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_debug_canvas(FILE* f,
                         const char* message_prefix,
                         iconvg_canvas* wrapped) {
  iconvg_canvas c;
  c.vtable = &iconvg_private_debug_canvas_vtable;
  c.context_nonconst_ptr0 = wrapped;
  c.context_nonconst_ptr1 = f;
  c.context_const_ptr = message_prefix ? message_prefix : "";
  c.context_extra = 0;
  return c;
}

// -------------------------------- #include "./decoder.c"

static void  //
iconvg_private_decoder__advance_to_ptr(iconvg_private_decoder* self,
                                       const uint8_t* new_ptr) {
  if (new_ptr >= self->ptr) {
    size_t delta = ((size_t)(new_ptr - self->ptr));
    if (delta <= self->len) {
      self->ptr += delta;
      self->len -= delta;
      return;
    }
  }
  self->ptr = NULL;
  self->len = 0;
}

static iconvg_private_decoder  //
iconvg_private_decoder__limit_u32(iconvg_private_decoder* self,
                                  uint32_t limit) {
  iconvg_private_decoder d;
  d.ptr = self->ptr;
  d.len = (limit < self->len) ? limit : self->len;
  return d;
}

static void  //
iconvg_private_decoder__skip_to_the_end(iconvg_private_decoder* self) {
  self->ptr += self->len;
  self->len = 0;
}

// ----

static bool  //
iconvg_private_decoder__decode_coordinate_number(iconvg_private_decoder* self,
                                                 float* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      int32_t i = (int32_t)(v >> 1);
      *dst = ((float)(i - 64));
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        int32_t i = (int32_t)(iconvg_private_peek_u16le(self->ptr) >> 2);
        *dst = ((float)(i - (128 * 64))) / 64.0f;
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        // TODO: reject NaNs?
        *dst = iconvg_private_reinterpret_from_u32_to_f32(
            0xFFFFFFFCu & iconvg_private_peek_u32le(self->ptr));
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

static bool  //
iconvg_private_decoder__decode_natural_number(iconvg_private_decoder* self,
                                              uint32_t* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      *dst = v >> 1;
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        *dst = iconvg_private_peek_u16le(self->ptr) >> 2;
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        *dst = iconvg_private_peek_u32le(self->ptr) >> 2;
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

static bool  //
iconvg_private_decoder__decode_real_number(iconvg_private_decoder* self,
                                           float* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      *dst = (float)(v >> 1);
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        *dst = (float)(iconvg_private_peek_u16le(self->ptr) >> 2);
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        // TODO: reject NaNs?
        *dst = iconvg_private_reinterpret_from_u32_to_f32(
            0xFFFFFFFCu & iconvg_private_peek_u32le(self->ptr));
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

static bool  //
iconvg_private_decoder__decode_zero_to_one_number(iconvg_private_decoder* self,
                                                  float* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      *dst = (float)(((double)(v >> 1)) / 120.0);
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        *dst = (float)(((double)(iconvg_private_peek_u16le(self->ptr) >> 2)) /
                       15120.0);
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        // TODO: reject NaNs?
        *dst = iconvg_private_reinterpret_from_u32_to_f32(
            0xFFFFFFFCu & iconvg_private_peek_u32le(self->ptr));
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

// ----

static bool  //
iconvg_private_decoder__decode_magic_identifier(iconvg_private_decoder* self) {
  if ((self->len < 4) &&         //
      (self->ptr[0] != 0x89) &&  //
      (self->ptr[1] != 0x49) &&  //
      (self->ptr[2] != 0x56) &&  //
      (self->ptr[3] != 0x47)) {
    return false;
  }
  self->ptr += 4;
  self->len -= 4;
  return true;
}

static bool  //
iconvg_private_decoder__decode_metadata_viewbox(iconvg_private_decoder* self,
                                                iconvg_rectangle_f32* dst) {
  return iconvg_private_decoder__decode_coordinate_number(self, &dst->min_x) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->min_y) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->max_x) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->max_y);
}

static bool  //
iconvg_private_decoder__decode_metadata_suggested_palette(
    iconvg_private_decoder* self,
    iconvg_palette* dst) {
  if (self->len == 0) {
    return false;
  }
  uint8_t spec = self->ptr[0];
  self->ptr += 1;
  self->len -= 1;

  size_t n = 1 + (spec & 0x3F);
  size_t bytes_per_elem = 1 + (spec >> 6);
  if (self->len != (n * bytes_per_elem)) {
    return false;
  }
  const uint8_t* p = self->ptr;
  self->ptr += self->len;
  self->len = 0;

  iconvg_premul_color* c = &dst->colors[0];
  switch (bytes_per_elem) {
    case 1:
      for (; n > 0; n--) {
        uint8_t u = *p++;
        uint32_t rgba =
            (u < 0x80) ? iconvg_private_peek_u32le(
                             &iconvg_private_one_byte_colors[4 * ((size_t)u)])
                       : 0xFF000000u;
        iconvg_private_poke_u32le(&c->rgba[0], rgba);
        c++;
      }
      break;

    case 2:
      for (; n > 0; n--) {
        uint8_t rg = *p++;
        c->rgba[0] = 0x11 * (rg >> 4);
        c->rgba[1] = 0x11 * (rg & 0x0F);
        uint8_t ba = *p++;
        c->rgba[2] = 0x11 * (ba >> 4);
        c->rgba[3] = 0x11 * (ba & 0x0F);
        c++;
      }
      break;

    case 3:
      for (; n > 0; n--) {
        c->rgba[0] = *p++;
        c->rgba[1] = *p++;
        c->rgba[2] = *p++;
        c->rgba[3] = 0xFF;
        c++;
      }
      break;

    case 4:
      for (; n > 0; n--) {
        c->rgba[0] = *p++;
        c->rgba[1] = *p++;
        c->rgba[2] = *p++;
        c->rgba[3] = *p++;
        c++;
      }
      break;
  }
  return true;
}

// ----

static const char*  //
iconvg_private_execute_bytecode(iconvg_canvas* c,
                                iconvg_private_decoder* d,
                                iconvg_paint* state) {
  // adjustments are the ADJ values from the IconVG spec.
  static const uint32_t adjustments[8] = {0, 1, 2, 3, 4, 5, 6, 0};

  // Drawing ops will typically set curr_x and curr_y. They also set x1 and y1
  // in case the subsequent op is smooth and needs an implicit point.
  float curr_x = +0.0f;
  float curr_y = +0.0f;
  float x1 = +0.0f;
  float y1 = +0.0f;
  float x2 = +0.0f;
  float y2 = +0.0f;
  float x3 = +0.0f;
  float y3 = +0.0f;
  uint32_t flags = 0;

  // sel[0] and sel[1] are the CSEL and NSEL registers.
  uint32_t sel[2] = {0};
  float lod[2] = {0};

styling_mode:
  while (true) {
    if (d->len == 0) {
      return NULL;
    }
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    if (opcode < 0x80) {
      sel[opcode >> 6] = opcode & 0x3F;
      continue;

    } else if (opcode < 0x88) {  // Set CREG[etc]; 1 byte color.
      if (d->len < 1) {
        return iconvg_error_bad_color;
      }
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      uint8_t* rgba = &state->creg.colors[creg_index].rgba[0];
      iconvg_private_set_one_byte_color(rgba, &state->custom_palette,
                                        &state->creg, d->ptr[0]);
      d->ptr += 1;
      d->len -= 1;
      sel[0] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0x90) {  // Set CREG[etc]; 2 byte color.
      if (d->len < 2) {
        return iconvg_error_bad_color;
      }
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      uint8_t* rgba = &state->creg.colors[creg_index].rgba[0];
      rgba[0] = 0x11 * (d->ptr[0] >> 4);
      rgba[1] = 0x11 * (d->ptr[0] & 0x0F);
      rgba[2] = 0x11 * (d->ptr[1] >> 4);
      rgba[3] = 0x11 * (d->ptr[1] & 0x0F);
      d->ptr += 2;
      d->len -= 2;
      sel[0] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0x98) {  // Set CREG[etc]; 3 byte (direct) color.
      if (d->len < 3) {
        return iconvg_error_bad_color;
      }
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      uint8_t* rgba = &state->creg.colors[creg_index].rgba[0];
      rgba[0] = d->ptr[0];
      rgba[1] = d->ptr[1];
      rgba[2] = d->ptr[2];
      rgba[3] = 0xFF;
      d->ptr += 3;
      d->len -= 3;
      sel[0] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xA0) {  // Set CREG[etc]; 4 byte color.
      if (d->len < 4) {
        return iconvg_error_bad_color;
      }
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      uint8_t* rgba = &state->creg.colors[creg_index].rgba[0];
      rgba[0] = d->ptr[0];
      rgba[1] = d->ptr[1];
      rgba[2] = d->ptr[2];
      rgba[3] = d->ptr[3];
      d->ptr += 4;
      d->len -= 4;
      sel[0] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xA8) {  // Set CREG[etc]; 3 byte (indirect) color.
      if (d->len < 3) {
        return iconvg_error_bad_color;
      }
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      uint8_t* rgba = &state->creg.colors[creg_index].rgba[0];
      uint8_t p[4] = {0};
      uint8_t q[4] = {0};
      iconvg_private_set_one_byte_color(&p[0], &state->custom_palette,
                                        &state->creg, d->ptr[1]);
      iconvg_private_set_one_byte_color(&q[0], &state->custom_palette,
                                        &state->creg, d->ptr[2]);
      uint32_t q_blend = d->ptr[0];
      uint32_t p_blend = 255 - q_blend;
      rgba[0] = (uint8_t)(((p_blend * p[0]) + (q_blend * q[0]) + 128) / 255);
      rgba[1] = (uint8_t)(((p_blend * p[1]) + (q_blend * q[1]) + 128) / 255);
      rgba[2] = (uint8_t)(((p_blend * p[2]) + (q_blend * q[2]) + 128) / 255);
      rgba[3] = (uint8_t)(((p_blend * p[3]) + (q_blend * q[3]) + 128) / 255);
      d->ptr += 3;
      d->len -= 3;
      sel[0] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xB0) {  // Set NREG[etc]; real number.
      uint8_t nreg_index = (sel[1] - adjustments[opcode & 0x07]) & 0x3F;
      float* num = &state->nreg[nreg_index];
      if (!iconvg_private_decoder__decode_real_number(d, num)) {
        return iconvg_error_bad_number;
      }
      sel[1] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xB8) {  // Set NREG[etc]; coordinate number.
      uint8_t nreg_index = (sel[1] - adjustments[opcode & 0x07]) & 0x3F;
      float* num = &state->nreg[nreg_index];
      if (!iconvg_private_decoder__decode_coordinate_number(d, num)) {
        return iconvg_error_bad_coordinate;
      }
      sel[1] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xC0) {  // Set NREG[etc]; zero-to-one number.
      uint8_t nreg_index = (sel[1] - adjustments[opcode & 0x07]) & 0x3F;
      float* num = &state->nreg[nreg_index];
      if (!iconvg_private_decoder__decode_zero_to_one_number(d, num)) {
        return iconvg_error_bad_number;
      }
      sel[1] += ((opcode & 0x07) == 0x07) ? 1 : 0;
      continue;

    } else if (opcode < 0xC7) {  // Switch to the drawing mode.
      if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
          !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
        return iconvg_error_bad_coordinate;
      }
      ICONVG_PRIVATE_TRY((*c->vtable->begin_drawing)(c));
      ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
      x1 = curr_x;
      y1 = curr_y;
      memcpy(&state->paint_rgba, &state->creg.colors[sel[0]],
             sizeof(state->paint_rgba));
      // TODO: if H is outside the LOD range then skip the drawing.
      goto drawing_mode;

    } else if (opcode < 0xC8) {  // Set Level of Detail bounds.
      if (!iconvg_private_decoder__decode_real_number(d, &lod[0]) ||
          !iconvg_private_decoder__decode_real_number(d, &lod[1])) {
        return iconvg_error_bad_number;
      }
      continue;
    }

    return iconvg_error_bad_styling_opcode;
  }

drawing_mode:
  while (true) {
    if (d->len == 0) {
      return iconvg_error_bad_path_unfinished;
    }
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    switch (opcode >> 4) {
      case 0x00:
      case 0x01: {  // 'L' mnemonic: absolute line_to.
        for (int reps = opcode & 0x1F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
          x1 = curr_x;
          y1 = curr_y;
        }
        continue;
      }

      case 0x02:
      case 0x03: {  // 'l' mnemonic: relative line_to.
        for (int reps = opcode & 0x1F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
            return iconvg_error_bad_coordinate;
          }
          curr_x += x1;
          curr_y += y1;
          ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
          x1 = curr_x;
          y1 = curr_y;
        }
        continue;
      }

      case 0x04: {  // 'T' mnemonic: absolute smooth quad_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->path_quad_to)(c, x1, y1, x2, y2));
          curr_x = x2;
          curr_y = y2;
          x1 = (2 * curr_x) - x1;
          y1 = (2 * curr_y) - y1;
        }
        continue;
      }

      case 0x05: {  // 't' mnemonic: relative smooth quad_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2)) {
            return iconvg_error_bad_coordinate;
          }
          x2 += curr_x;
          y2 += curr_y;
          ICONVG_PRIVATE_TRY((*c->vtable->path_quad_to)(c, x1, y1, x2, y2));
          curr_x = x2;
          curr_y = y2;
          x1 = (2 * curr_x) - x1;
          y1 = (2 * curr_y) - y1;
        }
        continue;
      }

      case 0x06: {  // 'Q' mnemonic: absolute quad_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->path_quad_to)(c, x1, y1, x2, y2));
          curr_x = x2;
          curr_y = y2;
          x1 = (2 * curr_x) - x1;
          y1 = (2 * curr_y) - y1;
        }
        continue;
      }

      case 0x07: {  // 'q' mnemonic: relative quad_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2)) {
            return iconvg_error_bad_coordinate;
          }
          x1 += curr_x;
          y1 += curr_y;
          x2 += curr_x;
          y2 += curr_y;
          ICONVG_PRIVATE_TRY((*c->vtable->path_quad_to)(c, x1, y1, x2, y2));
          curr_x = x2;
          curr_y = y2;
          x1 = (2 * curr_x) - x1;
          y1 = (2 * curr_y) - y1;
        }
        continue;
      }

      case 0x08: {  // 'S' mnemonic: absolute smooth cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * curr_x) - x2;
          y1 = (2 * curr_y) - y2;
        }
        continue;
      }

      case 0x09: {  // 's' mnemonic: relative smooth cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          x2 += curr_x;
          y2 += curr_y;
          x3 += curr_x;
          y3 += curr_y;
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * curr_x) - x2;
          y1 = (2 * curr_y) - y2;
        }
        continue;
      }

      case 0x0A: {  // 'C' mnemonic: absolute cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * curr_x) - x2;
          y1 = (2 * curr_y) - y2;
        }
        continue;
      }

      case 0x0B: {  // 'c' mnemonic: relative cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          x1 += curr_x;
          y1 += curr_y;
          x2 += curr_x;
          y2 += curr_y;
          x3 += curr_x;
          y3 += curr_y;
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * curr_x) - x2;
          y1 = (2 * curr_y) - y2;
        }
        continue;
      }

      case 0x0C: {  // 'A' mnemonic: absolute arc_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_zero_to_one_number(d, &x2) ||
              !iconvg_private_decoder__decode_natural_number(d, &flags) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
            return iconvg_error_bad_coordinate;
          }
          // TODO: do we have to scale x1 and y1 (radius_x and radius_y)?
          ICONVG_PRIVATE_TRY((*c->vtable->path_arc_to)(
              c, x1, y1, x2, flags & 0x01, flags & 0x02, curr_x, curr_y));
          x1 = curr_x;
          y1 = curr_y;
        }
        continue;
      }

      case 0x0D: {  // 'a' mnemonic: relative arc_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_zero_to_one_number(d, &x2) ||
              !iconvg_private_decoder__decode_natural_number(d, &flags) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          // TODO: do we have to scale x1 and y1 (radius_x and radius_y)?
          curr_x += x3;
          curr_y += y3;
          ICONVG_PRIVATE_TRY((*c->vtable->path_arc_to)(
              c, x1, y1, x2, flags & 0x01, flags & 0x02, curr_x, curr_y));
          x1 = curr_x;
          y1 = curr_y;
        }
        continue;
      }
    }

    switch (opcode) {
      case 0xE1: {  // 'z' mnemonic: close_path.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        ICONVG_PRIVATE_TRY((*c->vtable->end_drawing)(c, state));
        goto styling_mode;
      }

      case 0xE2: {  // 'z; M' mnemonics: close_path; absolute move_to.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
            !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE3: {  // 'z; m' mnemonics: close_path; relative move_to.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
            !iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_x += x1;
        curr_y += y1;
        ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE6: {  // 'H' mnemonic: absolute horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE7: {  // 'h' mnemonic: relative horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &x1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_x += x1;
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE8: {  // 'V' mnemonic: absolute vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE9: {  // 'v' mnemonic: relative vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_y += y1;
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }
    }

    return iconvg_error_bad_drawing_opcode;
  }
  return iconvg_private_internal_error_unreachable;
}

// ----

const char*  //
iconvg_decode_viewbox(iconvg_rectangle_f32* dst_viewbox,
                      const uint8_t* src_ptr,
                      size_t src_len) {
  iconvg_private_decoder d;
  d.ptr = src_ptr;
  d.len = src_len;

  if (!iconvg_private_decoder__decode_magic_identifier(&d)) {
    return iconvg_error_bad_magic_identifier;
  }
  uint32_t num_metadata_chunks;
  if (!iconvg_private_decoder__decode_natural_number(&d,
                                                     &num_metadata_chunks)) {
    return iconvg_error_bad_metadata;
  }

  bool use_default_viewbox = true;
  int32_t previous_metadata_id = -1;
  for (; num_metadata_chunks > 0; num_metadata_chunks--) {
    uint32_t chunk_length;
    if (!iconvg_private_decoder__decode_natural_number(&d, &chunk_length) ||
        (chunk_length > d.len)) {
      return iconvg_error_bad_metadata;
    }
    iconvg_private_decoder chunk =
        iconvg_private_decoder__limit_u32(&d, chunk_length);
    uint32_t metadata_id;
    if (!iconvg_private_decoder__decode_natural_number(&chunk, &metadata_id)) {
      return iconvg_error_bad_metadata;
    } else if (previous_metadata_id >= ((int32_t)metadata_id)) {
      return iconvg_error_bad_metadata_id_order;
    }

    if (metadata_id == 0) {  // MID 0 (ViewBox).
      use_default_viewbox = false;
      iconvg_rectangle_f32 r;
      if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk, &r) ||
          (chunk.len != 0)) {
        return iconvg_error_bad_metadata_viewbox;
      } else if (dst_viewbox) {
        *dst_viewbox = r;
      }
    }

    iconvg_private_decoder__skip_to_the_end(&chunk);
    iconvg_private_decoder__advance_to_ptr(&d, chunk.ptr);
    previous_metadata_id = ((int32_t)metadata_id);
  }

  if (use_default_viewbox && dst_viewbox) {
    *dst_viewbox = iconvg_private_default_viewbox();
  }
  return NULL;
}

static const char*  //
iconvg_private_decode(iconvg_canvas* c,
                      iconvg_private_decoder* d,
                      const iconvg_decode_options* options) {
  iconvg_rectangle_f32 viewbox = iconvg_private_default_viewbox();
  iconvg_paint state;
  memset(&state.paint_rgba, 0, sizeof(state.paint_rgba));
  memcpy(&state.custom_palette, &iconvg_private_default_palette,
         sizeof(state.custom_palette));

  if (!iconvg_private_decoder__decode_magic_identifier(d)) {
    return iconvg_error_bad_magic_identifier;
  }
  uint32_t num_metadata_chunks;
  if (!iconvg_private_decoder__decode_natural_number(d, &num_metadata_chunks)) {
    return iconvg_error_bad_metadata;
  }

  int32_t previous_metadata_id = -1;
  for (; num_metadata_chunks > 0; num_metadata_chunks--) {
    uint32_t chunk_length;
    if (!iconvg_private_decoder__decode_natural_number(d, &chunk_length) ||
        (chunk_length > d->len)) {
      return iconvg_error_bad_metadata;
    }
    iconvg_private_decoder chunk =
        iconvg_private_decoder__limit_u32(d, chunk_length);
    uint32_t metadata_id;
    if (!iconvg_private_decoder__decode_natural_number(&chunk, &metadata_id)) {
      return iconvg_error_bad_metadata;
    } else if (previous_metadata_id >= ((int32_t)metadata_id)) {
      return iconvg_error_bad_metadata_id_order;
    }

    switch (metadata_id) {
      case 0:  // MID 0 (ViewBox).
        if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk,
                                                             &viewbox) ||
            (chunk.len != 0)) {
          return iconvg_error_bad_metadata_viewbox;
        }
        break;

      case 1:  // MID 1 (Suggested Palette).
        if (!iconvg_private_decoder__decode_metadata_suggested_palette(
                &chunk, &state.custom_palette) ||
            (chunk.len != 0)) {
          return iconvg_error_bad_metadata_suggested_palette;
        }
        break;

      default:
        return iconvg_error_bad_metadata;
    }

    iconvg_private_decoder__skip_to_the_end(&chunk);
    iconvg_private_decoder__advance_to_ptr(d, chunk.ptr);
    previous_metadata_id = ((int32_t)metadata_id);
  }

  ICONVG_PRIVATE_TRY((*c->vtable->on_metadata_viewbox)(c, viewbox));
  ICONVG_PRIVATE_TRY(
      (*c->vtable->on_metadata_suggested_palette)(c, &state.custom_palette));

  if (options && options->palette) {
    memcpy(&state.custom_palette, options->palette,
           sizeof(state.custom_palette));
  }

  memcpy(&state.creg, &state.custom_palette, sizeof(state.creg));
  memset(&state.nreg[0], 0, sizeof(state.nreg));
  return iconvg_private_execute_bytecode(c, d, &state);
}

const char*  //
iconvg_decode(iconvg_canvas* dst_canvas,
              const uint8_t* src_ptr,
              size_t src_len,
              const iconvg_decode_options* options) {
  iconvg_canvas fallback_canvas = iconvg_make_debug_canvas(NULL, NULL, NULL);
  if (!dst_canvas) {
    dst_canvas = &fallback_canvas;
  }

  if (!dst_canvas->vtable) {
    return iconvg_error_null_vtable;
  } else if (dst_canvas->vtable->sizeof__iconvg_canvas_vtable !=
             sizeof(iconvg_canvas_vtable)) {
    // If we want to support multiple library versions (with dynamic linking),
    // we could detect older versions here (with smaller vtable sizes) and
    // substitute in an adapter implementation.
    return iconvg_error_unsupported_vtable;
  }

  iconvg_private_decoder d;
  d.ptr = src_ptr;
  d.len = src_len;

  const char* err_msg = (*dst_canvas->vtable->begin_decode)(dst_canvas);
  if (!err_msg) {
    err_msg = iconvg_private_decode(dst_canvas, &d, options);
  }
  return (*dst_canvas->vtable->end_decode)(dst_canvas, err_msg, src_len - d.len,
                                           d.len);
}

// -------------------------------- #include "./errors.c"

const char iconvg_error_bad_color[] =  //
    "iconvg: bad color";
const char iconvg_error_bad_coordinate[] =  //
    "iconvg: bad coordinate";
const char iconvg_error_bad_drawing_opcode[] =  //
    "iconvg: bad drawing opcode";
const char iconvg_error_bad_magic_identifier[] =  //
    "iconvg: bad magic identifier";
const char iconvg_error_bad_metadata[] =  //
    "iconvg: bad metadata";
const char iconvg_error_bad_metadata_id_order[] =  //
    "iconvg: bad metadata ID order";
const char iconvg_error_bad_metadata_suggested_palette[] =  //
    "iconvg: bad metadata (suggested palette)";
const char iconvg_error_bad_metadata_viewbox[] =  //
    "iconvg: bad metadata (viewbox)";
const char iconvg_error_bad_number[] =  //
    "iconvg: bad number";
const char iconvg_error_bad_path_unfinished[] =  //
    "iconvg: bad path (unfinished)";
const char iconvg_error_bad_styling_opcode[] =  //
    "iconvg: bad styling opcode";

const char iconvg_error_invalid_backend_not_enabled[] =  //
    "iconvg: invalid backend (not enabled)";
const char iconvg_error_invalid_constructor_argument[] =  //
    "iconvg: invalid constructor argument";
const char iconvg_error_null_vtable[] =  //
    "iconvg: null vtable";
const char iconvg_error_unsupported_vtable[] =  //
    "iconvg: unsupported vtable";

const char iconvg_private_internal_error_unreachable[] =  //
    "iconvg: internal error: unreachable";

// ----

bool  //
iconvg_error_is_file_format_error(const char* err_msg) {
  return (err_msg == iconvg_error_bad_color) ||
         (err_msg == iconvg_error_bad_coordinate) ||
         (err_msg == iconvg_error_bad_drawing_opcode) ||
         (err_msg == iconvg_error_bad_magic_identifier) ||
         (err_msg == iconvg_error_bad_metadata) ||
         (err_msg == iconvg_error_bad_metadata_id_order) ||
         (err_msg == iconvg_error_bad_metadata_suggested_palette) ||
         (err_msg == iconvg_error_bad_metadata_viewbox) ||
         (err_msg == iconvg_error_bad_number) ||
         (err_msg == iconvg_error_bad_path_unfinished) ||
         (err_msg == iconvg_error_bad_styling_opcode);
}

// -------------------------------- #include "./paint.c"

bool  //
iconvg_paint__is_flat_color(const iconvg_paint* self) {
  if (!self) {
    return true;
  }
  const uint8_t* rgba = &self->paint_rgba[0];
  return (rgba[0] <= rgba[3]) &&  //
         (rgba[1] <= rgba[3]) &&  //
         (rgba[2] <= rgba[3]);
}

iconvg_nonpremul_color  //
iconvg_paint__flat_color_as_nonpremul_color(const iconvg_paint* self) {
  iconvg_nonpremul_color k;
  if (!self || (self->paint_rgba[3] == 0x00)) {
    memset(&k.rgba[0], 0, 4);
  } else if (self->paint_rgba[3] == 0xFF) {
    memcpy(&k.rgba[0], &self->paint_rgba[0], 4);
  } else {
    uint32_t a = self->paint_rgba[3];
    k.rgba[0] = ((uint8_t)(((uint32_t)(self->paint_rgba[0])) * 0xFF / a));
    k.rgba[1] = ((uint8_t)(((uint32_t)(self->paint_rgba[1])) * 0xFF / a));
    k.rgba[2] = ((uint8_t)(((uint32_t)(self->paint_rgba[2])) * 0xFF / a));
    k.rgba[3] = ((uint8_t)a);
  }
  return k;
}

iconvg_premul_color  //
iconvg_paint__flat_color_as_premul_color(const iconvg_paint* self) {
  iconvg_premul_color k;
  if (!self) {
    memset(&k.rgba[0], 0, 4);
  } else {
    memcpy(&k.rgba[0], &self->paint_rgba[0], 4);
  }
  return k;
}

// -------------------------------- #include "./rectangle.c"

float  //
iconvg_rectangle_f32__width(const iconvg_rectangle_f32* self) {
  // Note that max_x or min_x may be NaN.
  if (self && (self->max_x > self->min_x)) {
    return self->max_x - self->min_x;
  }
  return 0.0f;
}

float  //
iconvg_rectangle_f32__height(const iconvg_rectangle_f32* self) {
  // Note that max_y or min_y may be NaN.
  if (self && (self->max_y > self->min_y)) {
    return self->max_y - self->min_y;
  }
  return 0.0f;
}

#endif  // ICONVG_IMPLEMENTATION

#endif  // ICONVG_INCLUDE_GUARD
