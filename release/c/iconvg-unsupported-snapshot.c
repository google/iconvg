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

// -------------------------------- #include "./aaa_public.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// ----

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

// ----

// Public API Index.
//
// Functions (-):
//   - iconvg_decode
//   - iconvg_decode_viewbox
//   - iconvg_error_is_file_format_error
//
// Data structures (-), their constructors (*) and their methods (+):
//   - iconvg_canvas
//           * iconvg::canvas__make_skia
//           * iconvg_canvas__make_broken
//           * iconvg_canvas__make_cairo
//           * iconvg_canvas__make_debug
//           * iconvg_canvas__make_skia
//       + iconvg_canvas__does_nothing
//   - iconvg_canvas_vtable
//   - iconvg_decode_options
//           * iconvg_decode_options__make_ffv1
//   - iconvg_matrix_2x3_f64
//           * iconvg_matrix_2x3_f64__make
//       + iconvg_matrix_2x3_f64__determinant
//       + iconvg_matrix_2x3_f64__inverse
//       + iconvg_matrix_2x3_f64__override_second_row
//   - iconvg_nonpremul_color
//   - iconvg_optional_i64
//           * iconvg_optional_i64__make_none
//           * iconvg_optional_i64__make_some
//   - iconvg_paint
//       + iconvg_paint__flat_color_as_nonpremul_color
//       + iconvg_paint__flat_color_as_premul_color
//       + iconvg_paint__gradient_number_of_stops
//       + iconvg_paint__gradient_spread
//       + iconvg_paint__gradient_stop_color_as_nonpremul_color
//       + iconvg_paint__gradient_stop_color_as_premul_color
//       + iconvg_paint__gradient_stop_offset
//       + iconvg_paint__gradient_transformation_matrix
//       + iconvg_paint__type
//   - iconvg_palette
//   - iconvg_premul_color
//   - iconvg_rectangle_f32
//           * iconvg_rectangle_f32__make
//       + iconvg_rectangle_f32__height_f64
//       + iconvg_rectangle_f32__is_finite_and_not_empty
//       + iconvg_rectangle_f32__width_f64
//
// Enumerations (-), their constructors (*) and their values (=):
//   - iconvg_gradient_spread
//       = ICONVG_GRADIENT_SPREAD__NONE
//       = ICONVG_GRADIENT_SPREAD__PAD
//       = ICONVG_GRADIENT_SPREAD__REFLECT
//       = ICONVG_GRADIENT_SPREAD__REPEAT
//   - iconvg_paint_type
//       = ICONVG_PAINT_TYPE__FLAT_COLOR
//       = ICONVG_PAINT_TYPE__INVALID
//       = ICONVG_PAINT_TYPE__LINEAR_GRADIENT
//       = ICONVG_PAINT_TYPE__RADIAL_GRADIENT
//
// Other globals (-):
//   - iconvg_error_bad_color
//   - iconvg_error_bad_coordinate
//   - iconvg_error_bad_drawing_opcode
//   - iconvg_error_bad_magic_identifier
//   - iconvg_error_bad_metadata
//   - iconvg_error_bad_metadata_id_order
//   - iconvg_error_bad_metadata_suggested_palette
//   - iconvg_error_bad_metadata_viewbox
//   - iconvg_error_bad_number
//   - iconvg_error_bad_path_unfinished
//   - iconvg_error_bad_styling_opcode
//   - iconvg_error_invalid_backend_not_enabled
//   - iconvg_error_invalid_constructor_argument
//   - iconvg_error_invalid_paint_type
//   - iconvg_error_system_failure_out_of_memory
//   - iconvg_error_unsupported_vtable

// ----

// Some terse comments below contain the U+00B6 PILCROW SIGN (¶), indicating
// that the annotated function, type, etc is part of the public API. The
// pilcrow is followed by the library version (e.g. ¶0.2 or ¶1.0.56) that the
// annotated item has debuted in or will debut in.
//
// Some structs may grow in size across releases (and passed by pointer),
// provided that the first field holds the sizeof that struct and that new
// versions only add fields, never remove or otherwise change existing fields.
// "The fields above are ¶etc" pilcrow comments within such structs annotate
// which fields were added in which versions.

// ----

// Functions that return a "const char*" typically use that to denote success
// (returning NULL) or failure (returning non-NULL). On failure, that C string
// is a human-readable but non-localized error message. It can also be compared
// (by the == operator, not just by strcmp) to an iconvg_error_etc constant.
//
// bad_etc indicates a file format error. The source bytes are not IconVG.
//
// system_failure_etc indicates a system or resource issue, such as running out
// of memory or file descriptors.
//
// Other errors (invalid_etc, null_etc, unsupported_etc) are programming errors
// instead of file format errors.

extern const char iconvg_error_bad_color[];                       // ¶0.1
extern const char iconvg_error_bad_coordinate[];                  // ¶0.1
extern const char iconvg_error_bad_drawing_opcode[];              // ¶0.1
extern const char iconvg_error_bad_magic_identifier[];            // ¶0.1
extern const char iconvg_error_bad_metadata[];                    // ¶0.1
extern const char iconvg_error_bad_metadata_id_order[];           // ¶0.1
extern const char iconvg_error_bad_metadata_suggested_palette[];  // ¶0.1
extern const char iconvg_error_bad_metadata_viewbox[];            // ¶0.1
extern const char iconvg_error_bad_number[];                      // ¶0.1
extern const char iconvg_error_bad_path_unfinished[];             // ¶0.1
extern const char iconvg_error_bad_styling_opcode[];              // ¶0.1

extern const char iconvg_error_system_failure_out_of_memory[];  // ¶0.1

extern const char iconvg_error_invalid_backend_not_enabled[];   // ¶0.1
extern const char iconvg_error_invalid_constructor_argument[];  // ¶0.1
extern const char iconvg_error_invalid_paint_type[];            // ¶0.1
extern const char iconvg_error_unsupported_vtable[];            // ¶0.1

// ----

// iconvg_rectangle_f32 is an axis-aligned rectangle with float32 coordinates.
//
// It is valid for a minimum coordinate to be greater than or equal to the
// corresponding maximum, or for any coordinate to be NaN, in which case the
// rectangle is empty. There are multiple ways to represent an empty rectangle
// but the canonical representation has all fields set to positive zero.
typedef struct iconvg_rectangle_f32_struct {
  float min_x;
  float min_y;
  float max_x;
  float max_y;
} iconvg_rectangle_f32;  // ¶0.1

// iconvg_rectangle_f32__make is an iconvg_rectangle_f32 constructor.
static inline iconvg_rectangle_f32  //
iconvg_rectangle_f32__make(         // ¶0.1
    float min_x,
    float min_y,
    float max_x,
    float max_y) {
  iconvg_rectangle_f32 r;
  r.min_x = min_x;
  r.min_y = min_y;
  r.max_x = max_x;
  r.max_y = max_y;
  return r;
}

// ----

// iconvg_optional_i64 is the C equivalent of C++'s std::optional<int64_t>.
typedef struct iconvg_optional_i64_struct {
  int64_t value;
  bool has_value;
} iconvg_optional_i64;  // ¶0.1

static inline iconvg_optional_i64  //
iconvg_optional_i64__make_none(    // ¶0.1
) {
  iconvg_optional_i64 o;
  o.value = 0;
  o.has_value = false;
  return o;
}

static inline iconvg_optional_i64  //
iconvg_optional_i64__make_some(    // ¶0.1
    int64_t value) {
  iconvg_optional_i64 o;
  o.value = value;
  o.has_value = true;
  return o;
}

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
} iconvg_nonpremul_color;  // ¶0.1

// iconvg_premul_color is an alpha-premultiplied RGBA color. Alpha-
// premultiplication means that {0x00, 0xC0, 0x00, 0xC0} represents a
// 75%-opaque, fully saturated green.
typedef struct iconvg_premul_color_struct {
  uint8_t rgba[4];
} iconvg_premul_color;  // ¶0.1

// iconvg_palette is a list of 64 alpha-premultiplied RGBA colors.
typedef struct iconvg_palette_struct {
  iconvg_premul_color colors[64];
} iconvg_palette;  // ¶0.1

// ----

typedef enum iconvg_paint_type_enum {
  ICONVG_PAINT_TYPE__INVALID = 0,          // ¶0.1
  ICONVG_PAINT_TYPE__FLAT_COLOR = 1,       // ¶0.1
  ICONVG_PAINT_TYPE__LINEAR_GRADIENT = 2,  // ¶0.1
  ICONVG_PAINT_TYPE__RADIAL_GRADIENT = 3,  // ¶0.1
} iconvg_paint_type;                       // ¶0.1

typedef enum iconvg_gradient_spread {
  ICONVG_GRADIENT_SPREAD__NONE = 0,     // ¶0.1
  ICONVG_GRADIENT_SPREAD__PAD = 1,      // ¶0.1
  ICONVG_GRADIENT_SPREAD__REFLECT = 2,  // ¶0.1
  ICONVG_GRADIENT_SPREAD__REPEAT = 3,   // ¶0.1
} iconvg_gradient_spread;               // ¶0.1

struct iconvg_paint_struct;

// iconvg_paint is an opaque data structure passed to iconvg_canvas_vtable's
// paint method.
typedef struct iconvg_paint_struct iconvg_paint;  // ¶0.1

// ----

// iconvg_matrix_2x3_f64 is an affine transformation matrix. The elements are
// given in row-major order:
//
//   elems[0][0]  elems[0][1]  elems[0][2]
//   elems[1][0]  elems[1][1]  elems[1][2]
//
// Matrix multiplication transforms (old_x, old_y) to produce (new_x, new_y):
//
//   new_x = (old_x * elems[0][0]) + (old_y * elems[0][1]) + elems[0][2]
//   new_y = (old_x * elems[1][0]) + (old_y * elems[1][1]) + elems[1][2]
//
// The 2x3 matrix is equivalent to a 3x3 matrix whose bottom row is [0, 0, 1].
// The 3x3 form works on 3-element vectors [x, y, 1].
typedef struct iconvg_matrix_2x3_f64_struct {
  double elems[2][3];
} iconvg_matrix_2x3_f64;  // ¶0.1

// iconvg_matrix_2x3_f64__make is an iconvg_matrix_2x3_f64 constructor.
static inline iconvg_matrix_2x3_f64  //
iconvg_matrix_2x3_f64__make(         // ¶0.1
    double elems00,
    double elems01,
    double elems02,
    double elems10,
    double elems11,
    double elems12) {
  iconvg_matrix_2x3_f64 m;
  m.elems[0][0] = elems00;
  m.elems[0][1] = elems01;
  m.elems[0][2] = elems02;
  m.elems[1][0] = elems10;
  m.elems[1][1] = elems11;
  m.elems[1][2] = elems12;
  return m;
}

// iconvg_matrix_2x3_f64__determinant returns self's determinant.
static inline double                 //
iconvg_matrix_2x3_f64__determinant(  // ¶0.1
    iconvg_matrix_2x3_f64* self) {
  if (!self) {
    return 0;
  }
  return (self->elems[0][0] * self->elems[1][1]) -
         (self->elems[0][1] * self->elems[1][0]);
}

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

  // height_in_pixels, if it has_value, is the rasterization height in pixels,
  // which can affect whether IconVG paths meet Level of Detail thresholds.
  //
  // If its has_value is false then the height (in pixels) is set to the height
  // (in dst coordinate space units) of the dst_rect argument to iconvg_decode.
  iconvg_optional_i64 height_in_pixels;

  // palette, if non-NULL, is the custom palette used for rendering. If NULL,
  // the IconVG file's suggested palette is used instead.
  iconvg_palette* palette;

  // The fields above are ¶0.1
} iconvg_decode_options;  // ¶0.1

// iconvg_decode_options__make_ffv1 returns an iconvg_decode_options suitable
// for FFV (file format version) 1.
static inline iconvg_decode_options  //
iconvg_decode_options__make_ffv1(    // ¶0.1
    iconvg_palette* palette) {
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
// iconvg_canvas_vtable types. Only that iconvg_canvas__make_etc creates a
// canvas and the iconvg_canvas__etc methods take a canvas as an argument.

struct iconvg_canvas_struct;

typedef struct iconvg_canvas_vtable_struct {
  size_t sizeof__iconvg_canvas_vtable;
  const char* (*begin_decode)(struct iconvg_canvas_struct* c,
                              iconvg_rectangle_f32 dst_rect);
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
  const char* (*on_metadata_viewbox)(struct iconvg_canvas_struct* c,
                                     iconvg_rectangle_f32 viewbox);
  const char* (*on_metadata_suggested_palette)(
      struct iconvg_canvas_struct* c,
      const iconvg_palette* suggested_palette);

  // The fields above are ¶0.1
} iconvg_canvas_vtable;  // ¶0.1

typedef struct iconvg_canvas_struct {
  // vtable defines what 'sub-class' we have.
  const iconvg_canvas_vtable* vtable;

  // context_etc semantics depend on the 'sub-class' and should be considered
  // private implementation details. For built-in 'sub-classes', as returned by
  // the library's iconvg_canvas__make_etc functions, users should not read or
  // write these fields directly and their semantics may change between minor
  // library releases.
  void* context_nonconst_ptr0;
  void* context_nonconst_ptr1;
  const void* context_const_ptr;
  size_t context_extra;
} iconvg_canvas;  // ¶0.1

// ----

#ifdef __cplusplus
extern "C" {
#endif

// iconvg_error_is_file_format_error returns whether err_msg is one of the
// built-in iconvg_error_bad_etc constants.
bool                                //
iconvg_error_is_file_format_error(  // ¶0.1
    const char* err_msg);

// ----

// iconvg_canvas__make_broken returns an iconvg_canvas whose callbacks all do
// nothing other than return err_msg.
//
// If err_msg is NULL then all canvas methods are no-op successes (returning a
// NULL error message).
//
// If err_msg is non-NULL then all canvas methods are no-op failures (returning
// the err_msg argument).
iconvg_canvas                //
iconvg_canvas__make_broken(  // ¶0.1
    const char* err_msg);

// iconvg_canvas__make_debug returns an iconvg_canvas that logs vtable calls to
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
iconvg_canvas               //
iconvg_canvas__make_debug(  // ¶0.1
    FILE* f,
    const char* message_prefix,
    iconvg_canvas* wrapped);

// iconvg_canvas__does_nothing returns whether self is NULL or *self is
// zero-valued or broken. Other canvas values are presumed to do something.
// Zero-valued means the result of "iconvg_canvas c = {0}". Broken means the
// result of "iconvg_canvas c = iconvg_canvas__make_broken(err_msg)".
//
// Note that do-nothing canvases are still usable. You can pass them to
// functions like iconvg_decode and iconvg_canvas__make_debug.
//
// A NULL or zero-valued canvas means that all canvas methods are no-op
// successes (returning a NULL error message).
//
// A broken canvas means that all canvas methods are no-op successes or
// failures (depending on the NULL-ness of the iconvg_canvas__make_broken
// err_msg argument).
bool                          //
iconvg_canvas__does_nothing(  // ¶0.1
    const iconvg_canvas* self);

// ----

typedef struct _cairo cairo_t;

// iconvg_canvas__make_cairo returns an iconvg_canvas that is backed by the
// Cairo graphics library, if the ICONVG_CONFIG__ENABLE_CAIRO_BACKEND macro
// was defined when the IconVG library was built.
//
// If that macro was not defined then the returned value will be broken (with
// iconvg_error_invalid_backend_not_enabled).
//
// If cr is NULL then the returned value will be broken (with
// iconvg_error_invalid_constructor_argument).
iconvg_canvas               //
iconvg_canvas__make_cairo(  // ¶0.1
    cairo_t* cr);

// ----

typedef struct sk_canvas_t sk_canvas_t;

// iconvg_canvas__make_skia returns an iconvg_canvas that is backed by the Skia
// graphics library, if the ICONVG_CONFIG__ENABLE_SKIA_BACKEND macro was
// defined when the IconVG library was built.
//
// If that macro was not defined then the returned value will be broken (with
// iconvg_error_invalid_backend_not_enabled).
//
// If sc is NULL then the returned value will be broken (with
// iconvg_error_invalid_constructor_argument).
iconvg_canvas              //
iconvg_canvas__make_skia(  // ¶0.1
    sk_canvas_t* sc);

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
const char*     //
iconvg_decode(  // ¶0.1
    iconvg_canvas* dst_canvas,
    iconvg_rectangle_f32 dst_rect,
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
const char*             //
iconvg_decode_viewbox(  // ¶0.1
    iconvg_rectangle_f32* dst_viewbox,
    const uint8_t* src_ptr,
    size_t src_len);

// ----

// iconvg_paint__type returns what type of paint self is.
iconvg_paint_type    //
iconvg_paint__type(  // ¶0.1
    const iconvg_paint* self);

// iconvg_paint__flat_color_as_nonpremul_color returns self's color (as non-
// alpha-premultiplied), assuming that self is a flat color.
//
// If self is not a flat color then the result may be a non-sensical color.
iconvg_nonpremul_color                        //
iconvg_paint__flat_color_as_nonpremul_color(  // ¶0.1
    const iconvg_paint* self);

// iconvg_paint__flat_color_as_premul_color returns self's color (as alpha-
// premultiplied), assuming that self is a flat color.
//
// If self is not a flat color then the result may be a non-sensical color.
iconvg_premul_color                        //
iconvg_paint__flat_color_as_premul_color(  // ¶0.1
    const iconvg_paint* self);

// iconvg_gradient_spread returns how self is painted for offsets outside of
// the 0.0 ..= 1.0 range.
//
// If self is not a gradient then the result will still be a valid enum value
// but otherwise non-sensical.
iconvg_gradient_spread          //
iconvg_paint__gradient_spread(  // ¶0.1
    const iconvg_paint* self);

// iconvg_paint__gradient_number_of_stops returns self's number of gradient
// stops, also known as N in sibling functions' documentation. The number will
// be in the range 0 ..= 63 inclusive.
//
// If self is not a gradient then the result will still be less than 64 but
// otherwise non-sensical.
uint32_t                                 //
iconvg_paint__gradient_number_of_stops(  // ¶0.1
    const iconvg_paint* self);

// iconvg_paint__gradient_stop_color_as_premul_color returns the color (as non-
// alpha-premultiplied) of the I'th gradient stop, if I < N, where I =
// which_stop and N is the result of iconvg_paint__gradient_number_of_stops.
//
// If self is not a gradient, or if I >= N, then the result may be a
// non-sensical color.
iconvg_nonpremul_color                                 //
iconvg_paint__gradient_stop_color_as_nonpremul_color(  // ¶0.1
    const iconvg_paint* self,
    uint32_t which_stop);

// iconvg_paint__gradient_stop_color_as_premul_color returns the color (as
// alpha-premultiplied) of the I'th gradient stop, if I < N, where I =
// which_stop and N is the result of iconvg_paint__gradient_number_of_stops.
//
// If self is not a gradient, or if I >= N, then the result may be a
// non-sensical color.
iconvg_premul_color                                 //
iconvg_paint__gradient_stop_color_as_premul_color(  // ¶0.1
    const iconvg_paint* self,
    uint32_t which_stop);

// iconvg_paint__gradient_stop_offset returns the offset (in the range 0.0 ..=
// 1.0 inclusive) of the I'th gradient stop, if I < N, where I = which_stop and
// N is the result of iconvg_paint__gradient_number_of_stops.
//
// If self is not a gradient, or if I >= N, then the result may be a
// non-sensical number.
float                                //
iconvg_paint__gradient_stop_offset(  // ¶0.1
    const iconvg_paint* self,
    uint32_t which_stop);

// iconvg_paint__gradient_transformation_matrix returns the affine
// transformation matrix that converts from dst coordinate space (also known as
// user or canvas coordinate space) to pattern coordinate space (also known as
// paint or gradient coordinate space).
//
// Pattern coordinate space is where linear gradients always range from x=0 to
// x=1 and radial gradients are always center=(0,0) and radius=1.
//
// If self is not a gradient then the result may be non-sensical.
iconvg_matrix_2x3_f64                          //
iconvg_paint__gradient_transformation_matrix(  // ¶0.1
    const iconvg_paint* self);

// ----

// iconvg_matrix_2x3_f64__inverse returns self's inverse.
iconvg_matrix_2x3_f64            //
iconvg_matrix_2x3_f64__inverse(  // ¶0.1
    iconvg_matrix_2x3_f64* self);

// iconvg_matrix_2x3_f64__override_second_row sets self's second row's values
// such that self has a non-zero determinant (and is therefore invertible). The
// second row is the bottom row of the 2x3 matrix, which is also the middle row
// of the equivalent 3x3 matrix after adding an implicit [0, 0, 1] third row.
//
// If self->elems[0][0] and self->elems[0][1] are both zero then this function
// might also change the first row, again to produce a non-zero determinant.
//
// IconVG linear gradients range from x=0 to x=1 in pattern space, independent
// of y. The second row therefore doesn't matter (because it's "independent of
// y") and can be [0, 0, 0] in the IconVG file format. However, some other
// graphics libraries need the transformation matrix to be invertible.
void                                         //
iconvg_matrix_2x3_f64__override_second_row(  // ¶0.1
    iconvg_matrix_2x3_f64* self);

// ----

// iconvg_rectangle_f32__is_finite_and_not_empty returns whether self is finite
// (none of its fields are infinite) and non-empty.
bool                                            //
iconvg_rectangle_f32__is_finite_and_not_empty(  // ¶0.1
    const iconvg_rectangle_f32* self);

// iconvg_rectangle_f32__width_f64 returns self's width as an f64.
double                            //
iconvg_rectangle_f32__width_f64(  // ¶0.1
    const iconvg_rectangle_f32* self);

// iconvg_rectangle_f32__height returns self's height as an f64.
double                             //
iconvg_rectangle_f32__height_f64(  // ¶0.1
    const iconvg_rectangle_f32* self);

#ifdef __cplusplus
}  // extern "C"

class SkCanvas;

namespace iconvg {

// iconvg::canvas__make_skia is equivalent to iconvg_canvas__make_skia except
// that it takes a SkCanvas* argument (part of Skia's C++ API) instead of a
// sk_canvas_t* argument (part of Skia's C API).
static inline iconvg_canvas  //
canvas__make_skia(           // ¶0.1
    SkCanvas* sc) {
  return iconvg_canvas__make_skia(reinterpret_cast<sk_canvas_t*>(sc));
}

}  // namespace iconvg
#endif

#ifdef ICONVG_IMPLEMENTATION
// -------------------------------- #include "./aaa_private.h"

#include <math.h>
#include <string.h>

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
  p[0] = (uint8_t)(x >> 0);
  p[1] = (uint8_t)(x >> 8);
  p[2] = (uint8_t)(x >> 16);
  p[3] = (uint8_t)(x >> 24);
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
  iconvg_rectangle_f32 viewbox;
  int64_t height_in_pixels;
  uint8_t paint_rgba[4];
  iconvg_palette custom_palette;
  iconvg_palette creg;
  float nreg[64];

  // Scale and bias convert between dst coordinates (what this library calls
  // user or canvas coordinate space) and src coordinates (what this library
  // calls viewbox or graphic coordinate space). When converting from p to q:
  //
  //   q_x = (p_x * p2q_scale_x) + p2q_bias_x
  //   q_y = (p_y * p2q_scale_y) + p2q_bias_y
  //
  // For example, an IconVG file might declare its viewbox ranging from -32 to
  // +32 along the X axis, in ideal (not pixel) space. The user might rasterize
  // this on screen from x=400 to x=500, 100 pixels wide. This corresponds to
  // s2d_scale_x = (100 / (+32 - -32)) = 1.5625 and s2d_bias_x = 450, because:
  //
  //   400 = ((-32) * 1.5625) + 450
  //   500 = ((+32) * 1.5625) + 450

  double s2d_scale_x;
  double s2d_bias_x;
  double s2d_scale_y;
  double s2d_bias_y;

  double d2s_scale_x;
  double d2s_bias_x;
  double d2s_scale_y;
  double d2s_bias_y;
};

// ----

const char*  //
iconvg_private_path_arc_to(iconvg_canvas* c,
                           double scale_x,
                           double bias_x,
                           double scale_y,
                           double bias_y,
                           float initial_x,
                           float initial_y,
                           float radius_x,
                           float radius_y,
                           float x_axis_rotation,
                           bool large_arc,
                           bool sweep,
                           float final_x,
                           float final_y);

// -------------------------------- #include "./arc.c"

// iconvg_private_angle returns the angle between two vectors u and v.
static inline double  //
iconvg_private_angle(double ux, double uy, double vx, double vy) {
  const double pi = 3.1415926535897932384626433832795028841972;  // π = τ/2

  double u_norm = sqrt((ux * ux) + (uy * uy));
  double v_norm = sqrt((vx * vx) + (vy * vy));
  double norm = u_norm * v_norm;
  double cosine = (ux * vx + uy * vy) / norm;
  double ret = 0.0;
  if (cosine <= -1.0) {
    ret = pi;
  } else if (cosine >= +1.0) {
    ret = 0.0;
  } else {
    ret = acos(cosine);
  }
  if ((ux * vy) < (uy * vx)) {
    return -ret;
  }
  return +ret;
}

static inline const char*  //
iconvg_private_path_arc_segment_to(iconvg_canvas* c,
                                   double scale_x,
                                   double bias_x,
                                   double scale_y,
                                   double bias_y,
                                   double cx,
                                   double cy,
                                   double theta1,
                                   double theta2,
                                   double rx,
                                   double ry,
                                   double cos_phi,
                                   double sin_phi) {
  double half_delta_theta = (theta2 - theta1) * 0.5;
  double q = sin(half_delta_theta * 0.5);
  double t = (8 * q * q) / (3 * sin(half_delta_theta));
  double cos1 = cos(theta1);
  double sin1 = sin(theta1);
  double cos2 = cos(theta2);
  double sin2 = sin(theta2);

  double ix1 = rx * (+cos1 - (t * sin1));
  double iy1 = ry * (+sin1 + (t * cos1));
  double ix2 = rx * (+cos2 + (t * sin2));
  double iy2 = ry * (+sin2 - (t * cos2));
  double ix3 = rx * (+cos2);
  double iy3 = ry * (+sin2);

  double jx1 = cx + (cos_phi * ix1) - (sin_phi * iy1);
  double jy1 = cy + (sin_phi * ix1) + (cos_phi * iy1);
  double jx2 = cx + (cos_phi * ix2) - (sin_phi * iy2);
  double jy2 = cy + (sin_phi * ix2) + (cos_phi * iy2);
  double jx3 = cx + (cos_phi * ix3) - (sin_phi * iy3);
  double jy3 = cy + (sin_phi * ix3) + (cos_phi * iy3);

  return (*c->vtable->path_cube_to)(c,                         //
                                    (jx1 * scale_x) + bias_x,  //
                                    (jy1 * scale_y) + bias_y,  //
                                    (jx2 * scale_x) + bias_x,  //
                                    (jy2 * scale_y) + bias_y,  //
                                    (jx3 * scale_x) + bias_x,  //
                                    (jy3 * scale_y) + bias_y);
}

const char*  //
iconvg_private_path_arc_to(iconvg_canvas* c,
                           double scale_x,
                           double bias_x,
                           double scale_y,
                           double bias_y,
                           float initial_x,
                           float initial_y,
                           float radius_x,
                           float radius_y,
                           float x_axis_rotation,
                           bool large_arc,
                           bool sweep,
                           float final_x,
                           float final_y) {
  const double pi = 3.1415926535897932384626433832795028841972;   // π = τ/2
  const double tau = 6.2831853071795864769252867665590057683943;  // τ = 2*π

  // "Conversion from endpoint to center parameterization" per
  // https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
  //
  // There seems to be a bug in the spec's "implementation notes". Actual
  // implementations, such as those below, do something slightly different
  // (marked with a †).
  //
  // https://gitlab.gnome.org/GNOME/librsvg/-/blob/8d13da3c5f5b2442b980d38d469923f926c27c11/src/path_builder.rs
  //
  // http://svn.apache.org/repos/asf/xmlgraphics/batik/branches/svg11/sources/org/apache/batik/ext/awt/geom/ExtendedGeneralPath.java
  //
  // https://github.com/blackears/svgSalamander/blob/a679f7cbd14703d95b61878f107bc52688a9e91d/svg-core/src/main/java/com/kitfox/svg/pathcmd/Arc.java
  //
  // https://github.com/millermedeiros/SVGParser/blob/e7f80a0810f6e2abe0db5e8e6a004c4cfd7f83ae/com/millermedeiros/geom/SVGArc.as

  // (†) The abs isn't part of the spec. Neither is checking that rx and ry are
  // non-zero (and non-NaN).
  double rx = fabs((double)radius_x);
  double ry = fabs((double)radius_y);
  if (!(rx > 0) || !(ry > 0)) {
    return (*c->vtable->path_line_to)(c,                             //
                                      (final_x * scale_x) + bias_x,  //
                                      (final_y * scale_y) + bias_y);
  }

  double x1 = (double)initial_x;
  double y1 = (double)initial_y;
  double x2 = (double)final_x;
  double y2 = (double)final_y;
  double phi = tau * ((double)x_axis_rotation);

  // Step 1: Compute (x1′, y1′)

  double half_dx = (x1 - x2) / 2;
  double half_dy = (y1 - y2) / 2;
  double cos_phi = cos(phi);
  double sin_phi = sin(phi);
  double x1_prime = +(cos_phi * half_dx) + (sin_phi * half_dy);
  double y1_prime = -(sin_phi * half_dx) + (cos_phi * half_dy);

  // Step 2: Compute (cx′, cy′)

  double rx_sq = rx * rx;
  double ry_sq = ry * ry;
  double x1_prime_sq = x1_prime * x1_prime;
  double y1_prime_sq = y1_prime * y1_prime;

  // (†) Check that the radii are large enough.
  double radii_check = (x1_prime_sq / rx_sq) + (y1_prime_sq / ry_sq);
  if (radii_check > 1) {
    double s = sqrt(radii_check);
    rx *= s;
    ry *= s;
    rx_sq = rx * rx;
    ry_sq = ry * ry;
  }

  double denom = (rx_sq * y1_prime_sq) + (ry_sq * x1_prime_sq);
  double step2 = 0.0;
  double a = ((rx_sq * ry_sq) / denom) - 1.0;
  if (a > 0.0) {
    step2 = sqrt(a);
  }
  if (large_arc == sweep) {
    step2 = -step2;
  }
  double cx_prime = +(step2 * rx * y1_prime) / ry;
  double cy_prime = -(step2 * ry * x1_prime) / rx;

  // Step 3: Compute (cx, cy) from (cx′, cy′)

  double cx = +(cos_phi * cx_prime) - (sin_phi * cy_prime) + ((x1 + x2) / 2);
  double cy = +(sin_phi * cx_prime) + (cos_phi * cy_prime) + ((y1 + y2) / 2);

  // Step 4: Compute θ1 and Δθ

  double ax = (+x1_prime - cx_prime) / rx;
  double ay = (+y1_prime - cy_prime) / ry;
  double bx = (-x1_prime - cx_prime) / rx;
  double by = (-y1_prime - cy_prime) / ry;
  double theta1 = iconvg_private_angle(1.0, 0.0, ax, ay);
  double delta_theta = iconvg_private_angle(ax, ay, bx, by);
  if (sweep) {
    if (delta_theta < 0.0) {
      delta_theta += tau;
    }
  } else {
    if (delta_theta > 0.0) {
      delta_theta -= tau;
    }
  }

  // This ends the
  // https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
  // algorithm. What follows below is specific to this implementation.

  // We approximate an arc by one or more cubic Bézier curves.
  int n = (int)(ceil(fabs(delta_theta) / ((pi / 2) + 0.001)));
  double inv_n = 1.0 / ((double)n);
  for (int i = 0; i < n; i++) {
    ICONVG_PRIVATE_TRY(iconvg_private_path_arc_segment_to(
        c, scale_x, bias_x, scale_y, bias_y, cx, cy,         //
        theta1 + (delta_theta * ((double)(i + 0)) * inv_n),  //
        theta1 + (delta_theta * ((double)(i + 1)) * inv_n),  //
        rx, ry, cos_phi, sin_phi));
  }
  return NULL;
}

// -------------------------------- #include "./broken.c"

static const char*  //
iconvg_private_broken_canvas__begin_decode(iconvg_canvas* c,
                                           iconvg_rectangle_f32 dst_rect) {
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
        &iconvg_private_broken_canvas__on_metadata_viewbox,
        &iconvg_private_broken_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_canvas__make_broken(const char* err_msg) {
  iconvg_canvas c;
  c.vtable = &iconvg_private_broken_canvas_vtable;
  c.context_nonconst_ptr0 = NULL;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = err_msg;
  c.context_extra = 0;
  return c;
}

bool  //
iconvg_canvas__does_nothing(const iconvg_canvas* self) {
  return !self || (self->vtable == NULL) ||
         (self->vtable == &iconvg_private_broken_canvas_vtable);
}

// -------------------------------- #include "./cairo.c"

#if !defined(ICONVG_CONFIG__ENABLE_CAIRO_BACKEND)

iconvg_canvas  //
iconvg_canvas__make_cairo(cairo_t* cr) {
  return iconvg_canvas__make_broken(iconvg_error_invalid_backend_not_enabled);
}

#else  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND

#include <cairo/cairo.h>

static const cairo_extend_t
    iconvg_private_gradient_spread_as_cairo_extend_t[4] = {
        CAIRO_EXTEND_NONE,     //
        CAIRO_EXTEND_PAD,      //
        CAIRO_EXTEND_REFLECT,  //
        CAIRO_EXTEND_REPEAT    //
};

static inline cairo_matrix_t  //
iconvg_private_matrix_2x3_f64_as_cairo_matrix_t(iconvg_matrix_2x3_f64 i) {
  cairo_matrix_t c;
  c.xx = i.elems[0][0];
  c.xy = i.elems[0][1];
  c.x0 = i.elems[0][2];
  c.yx = i.elems[1][0];
  c.yy = i.elems[1][1];
  c.y0 = i.elems[1][2];
  return c;
}

// iconvg_private_cairo_set_gradient_stops sets the Cairo gradient stop colors
// given the IconVG gradient stop colors.
//
// Unlike SVG, IconVG works solely with premultiplied alpha. In contrast,
// https://lists.freedesktop.org/archives/cairo/2006-June/007203.html says that
// Cairo accepts "both pre- and non-premultiplied colors in different parts of
// the API". Specifically, while CAIRO_FORMAT_ARGB32 is premultiplied, both
// cairo_set_source_rgba and cairo_pattern_add_color_stop_rgba are
// non-premultiplied.
//
// For flat colors, we can simply convert IconVG colors to non-premultiplied
// colors. Gradients are trickier (and hence this function is non-trivial)
// because IconVG interpolation should also happen in premultiplied alpha space
// (but Cairo interpolates in non-premultiplied alpha space). The mathematical
// halfway color between opaque bright red = RGBA(1, 0, 0, 1) and transparent
// black = RGBA(0, 0, 0, 0) is RGBA(½, 0, 0, ½). IconVG (premultiplied alpha)
// semantics are that this is a 50% opaque bright red, not a 50% opaque dark
// red. The halfway point still has 100% Saturation and 100% Value (in the HSV
// Hue Saturation Value sense). It just has smaller alpha.
//
// Some more discussion is at
// https://lists.freedesktop.org/archives/cairo/2021-May/029252.html
static void  //
iconvg_private_cairo_set_gradient_stops(cairo_pattern_t* cp,
                                        const iconvg_paint* p) {
  // foo0 and foo2 are the previous and current gradient stop. Sometimes we
  // need to synthesize additional stops in between them, whose variables are
  // named foo1.
  double offset0 = 0.0;
  double r0 = 0.0;
  double g0 = 0.0;
  double b0 = 0.0;
  double a0 = 0.0;

  uint32_t num_stops = iconvg_paint__gradient_number_of_stops(p);
  for (uint32_t i = 0; i < num_stops; i++) {
    // Calculate offset and color for the current stop.
    double offset2 = iconvg_paint__gradient_stop_offset(p, i);
    iconvg_premul_color k =
        iconvg_paint__gradient_stop_color_as_premul_color(p, i);
    double r2 = k.rgba[0] / 255.0;
    double g2 = k.rgba[1] / 255.0;
    double b2 = k.rgba[2] / 255.0;
    double a2 = k.rgba[3] / 255.0;

    if ((i == 0) ||                      //
        ((a0 == 1.0) && (a2 == 1.0)) ||  //
        ((a0 == 0.0) && (a2 == 0.0))) {
      // If it's the first stop, or if we're interpolating from 100% to 100%
      // opaque or from 0% to 0% opaque, we don't have to worry about
      // premultiplied versus non-premultiplied alpha.
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else if (a0 == 0.0) {
      // If we're blending e.g. from transparent black to (partially) opaque
      // blue, insert "transparent blue" immediately after the previous
      // "transparent black".
      cairo_pattern_add_color_stop_rgba(cp, offset0, r2, g2, b2, 0.0);
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else if (a2 == 0.0) {
      // If we're blending e.g. from (partially) opaque blue to transparent
      // black, insert "transparent blue" immediately before the current
      // "transparent black".
      cairo_pattern_add_color_stop_rgba(cp, offset2, r0, g0, b0, 0.0);
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else {
      // Otherwise, fake "interpolate with premultiplied alpha" by synthesizing
      // n Cairo stops for this 1 IconVG stop. The n stops' colors are
      // calculated explicitly here, in premultiplied alpha space. We then let
      // Cairo do its thing in non-premultiplied alpha space. The difference
      // between n stops (interpolating non-premultiplied) and 1 stop
      // (interpolating premultiplied) will hopefully be imperceivable.
      const int32_t n = 16;
      for (int32_t i = (n - 1); i >= 0; i--) {
        int32_t j = n - i;
        double offset1 = ((i * offset0) + (j * offset2)) / n;
        double r1 = ((i * r0) + (j * r2)) / n;
        double g1 = ((i * g0) + (j * g2)) / n;
        double b1 = ((i * b0) + (j * b2)) / n;
        double a1 = ((i * a0) + (j * a2)) / n;
        if (a1 == 0.0) {
          cairo_pattern_add_color_stop_rgba(cp, offset1, 0, 0, 0, 0);
        } else {
          cairo_pattern_add_color_stop_rgba(cp, offset1,  //
                                            r1 / a1, g1 / a1, b1 / a1, a1);
        }
      }
    }

    // Update offset and color for the previous stop.
    offset0 = offset2;
    r0 = r2;
    g0 = g2;
    b0 = b2;
    a0 = a2;
  }
}

static const char*  //
iconvg_private_cairo_canvas__begin_decode(iconvg_canvas* c,
                                          iconvg_rectangle_f32 dst_rect) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_save(cr);
  cairo_rectangle(cr, dst_rect.min_x, dst_rect.min_y,
                  iconvg_rectangle_f32__width_f64(&dst_rect),
                  iconvg_rectangle_f32__height_f64(&dst_rect));
  cairo_clip(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_restore(cr);
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
  cairo_pattern_t* cp = NULL;
  cairo_matrix_t cm = {0};

  switch (iconvg_paint__type(p)) {
    case ICONVG_PAINT_TYPE__FLAT_COLOR: {
      iconvg_nonpremul_color k = iconvg_paint__flat_color_as_nonpremul_color(p);
      cairo_set_source_rgba(cr, k.rgba[0] / 255.0, k.rgba[1] / 255.0,
                            k.rgba[2] / 255.0, k.rgba[3] / 255.0);
      cairo_fill(cr);
      return NULL;
    }

    case ICONVG_PAINT_TYPE__LINEAR_GRADIENT: {
      iconvg_matrix_2x3_f64 gtm =
          iconvg_paint__gradient_transformation_matrix(p);
      iconvg_matrix_2x3_f64__override_second_row(&gtm);
      cp = cairo_pattern_create_linear(0, 0, 1, 0);
      cm = iconvg_private_matrix_2x3_f64_as_cairo_matrix_t(gtm);
      break;
    }

    case ICONVG_PAINT_TYPE__RADIAL_GRADIENT: {
      iconvg_matrix_2x3_f64 gtm =
          iconvg_paint__gradient_transformation_matrix(p);
      cp = cairo_pattern_create_radial(0, 0, 0, 0, 0, 1);
      cm = iconvg_private_matrix_2x3_f64_as_cairo_matrix_t(gtm);
      break;
    }

    default:
      return iconvg_error_invalid_paint_type;
  }

  cairo_pattern_set_matrix(cp, &cm);
  cairo_pattern_set_extend(cp, iconvg_private_gradient_spread_as_cairo_extend_t
                                   [iconvg_paint__gradient_spread(p)]);
  iconvg_private_cairo_set_gradient_stops(cp, p);
  if (cairo_pattern_status(cp) == CAIRO_STATUS_SUCCESS) {
    cairo_set_source(cr, cp);
  } else {
    // Substitute in a 50% transparent grayish purple so that "something is
    // wrong with the Cairo pattern" is hopefully visible without abandoning
    // the graphic entirely.
    cairo_set_source_rgba(cr, 0.75, 0.25, 0.75, 0.5);
  }

  cairo_fill(cr);
  cairo_pattern_destroy(cp);
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
        &iconvg_private_cairo_canvas__on_metadata_viewbox,
        &iconvg_private_cairo_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_canvas__make_cairo(cairo_t* cr) {
  if (!cr) {
    return iconvg_canvas__make_broken(
        iconvg_error_invalid_constructor_argument);
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

// -------------------------------- #include "./color.c"

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
iconvg_private_debug_canvas__begin_decode(iconvg_canvas* c,
                                          iconvg_rectangle_f32 dst_rect) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_decode({%g, %g, %g, %g})\n",
            ((const char*)(c->context_const_ptr)), dst_rect.min_x,
            dst_rect.min_y, dst_rect.max_x, dst_rect.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_decode)(wrapped, dst_rect);
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
  static const char* spread_names[4] = {"none", "pad", "reflect", "repeat"};

  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    switch (iconvg_paint__type(p)) {
      case ICONVG_PAINT_TYPE__FLAT_COLOR: {
        iconvg_premul_color k = iconvg_paint__flat_color_as_premul_color(p);
        fprintf(f, "%send_drawing(flat_color{%02X:%02X:%02X:%02X})\n",
                ((const char*)(c->context_const_ptr)), ((int)(k.rgba[0])),
                ((int)(k.rgba[1])), ((int)(k.rgba[2])), ((int)(k.rgba[3])));
        break;
      }

      case ICONVG_PAINT_TYPE__LINEAR_GRADIENT: {
        fprintf(f,
                "%send_drawing(linear_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context_const_ptr)),
                ((int)(iconvg_paint__gradient_number_of_stops(p))),
                spread_names[iconvg_paint__gradient_spread(p)]);
        break;
      }

      case ICONVG_PAINT_TYPE__RADIAL_GRADIENT: {
        fprintf(f,
                "%send_drawing(radial_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context_const_ptr)),
                ((int)(iconvg_paint__gradient_number_of_stops(p))),
                spread_names[iconvg_paint__gradient_spread(p)]);
        break;
      }

      case ICONVG_PAINT_TYPE__INVALID:
      default: {
        return iconvg_error_invalid_paint_type;
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
        &iconvg_private_debug_canvas__on_metadata_viewbox,
        &iconvg_private_debug_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_canvas__make_debug(FILE* f,
                          const char* message_prefix,
                          iconvg_canvas* wrapped) {
  if (wrapped && !wrapped->vtable) {
    wrapped = NULL;
  }
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
  if ((self->len < 4) ||         //
      (self->ptr[0] != 0x89) ||  //
      (self->ptr[1] != 0x49) ||  //
      (self->ptr[2] != 0x56) ||  //
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
         iconvg_private_decoder__decode_coordinate_number(self, &dst->max_y) &&
         (-INFINITY < dst->min_x) &&    //
         (dst->min_x <= dst->max_x) &&  //
         (dst->max_x < +INFINITY) &&    //
         (-INFINITY < dst->min_y) &&    //
         (dst->min_y <= dst->max_y) &&  //
         (dst->max_y < +INFINITY);
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
iconvg_private_execute_bytecode(iconvg_canvas* c_arg,
                                iconvg_rectangle_f32 r,
                                iconvg_private_decoder* d,
                                iconvg_paint* state) {
  // adjustments are the ADJ values from the IconVG spec.
  static const uint32_t adjustments[8] = {0, 1, 2, 3, 4, 5, 6, 0};

  iconvg_canvas no_op_canvas = iconvg_canvas__make_broken(NULL);
  iconvg_canvas* c = &no_op_canvas;

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

  double scale_x = +1.0;
  double bias_x = +0.0;
  double scale_y = +1.0;
  double bias_y = +0.0;
  {
    double rw = iconvg_rectangle_f32__width_f64(&r);
    double rh = iconvg_rectangle_f32__height_f64(&r);
    double vw = iconvg_rectangle_f32__width_f64(&state->viewbox);
    double vh = iconvg_rectangle_f32__height_f64(&state->viewbox);
    if ((rw > 0) && (rh > 0) && (vw > 0) && (vh > 0)) {
      scale_x = rw / vw;
      scale_y = rh / vh;
      bias_x = r.min_x - (state->viewbox.min_x * scale_x);
      bias_y = r.min_y - (state->viewbox.min_y * scale_y);
    }
  }
  state->s2d_scale_x = scale_x;
  state->s2d_bias_x = bias_x;
  state->s2d_scale_y = scale_y;
  state->s2d_bias_y = bias_y;
  state->d2s_scale_x = 1.0 / scale_x;
  state->d2s_bias_x = -bias_x * state->d2s_scale_x;
  state->d2s_scale_y = 1.0 / scale_y;
  state->d2s_bias_y = -bias_y * state->d2s_scale_y;

  // sel[0] and sel[1] are the CSEL and NSEL registers.
  uint32_t sel[2] = {0};
  double lod[2];
  lod[0] = 0.0;
  lod[1] = INFINITY;

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
      uint8_t creg_index = (sel[0] - adjustments[opcode & 0x07]) & 0x3F;
      memcpy(&state->paint_rgba, &state->creg.colors[creg_index],
             sizeof(state->paint_rgba));
      if (iconvg_paint__type(state) == ICONVG_PAINT_TYPE__INVALID) {
        return iconvg_error_invalid_paint_type;
      }
      if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
          !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
        return iconvg_error_bad_coordinate;
      }
      double h = (double)state->height_in_pixels;
      c = ((lod[0] <= h) && (h < lod[1])) ? c_arg : &no_op_canvas;
      ICONVG_PRIVATE_TRY((*c->vtable->begin_drawing)(c));
      ICONVG_PRIVATE_TRY(
          (*c->vtable->begin_path)(c,                            //
                                   (curr_x * scale_x) + bias_x,  //
                                   (curr_y * scale_y) + bias_y));
      x1 = curr_x;
      y1 = curr_y;
      goto drawing_mode;

    } else if (opcode < 0xC8) {  // Set Level of Detail bounds.
      float lod0;
      float lod1;
      if (!iconvg_private_decoder__decode_real_number(d, &lod0) ||
          !iconvg_private_decoder__decode_real_number(d, &lod1)) {
        return iconvg_error_bad_number;
      }
      lod[0] = (double)lod0;
      lod[1] = (double)lod1;
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_line_to)(c,                            //
                                         (curr_x * scale_x) + bias_x,  //
                                         (curr_y * scale_y) + bias_y));
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_line_to)(c,                            //
                                         (curr_x * scale_x) + bias_x,  //
                                         (curr_y * scale_y) + bias_y));
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_quad_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y));
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_quad_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y));
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_quad_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y));
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
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_quad_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y));
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
              (*c->vtable->path_cube_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y,  //
                                         (x3 * scale_x) + bias_x,  //
                                         (y3 * scale_y) + bias_y));
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
              (*c->vtable->path_cube_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y,  //
                                         (x3 * scale_x) + bias_x,  //
                                         (y3 * scale_y) + bias_y));
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
              (*c->vtable->path_cube_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y,  //
                                         (x3 * scale_x) + bias_x,  //
                                         (y3 * scale_y) + bias_y));
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
              (*c->vtable->path_cube_to)(c,                        //
                                         (x1 * scale_x) + bias_x,  //
                                         (y1 * scale_y) + bias_y,  //
                                         (x2 * scale_x) + bias_x,  //
                                         (y2 * scale_y) + bias_y,  //
                                         (x3 * scale_x) + bias_x,  //
                                         (y3 * scale_y) + bias_y));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * curr_x) - x2;
          y1 = (2 * curr_y) - y2;
        }
        continue;
      }

      case 0x0C: {  // 'A' mnemonic: absolute arc_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          float x0 = curr_x;
          float y0 = curr_y;
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_zero_to_one_number(d, &x2) ||
              !iconvg_private_decoder__decode_natural_number(d, &flags) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY(iconvg_private_path_arc_to(
              c, scale_x, bias_x, scale_y, bias_y, x0, y0, x1, y1, x2,
              flags & 0x01, flags & 0x02, curr_x, curr_y));
          x1 = curr_x;
          y1 = curr_y;
        }
        continue;
      }

      case 0x0D: {  // 'a' mnemonic: relative arc_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          float x0 = curr_x;
          float y0 = curr_y;
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_zero_to_one_number(d, &x2) ||
              !iconvg_private_decoder__decode_natural_number(d, &flags) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          curr_x += x3;
          curr_y += y3;
          ICONVG_PRIVATE_TRY(iconvg_private_path_arc_to(
              c, scale_x, bias_x, scale_y, bias_y, x0, y0, x1, y1, x2,
              flags & 0x01, flags & 0x02, curr_x, curr_y));
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
        ICONVG_PRIVATE_TRY(
            (*c->vtable->begin_path)(c,                            //
                                     (curr_x * scale_x) + bias_x,  //
                                     (curr_y * scale_y) + bias_y));
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
        ICONVG_PRIVATE_TRY(
            (*c->vtable->begin_path)(c,                            //
                                     (curr_x * scale_x) + bias_x,  //
                                     (curr_y * scale_y) + bias_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE6: {  // 'H' mnemonic: absolute horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY(
            (*c->vtable->path_line_to)(c,                            //
                                       (curr_x * scale_x) + bias_x,  //
                                       (curr_y * scale_y) + bias_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE7: {  // 'h' mnemonic: relative horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &x1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_x += x1;
        ICONVG_PRIVATE_TRY(
            (*c->vtable->path_line_to)(c,                            //
                                       (curr_x * scale_x) + bias_x,  //
                                       (curr_y * scale_y) + bias_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE8: {  // 'V' mnemonic: absolute vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY(
            (*c->vtable->path_line_to)(c,                            //
                                       (curr_x * scale_x) + bias_x,  //
                                       (curr_y * scale_y) + bias_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE9: {  // 'v' mnemonic: relative vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_y += y1;
        ICONVG_PRIVATE_TRY(
            (*c->vtable->path_line_to)(c,                            //
                                       (curr_x * scale_x) + bias_x,  //
                                       (curr_y * scale_y) + bias_y));
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
                      iconvg_rectangle_f32 r,
                      iconvg_private_decoder* d,
                      const iconvg_decode_options* options) {
  iconvg_paint state;
  state.viewbox = iconvg_private_default_viewbox();
  if (options && options->height_in_pixels.has_value) {
    state.height_in_pixels = options->height_in_pixels.value;
  } else {
    double h = iconvg_rectangle_f32__height_f64(&r);
    // The 0x10_0000 = (1 << 20) = 1048576 limit is arbitrary but it's less
    // than MAX_INT32 and also ensures that conversion between integer and
    // float or double is lossless.
    if (h <= 0x100000) {
      state.height_in_pixels = (int64_t)h;
    } else {
      state.height_in_pixels = 0x100000;
    }
  }
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
                                                             &state.viewbox) ||
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

  ICONVG_PRIVATE_TRY((*c->vtable->on_metadata_viewbox)(c, state.viewbox));
  ICONVG_PRIVATE_TRY(
      (*c->vtable->on_metadata_suggested_palette)(c, &state.custom_palette));

  if (options && options->palette) {
    memcpy(&state.custom_palette, options->palette,
           sizeof(state.custom_palette));
  }

  memcpy(&state.creg, &state.custom_palette, sizeof(state.creg));
  memset(&state.nreg[0], 0, sizeof(state.nreg));
  state.s2d_scale_x = +1.0;
  state.s2d_bias_x = +0.0;
  state.s2d_scale_y = +1.0;
  state.s2d_bias_y = +0.0;
  state.d2s_scale_x = +1.0;
  state.d2s_bias_x = +0.0;
  state.d2s_scale_y = +1.0;
  state.d2s_bias_y = +0.0;

  return iconvg_private_execute_bytecode(c, r, d, &state);
}

const char*  //
iconvg_decode(iconvg_canvas* dst_canvas,
              iconvg_rectangle_f32 dst_rect,
              const uint8_t* src_ptr,
              size_t src_len,
              const iconvg_decode_options* options) {
  iconvg_canvas fallback_canvas = iconvg_canvas__make_broken(NULL);
  if (!dst_canvas || !dst_canvas->vtable) {
    dst_canvas = &fallback_canvas;
  }

  if (dst_canvas->vtable->sizeof__iconvg_canvas_vtable !=
      sizeof(iconvg_canvas_vtable)) {
    // If we want to support multiple library versions (with dynamic linking),
    // we could detect older versions here (with smaller vtable sizes) and
    // substitute in an adapter implementation.
    return iconvg_error_unsupported_vtable;
  }

  iconvg_private_decoder d;
  d.ptr = src_ptr;
  d.len = src_len;

  const char* err_msg =
      (*dst_canvas->vtable->begin_decode)(dst_canvas, dst_rect);
  if (!err_msg) {
    err_msg = iconvg_private_decode(dst_canvas, dst_rect, &d, options);
  }
  return (*dst_canvas->vtable->end_decode)(dst_canvas, err_msg, src_len - d.len,
                                           d.len);
}

// -------------------------------- #include "./error.c"

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

const char iconvg_error_system_failure_out_of_memory[] =  //
    "iconvg: system failure: out of memory";

const char iconvg_error_invalid_backend_not_enabled[] =  //
    "iconvg: invalid backend (not enabled)";
const char iconvg_error_invalid_constructor_argument[] =  //
    "iconvg: invalid constructor argument";
const char iconvg_error_invalid_paint_type[] =  //
    "iconvg: invalid paint type";
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

// -------------------------------- #include "./matrix.c"

iconvg_matrix_2x3_f64  //
iconvg_matrix_2x3_f64__inverse(iconvg_matrix_2x3_f64* self) {
  double inv = 1.0 / iconvg_matrix_2x3_f64__determinant(self);
  if (isinf(inv) || isnan(inv)) {
    return iconvg_matrix_2x3_f64__make(1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  }

  // https://ardoris.wordpress.com/2008/07/18/general-formula-for-the-inverse-of-a-3x3-matrix/
  // recalling that self's implicit bottom row is [0, 0, 1].
  double e02 = (self->elems[0][1] * self->elems[1][2]) -
               (self->elems[0][2] * self->elems[1][1]);
  double e12 = (self->elems[0][0] * self->elems[1][2]) -
               (self->elems[0][2] * self->elems[1][0]);
  return iconvg_matrix_2x3_f64__make(+inv * self->elems[1][1],  //
                                     -inv * self->elems[0][1],  //
                                     +inv * e02,                //
                                     -inv * self->elems[1][0],  //
                                     +inv * self->elems[0][0],  //
                                     -inv * e12);               //
}

void  //
iconvg_matrix_2x3_f64__override_second_row(iconvg_matrix_2x3_f64* self) {
  if (!self) {
    return;
  }
  if (self->elems[0][0] != 0.0) {
    self->elems[1][0] = 0.0;
    self->elems[1][1] = 1.0;
  } else if (self->elems[0][1] != 0.0) {
    self->elems[1][0] = 1.0;
    self->elems[1][1] = 0.0;
  } else {
    // 1e-10 is arbitrary but very small and squaring it still gives
    // something larger than FLT_MIN, approximately 1.175494e-38.
    self->elems[0][0] = 1e-10;
    self->elems[0][1] = 0.0;
    self->elems[1][0] = 0.0;
    self->elems[1][1] = 1e-10;
  }
}

// -------------------------------- #include "./paint.c"

iconvg_paint_type  //
iconvg_paint__type(const iconvg_paint* self) {
  if (self) {
    const uint8_t* rgba = &self->paint_rgba[0];
    if ((rgba[0] <= rgba[3]) &&  //
        (rgba[1] <= rgba[3]) &&  //
        (rgba[2] <= rgba[3])) {
      return ICONVG_PAINT_TYPE__FLAT_COLOR;
    } else if ((rgba[3] == 0x00) && (rgba[2] >= 0x80)) {
      return (rgba[2] & 0x40) ? ICONVG_PAINT_TYPE__RADIAL_GRADIENT
                              : ICONVG_PAINT_TYPE__LINEAR_GRADIENT;
    }
  }
  return ICONVG_PAINT_TYPE__INVALID;
}

// ----

static inline iconvg_nonpremul_color  //
iconvg_private_flat_color_as_nonpremul_color(const uint8_t* rgba) {
  iconvg_nonpremul_color k;
  if (!rgba || (rgba[3] == 0x00)) {
    memset(&k.rgba[0], 0, 4);
  } else if (rgba[3] == 0xFF) {
    memcpy(&k.rgba[0], rgba, 4);
  } else {
    uint32_t a = rgba[3];
    k.rgba[0] = ((uint8_t)(((uint32_t)(rgba[0])) * 0xFF / a));
    k.rgba[1] = ((uint8_t)(((uint32_t)(rgba[1])) * 0xFF / a));
    k.rgba[2] = ((uint8_t)(((uint32_t)(rgba[2])) * 0xFF / a));
    k.rgba[3] = ((uint8_t)a);
  }
  return k;
}

static inline iconvg_premul_color  //
iconvg_private_flat_color_as_premul_color(const uint8_t* rgba) {
  iconvg_premul_color k;
  if (!rgba) {
    memset(&k.rgba[0], 0, 4);
  } else {
    memcpy(&k.rgba[0], rgba, 4);
  }
  return k;
}

// ----

iconvg_nonpremul_color  //
iconvg_paint__flat_color_as_nonpremul_color(const iconvg_paint* self) {
  return iconvg_private_flat_color_as_nonpremul_color(
      self ? &self->paint_rgba[0] : NULL);
}

iconvg_premul_color  //
iconvg_paint__flat_color_as_premul_color(const iconvg_paint* self) {
  return iconvg_private_flat_color_as_premul_color(self ? &self->paint_rgba[0]
                                                        : NULL);
}

// ----

iconvg_gradient_spread  //
iconvg_paint__gradient_spread(const iconvg_paint* self) {
  if (!self) {
    return ICONVG_GRADIENT_SPREAD__NONE;
  }
  return (iconvg_gradient_spread)(self->paint_rgba[1] >> 6);
}

uint32_t  //
iconvg_paint__gradient_number_of_stops(const iconvg_paint* self) {
  if (!self) {
    return 0;
  }
  return 0x3F & self->paint_rgba[0];
}

iconvg_nonpremul_color  //
iconvg_paint__gradient_stop_color_as_nonpremul_color(const iconvg_paint* self,
                                                     uint32_t which_stop) {
  const uint8_t* rgba = NULL;
  if (self) {
    uint32_t cbase = self->paint_rgba[1];
    rgba = &self->creg.colors[0x3F & (cbase + which_stop)].rgba[0];
  }
  return iconvg_private_flat_color_as_nonpremul_color(rgba);
}

iconvg_premul_color  //
iconvg_paint__gradient_stop_color_as_premul_color(const iconvg_paint* self,
                                                  uint32_t which_stop) {
  const uint8_t* rgba = NULL;
  if (self) {
    uint32_t cbase = self->paint_rgba[1];
    rgba = &self->creg.colors[0x3F & (cbase + which_stop)].rgba[0];
  }
  return iconvg_private_flat_color_as_premul_color(rgba);
}

float  //
iconvg_paint__gradient_stop_offset(const iconvg_paint* self,
                                   uint32_t which_stop) {
  if (!self) {
    return 0;
  }
  uint32_t nbase = self->paint_rgba[2];
  return self->nreg[0x3F & (nbase + which_stop)];
}

iconvg_matrix_2x3_f64  //
iconvg_paint__gradient_transformation_matrix(const iconvg_paint* self) {
  if (!self) {
    return iconvg_matrix_2x3_f64__make(1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  }

  uint32_t nbase = self->paint_rgba[2];
  double s00 = self->nreg[0x3F & (nbase - 6)];
  double s01 = self->nreg[0x3F & (nbase - 5)];
  double s02 = self->nreg[0x3F & (nbase - 4)];
  double s10 = self->nreg[0x3F & (nbase - 3)];
  double s11 = self->nreg[0x3F & (nbase - 2)];
  double s12 = self->nreg[0x3F & (nbase - 1)];

  // The [s00, s01, s02; s10, s11, s12] matrix transforms from *src*
  // coordinates to pattern coordinates.
  //
  //   pat_x = (src_x * s00) + (src_y * s01) + s02
  //   pat_y = (src_x * s10) + (src_y * s11) + s12
  //
  // Pattern coordinate space (also known as paint or gradient coordinate
  // space) is where linear gradients always range from x=0 to x=1 and radial
  // gradients are always center=(0,0) and radius=1. We can't just return this
  // matrix to the caller. We need to produce the equivalent [d00, d01, d02;
  // d10, d11, d12] matrix that transforms from *dst* coordinates to pattern
  // coordinates. Recall that:
  //
  //   src_x = (dst_x * d2s_scale_x) + d2s_bias_x
  //   src_y = (dst_y * d2s_scale_y) + d2s_bias_y
  //
  // Combining the above, we can solve for d00, d01, etc such that:
  //
  //   pat_x = (dst_x * d00) + (dst_y * d01) + d02
  //   pat_y = (dst_x * d10) + (dst_y * d11) + d12
  double d00 = s00 * self->d2s_scale_x;
  double d01 = s01 * self->d2s_scale_y;
  double d02 = (s00 * self->d2s_bias_x) + (s01 * self->d2s_bias_y) + s02;
  double d10 = s10 * self->d2s_scale_x;
  double d11 = s11 * self->d2s_scale_y;
  double d12 = (s10 * self->d2s_bias_x) + (s11 * self->d2s_bias_y) + s12;

  return iconvg_matrix_2x3_f64__make(d00, d01, d02, d10, d11, d12);
}

// -------------------------------- #include "./rectangle.c"

// Note that iconvg_rectangle_f32 fields may be NaN, so that (min < max) is not
// the same as !(min >= max).

bool  //
iconvg_rectangle_f32__is_finite_and_not_empty(
    const iconvg_rectangle_f32* self) {
  return self &&                         //
         (-INFINITY < self->min_x) &&    //
         (self->min_x < self->max_x) &&  //
         (self->max_x < +INFINITY) &&    //
         (-INFINITY < self->min_y) &&    //
         (self->min_y < self->max_y) &&  //
         (self->max_y < +INFINITY);
}

double  //
iconvg_rectangle_f32__width_f64(const iconvg_rectangle_f32* self) {
  if (self && (self->max_x > self->min_x)) {
    return ((double)self->max_x) - ((double)self->min_x);
  }
  return 0.0;
}

double  //
iconvg_rectangle_f32__height_f64(const iconvg_rectangle_f32* self) {
  if (self && (self->max_y > self->min_y)) {
    return ((double)self->max_y) - ((double)self->min_y);
  }
  return 0.0;
}

// -------------------------------- #include "./skia.c"

#if !defined(ICONVG_CONFIG__ENABLE_SKIA_BACKEND)

iconvg_canvas  //
iconvg_canvas__make_skia(sk_canvas_t* sc) {
  return iconvg_canvas__make_broken(iconvg_error_invalid_backend_not_enabled);
}

#else  // ICONVG_CONFIG__ENABLE_SKIA_BACKEND

#include "include/c/sk_canvas.h"
#include "include/c/sk_matrix.h"
#include "include/c/sk_paint.h"
#include "include/c/sk_path.h"
#include "include/c/sk_shader.h"

static const sk_shader_tilemode_t
    iconvg_private_gradient_spread_as_sk_shader_tilemode_t[4] = {
        CLAMP_SK_SHADER_TILEMODE,   //
        CLAMP_SK_SHADER_TILEMODE,   //
        MIRROR_SK_SHADER_TILEMODE,  //
        REPEAT_SK_SHADER_TILEMODE   //
};

// iconvg_private_skia_set_gradient_stops sets the Skia gradient stop colors
// given the IconVG gradient stop colors.
//
// Like iconvg_private_cairo_set_gradient_stops, the complexity is due to
// premultiplied versus non-premultiplied alpha.
//
// It returns the number of Skia stops added.
static uint32_t  //
iconvg_private_skia_set_gradient_stops(sk_color_t* gcol,
                                       float* goff,
                                       const iconvg_paint* p) {
  uint32_t ret = 0;

  // foo0 and foo2 are the previous and current gradient stop. Sometimes we
  // need to synthesize additional stops in between them, whose variables are
  // named foo1.
  double offset0 = 0.0;
  uint8_t r0 = 0x00;
  uint8_t g0 = 0x00;
  uint8_t b0 = 0x00;
  uint8_t a0 = 0x00;

  uint32_t num_stops = iconvg_paint__gradient_number_of_stops(p);
  for (uint32_t i = 0; i < num_stops; i++) {
    // Calculate offset and color for the current stop.
    double offset2 = iconvg_paint__gradient_stop_offset(p, i);
    iconvg_premul_color k =
        iconvg_paint__gradient_stop_color_as_premul_color(p, i);
    uint8_t r2 = k.rgba[0];
    uint8_t g2 = k.rgba[1];
    uint8_t b2 = k.rgba[2];
    uint8_t a2 = k.rgba[3];

    if ((i == 0) ||                        //
        ((a0 == 0xFF) && (a2 == 0xFF)) ||  //
        ((a0 == 0x00) && (a2 == 0x00))) {
      // If it's the first stop, or if we're interpolating from 100% to 100%
      // opaque or from 0% to 0% opaque, we don't have to worry about
      // premultiplied versus non-premultiplied alpha.
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret++;

    } else if (a0 == 0x00) {
      // If we're blending e.g. from transparent black to (partially) opaque
      // blue, insert "transparent blue" immediately after the previous
      // "transparent black".
      *gcol++ = sk_color_set_argb(0x00, r2, g2, b2);
      *goff++ = offset0;
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret += 2;

    } else if (a2 == 0x00) {
      // If we're blending e.g. from (partially) opaque blue to transparent
      // black, insert "transparent blue" immediately before the current
      // "transparent black".
      *gcol++ = sk_color_set_argb(0x00, r0, g0, b0);
      *goff++ = offset2;
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret += 2;

    } else {
      // Otherwise, fake "interpolate with premultiplied alpha" like
      // iconvg_private_cairo_set_gradient_stops does.
      const int32_t n = 16;
      for (int32_t i = (n - 1); i >= 0; i--) {
        int32_t j = n - i;
        double offset1 = ((i * offset0) + (j * offset2)) / n;
        uint8_t r1 = ((i * r0) + (j * r2)) / n;
        uint8_t g1 = ((i * g0) + (j * g2)) / n;
        uint8_t b1 = ((i * b0) + (j * b2)) / n;
        uint8_t a1 = ((i * a0) + (j * a2)) / n;
        if (a1 == 0x00) {
          *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
          *goff++ = offset1;
          ret++;
        } else {
          *gcol++ = sk_color_set_argb(a1,                 //
                                      (0xFF * r1) / a1,   //
                                      (0xFF * g1) / a1,   //
                                      (0xFF * b1) / a1);  //
          *goff++ = offset1;
          ret++;
        }
      }
    }

    // Update offset and color for the previous stop.
    offset0 = offset2;
    r0 = r2;
    g0 = g2;
    b0 = b2;
    a0 = a2;
  }

  return ret;
}

static const char*  //
iconvg_private_skia_canvas__begin_decode(iconvg_canvas* c,
                                         iconvg_rectangle_f32 dst_rect) {
  sk_canvas_t* sc = (sk_canvas_t*)(c->context_nonconst_ptr0);
  sk_canvas_save(sc);

  sk_rect_t rect;
  rect.left = dst_rect.min_x;
  rect.top = dst_rect.min_y;
  rect.right = dst_rect.max_x;
  rect.bottom = dst_rect.max_y;
  sk_canvas_clip_rect(sc, &rect);

  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_decode(iconvg_canvas* c,
                                       const char* err_msg,
                                       size_t num_bytes_consumed,
                                       size_t num_bytes_remaining) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr1 = NULL;
  }
  sk_canvas_t* sc = (sk_canvas_t*)(c->context_nonconst_ptr0);
  sk_canvas_restore(sc);
  return err_msg;
}

static const char*  //
iconvg_private_skia_canvas__begin_drawing(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr1 = NULL;
  }
  spb = sk_pathbuilder_new();
  if (!spb) {
    return iconvg_error_system_failure_out_of_memory;
  }
  c->context_nonconst_ptr1 = spb;
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_drawing(iconvg_canvas* c,
                                        const iconvg_paint* p) {
  sk_canvas_t* sc = (sk_canvas_t*)(c->context_nonconst_ptr0);
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);

  iconvg_paint_type paint_type = iconvg_paint__type(p);
  switch (paint_type) {
    case ICONVG_PAINT_TYPE__FLAT_COLOR: {
      iconvg_nonpremul_color k = iconvg_paint__flat_color_as_nonpremul_color(p);
      sk_path_t* path = sk_pathbuilder_detach_path(spb);
      sk_paint_t* paint = sk_paint_new();
      sk_paint_set_antialias(paint, true);
      sk_paint_set_color(
          paint, sk_color_set_argb(k.rgba[3], k.rgba[0], k.rgba[1], k.rgba[2]));
      sk_canvas_draw_path(sc, path, paint);
      sk_paint_delete(paint);
      sk_path_delete(path);
      return NULL;
    }
    case ICONVG_PAINT_TYPE__LINEAR_GRADIENT:
    case ICONVG_PAINT_TYPE__RADIAL_GRADIENT:
      break;
    default:
      return iconvg_error_invalid_paint_type;
  }

  // The matrix in IconVG's API converts from dst coordinate space to pattern
  // coordinate space. Skia's API is the other way around (matrix inversion).
  iconvg_matrix_2x3_f64 im = iconvg_paint__gradient_transformation_matrix(p);
  if (paint_type == ICONVG_PAINT_TYPE__LINEAR_GRADIENT) {
    iconvg_matrix_2x3_f64__override_second_row(&im);
  }
  im = iconvg_matrix_2x3_f64__inverse(&im);
  sk_matrix_t sm;
  sm.mat[0] = im.elems[0][0];
  sm.mat[1] = im.elems[0][1];
  sm.mat[2] = im.elems[0][2];
  sm.mat[3] = im.elems[1][0];
  sm.mat[4] = im.elems[1][1];
  sm.mat[5] = im.elems[1][2];
  sm.mat[6] = 0.0f;
  sm.mat[7] = 0.0f;
  sm.mat[8] = 1.0f;

  // The gradient is either:
  //   - linear, from (0, 0) to (1, 0), or
  //   - radial, centered at (0, 0).
  sk_point_t gradient_points[2];
  gradient_points[0].x = 0;
  gradient_points[0].y = 0;
  gradient_points[1].x = 1;
  gradient_points[1].y = 0;

  // Configure the gradient stops.
  //
  // Skia doesn't have NONE_SK_SHADER_TILEMODE. Use CLAMP_SK_SHADER_TILEMODE
  // instead, for IconVG's ICONVG_GRADIENT_SPREAD__NONE, adding a transparent
  // black gradient stop at both ends.
  //
  // 1010 equals ((63 * 16) + 2). 63 is the maximum (inclusive) number of
  // gradient stops. iconvg_private_skia_set_gradient_stops can expand each
  // IconVG stop to up to 16 Skia stops. There's also 2 extra stops if we
  // use the ICONVG_GRADIENT_SPREAD__NONE workaround.
  sk_color_t gradient_colors[1010];
  float gradient_offsets[1010];
  sk_color_t* gcol = &gradient_colors[0];
  float* goff = &gradient_offsets[0];
  iconvg_gradient_spread gradient_spread = iconvg_paint__gradient_spread(p);
  uint32_t gradient_num_stops = 0;
  if (gradient_spread == ICONVG_GRADIENT_SPREAD__NONE) {
    *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
    *goff++ = 0.0f;
    gradient_num_stops++;
  }
  {
    uint32_t additional_stops =
        iconvg_private_skia_set_gradient_stops(gcol, goff, p);
    gcol += additional_stops;
    goff += additional_stops;
    gradient_num_stops += additional_stops;
  }
  if (gradient_spread == ICONVG_GRADIENT_SPREAD__NONE) {
    *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
    *goff++ = 0.0f;
    gradient_num_stops++;
  }

  // Make the Skia shader.
  sk_shader_t* shader = NULL;
  if (paint_type == ICONVG_PAINT_TYPE__LINEAR_GRADIENT) {
    shader = sk_shader_new_linear_gradient(
        gradient_points, gradient_colors, gradient_offsets, gradient_num_stops,
        iconvg_private_gradient_spread_as_sk_shader_tilemode_t[gradient_spread],
        &sm);
  } else {
    static const float radius = 1.0f;
    shader = sk_shader_new_radial_gradient(
        gradient_points, radius, gradient_colors, gradient_offsets,
        gradient_num_stops,
        iconvg_private_gradient_spread_as_sk_shader_tilemode_t[gradient_spread],
        &sm);
  }

  // Use the Skia shader.
  if (shader) {
    sk_path_t* path = sk_pathbuilder_detach_path(spb);
    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_antialias(paint, true);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_path(sc, path, paint);
    sk_paint_delete(paint);
    sk_path_delete(path);
  }

  // Clean up.
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr1 = NULL;
  }
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  sk_pathbuilder_move_to(spb, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_path(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  sk_pathbuilder_close(spb);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_line_to(iconvg_canvas* c, float x1, float y1) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  sk_pathbuilder_line_to(spb, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_quad_to(iconvg_canvas* c,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  sk_pathbuilder_quad_to(spb, x1, y1, x2, y2);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_cube_to(iconvg_canvas* c,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2,
                                         float x3,
                                         float y3) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr1);
  sk_pathbuilder_cubic_to(spb, x1, y1, x2, y2, x3, y3);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                iconvg_rectangle_f32 viewbox) {
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return NULL;
}

static const iconvg_canvas_vtable  //
    iconvg_private_skia_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_skia_canvas__begin_decode,
        &iconvg_private_skia_canvas__end_decode,
        &iconvg_private_skia_canvas__begin_drawing,
        &iconvg_private_skia_canvas__end_drawing,
        &iconvg_private_skia_canvas__begin_path,
        &iconvg_private_skia_canvas__end_path,
        &iconvg_private_skia_canvas__path_line_to,
        &iconvg_private_skia_canvas__path_quad_to,
        &iconvg_private_skia_canvas__path_cube_to,
        &iconvg_private_skia_canvas__on_metadata_viewbox,
        &iconvg_private_skia_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_canvas__make_skia(sk_canvas_t* sc) {
  if (!sc) {
    return iconvg_canvas__make_broken(
        iconvg_error_invalid_constructor_argument);
  }
  iconvg_canvas c;
  c.vtable = &iconvg_private_skia_canvas_vtable;
  c.context_nonconst_ptr0 = sc;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = NULL;
  c.context_extra = 0;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_SKIA_BACKEND

#endif  // ICONVG_IMPLEMENTATION

#endif  // ICONVG_INCLUDE_GUARD
