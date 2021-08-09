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
//   - iconvg_error_bad_coordinate
//   - iconvg_error_bad_jump
//   - iconvg_error_bad_magic_identifier
//   - iconvg_error_bad_metadata
//   - iconvg_error_bad_metadata_id_order
//   - iconvg_error_bad_metadata_suggested_palette
//   - iconvg_error_bad_metadata_viewbox
//   - iconvg_error_bad_number
//   - iconvg_error_bad_opcode_length
//   - iconvg_error_invalid_backend_not_enabled
//   - iconvg_error_invalid_constructor_argument
//   - iconvg_error_invalid_paint_type
//   - iconvg_error_invalid_vtable
//   - iconvg_error_system_failure_out_of_memory

// ----

// Some terse comments below contain the U+00B6 PILCROW SIGN (¶), indicating
// that the annotated function, type, etc is part of the public API. The
// pilcrow is followed by the library version (e.g. ¶0.2 or ¶1.0.56) that the
// annotated item has debuted in or will debut in.
//
// Some structs may grow in size across library versions (and passed by
// pointer), provided that the first field holds the sizeof that struct and
// that new versions only add fields, never remove or otherwise change existing
// fields. "The fields above are ¶etc" pilcrow comments within such structs
// annotate which fields were added in which versions.

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
// Other errors (invalid_etc) are programming errors.

extern const char iconvg_error_bad_coordinate[];                  // ¶0.1
extern const char iconvg_error_bad_jump[];                        // ¶0.1
extern const char iconvg_error_bad_magic_identifier[];            // ¶0.1
extern const char iconvg_error_bad_metadata[];                    // ¶0.1
extern const char iconvg_error_bad_metadata_id_order[];           // ¶0.1
extern const char iconvg_error_bad_metadata_suggested_palette[];  // ¶0.1
extern const char iconvg_error_bad_metadata_viewbox[];            // ¶0.1
extern const char iconvg_error_bad_number[];                      // ¶0.1
extern const char iconvg_error_bad_opcode_length[];               // ¶0.1

extern const char iconvg_error_system_failure_out_of_memory[];  // ¶0.1

extern const char iconvg_error_invalid_backend_not_enabled[];   // ¶0.1
extern const char iconvg_error_invalid_constructor_argument[];  // ¶0.1
extern const char iconvg_error_invalid_paint_type[];            // ¶0.1
extern const char iconvg_error_invalid_vtable[];                // ¶0.1

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
//
// Example code:
//   iconvg_decode_options opts = {0};
//   opts.sizeof__iconvg_decode_options = sizeof(iconvg_decode_options);
//   opts.palette = palette;
//   return iconvg_decode(etc, &opts);
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

  // context's fields' semantics depend on the 'sub-class' and should be
  // considered private implementation details. For built-in 'sub-classes', as
  // returned by the library's iconvg_canvas__make_etc functions, users should
  // not read or write these fields directly and their semantics may change
  // between library versions.
  //
  // Not all of these fields are used yet, but we overprovision because this
  // struct cannot grow in size across library versions.
  struct {
    void* nonconst_ptr1;
    void* nonconst_ptr2;
    const void* const_ptr3;
    const void* const_ptr4;
    size_t extra5;
    size_t extra6;
    size_t extra7;
  } context;
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

static inline uint16_t  //
iconvg_private_peek_u16le(const uint8_t* p) {
  return (uint16_t)(((uint16_t)(p[0]) << 0) | ((uint16_t)(p[1]) << 8));
}

static inline uint32_t  //
iconvg_private_peek_u32le(const uint8_t* p) {
  return ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

static inline uint64_t  //
iconvg_private_peek_u64le(const uint8_t* p) {
  return ((uint64_t)(p[0]) << 0) | ((uint64_t)(p[1]) << 8) |
         ((uint64_t)(p[2]) << 16) | ((uint64_t)(p[3]) << 24) |
         ((uint64_t)(p[4]) << 32) | ((uint64_t)(p[5]) << 40) |
         ((uint64_t)(p[6]) << 48) | ((uint64_t)(p[7]) << 56);
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

extern const uint32_t iconvg_private_one_byte_colors[128];
extern const iconvg_palette iconvg_private_default_palette;

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
  iconvg_palette custom_palette;

  // iconvg_private_initialize_remaining_paint_fields sets the fields below.

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

  uint8_t sel;
  bool begun_drawing;
  bool begun_path;

  uint8_t paint_type;
  uint8_t num_stops;
  uint8_t spread;
  uint8_t which_regs;

  union {
    // coords[0] are the current x and y coordinates. coords[1..4] are the x
    // and y coordinates of the path op arguments. That final space (6 floats)
    // is also used to hold gradient transformation matrices.
    float coords[4][2];
    struct {
      float current_x;
      float current_y;
      float transform[6];
    };
  };

  uint64_t regs[64];
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

// -------------------------------- #include "./broken.c"

static const char*  //
iconvg_private_broken_canvas__begin_decode(iconvg_canvas* c,
                                           iconvg_rectangle_f32 dst_rect) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__end_decode(iconvg_canvas* c,
                                         const char* err_msg,
                                         size_t num_bytes_consumed,
                                         size_t num_bytes_remaining) {
  return err_msg ? err_msg : ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__begin_drawing(iconvg_canvas* c) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__end_drawing(iconvg_canvas* c,
                                          const iconvg_paint* p) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__end_path(iconvg_canvas* c) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__path_line_to(iconvg_canvas* c,
                                           float x1,
                                           float y1) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__path_quad_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__path_cube_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2,
                                           float x3,
                                           float y3) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_viewbox(
    iconvg_canvas* c,
    iconvg_rectangle_f32 viewbox) {
  return ((const char*)(c->context.const_ptr3));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return ((const char*)(c->context.const_ptr3));
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
  memset(&c.context, 0, sizeof(c.context));
  c.context.const_ptr3 = err_msg;
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
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
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
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
  cairo_restore(cr);
  return err_msg;
}

static const char*  //
iconvg_private_cairo_canvas__begin_drawing(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
  cairo_new_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
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
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
  cairo_move_to(cr, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_path(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
  cairo_close_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
  cairo_line_to(cr, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
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
  cairo_t* cr = (cairo_t*)(c->context.nonconst_ptr1);
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
  memset(&c.context, 0, sizeof(c.context));
  c.context.nonconst_ptr1 = cr;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND

// -------------------------------- #include "./color.c"

// iconvg_private_one_byte_colors holds the first 128 one-byte colors, in
// 0xAABBGGRR alpha-premultiplied format.
const uint32_t iconvg_private_one_byte_colors[128] = {
    0x00000000,  //
    0x80808080,  //
    0xC0C0C0C0,  //
    0xFF000000,  //
    0xFF400000,  //
    0xFF800000,  //
    0xFFC00000,  //
    0xFFFF0000,  //
    0xFF004000,  //
    0xFF404000,  //
    0xFF804000,  //
    0xFFC04000,  //
    0xFFFF4000,  //
    0xFF008000,  //
    0xFF408000,  //
    0xFF808000,  //
    0xFFC08000,  //
    0xFFFF8000,  //
    0xFF00C000,  //
    0xFF40C000,  //
    0xFF80C000,  //
    0xFFC0C000,  //
    0xFFFFC000,  //
    0xFF00FF00,  //
    0xFF40FF00,  //
    0xFF80FF00,  //
    0xFFC0FF00,  //
    0xFFFFFF00,  //
    0xFF000040,  //
    0xFF400040,  //
    0xFF800040,  //
    0xFFC00040,  //
    0xFFFF0040,  //
    0xFF004040,  //
    0xFF404040,  //
    0xFF804040,  //
    0xFFC04040,  //
    0xFFFF4040,  //
    0xFF008040,  //
    0xFF408040,  //
    0xFF808040,  //
    0xFFC08040,  //
    0xFFFF8040,  //
    0xFF00C040,  //
    0xFF40C040,  //
    0xFF80C040,  //
    0xFFC0C040,  //
    0xFFFFC040,  //
    0xFF00FF40,  //
    0xFF40FF40,  //
    0xFF80FF40,  //
    0xFFC0FF40,  //
    0xFFFFFF40,  //
    0xFF000080,  //
    0xFF400080,  //
    0xFF800080,  //
    0xFFC00080,  //
    0xFFFF0080,  //
    0xFF004080,  //
    0xFF404080,  //
    0xFF804080,  //
    0xFFC04080,  //
    0xFFFF4080,  //
    0xFF008080,  //
    0xFF408080,  //
    0xFF808080,  //
    0xFFC08080,  //
    0xFFFF8080,  //
    0xFF00C080,  //
    0xFF40C080,  //
    0xFF80C080,  //
    0xFFC0C080,  //
    0xFFFFC080,  //
    0xFF00FF80,  //
    0xFF40FF80,  //
    0xFF80FF80,  //
    0xFFC0FF80,  //
    0xFFFFFF80,  //
    0xFF0000C0,  //
    0xFF4000C0,  //
    0xFF8000C0,  //
    0xFFC000C0,  //
    0xFFFF00C0,  //
    0xFF0040C0,  //
    0xFF4040C0,  //
    0xFF8040C0,  //
    0xFFC040C0,  //
    0xFFFF40C0,  //
    0xFF0080C0,  //
    0xFF4080C0,  //
    0xFF8080C0,  //
    0xFFC080C0,  //
    0xFFFF80C0,  //
    0xFF00C0C0,  //
    0xFF40C0C0,  //
    0xFF80C0C0,  //
    0xFFC0C0C0,  //
    0xFFFFC0C0,  //
    0xFF00FFC0,  //
    0xFF40FFC0,  //
    0xFF80FFC0,  //
    0xFFC0FFC0,  //
    0xFFFFFFC0,  //
    0xFF0000FF,  //
    0xFF4000FF,  //
    0xFF8000FF,  //
    0xFFC000FF,  //
    0xFFFF00FF,  //
    0xFF0040FF,  //
    0xFF4040FF,  //
    0xFF8040FF,  //
    0xFFC040FF,  //
    0xFFFF40FF,  //
    0xFF0080FF,  //
    0xFF4080FF,  //
    0xFF8080FF,  //
    0xFFC080FF,  //
    0xFFFF80FF,  //
    0xFF00C0FF,  //
    0xFF40C0FF,  //
    0xFF80C0FF,  //
    0xFFC0C0FF,  //
    0xFFFFC0FF,  //
    0xFF00FFFF,  //
    0xFF40FFFF,  //
    0xFF80FFFF,  //
    0xFFC0FFFF,  //
    0xFFFFFFFF,  //
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
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%sbegin_decode({%g, %g, %g, %g})\n",
            ((const char*)(c->context.const_ptr3)), dst_rect.min_x,
            dst_rect.min_y, dst_rect.max_x, dst_rect.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->begin_decode)(wrapped, dst_rect);
}

static const char*  //
iconvg_private_debug_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    const char* quote = err_msg ? "\"" : "";
    fprintf(f, "%send_decode(%s%s%s, %zu, %zu)\n",
            ((const char*)(c->context.const_ptr3)), quote,
            err_msg ? err_msg : "NULL", quote, num_bytes_consumed,
            num_bytes_remaining);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return err_msg;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->end_decode)(wrapped, err_msg, num_bytes_consumed,
                                        num_bytes_remaining);
}

static const char*  //
iconvg_private_debug_canvas__begin_drawing(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%sbegin_drawing()\n", ((const char*)(c->context.const_ptr3)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->begin_drawing)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  static const char* spread_names[4] = {"none", "pad", "reflect", "repeat"};

  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    switch (iconvg_paint__type(p)) {
      case ICONVG_PAINT_TYPE__FLAT_COLOR: {
        iconvg_premul_color k = iconvg_paint__flat_color_as_premul_color(p);
        fprintf(f, "%send_drawing(flat_color{%02X:%02X:%02X:%02X})\n",
                ((const char*)(c->context.const_ptr3)), ((int)(k.rgba[0])),
                ((int)(k.rgba[1])), ((int)(k.rgba[2])), ((int)(k.rgba[3])));
        break;
      }

      case ICONVG_PAINT_TYPE__LINEAR_GRADIENT: {
        fprintf(f,
                "%send_drawing(linear_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context.const_ptr3)),
                ((int)(iconvg_paint__gradient_number_of_stops(p))),
                spread_names[iconvg_paint__gradient_spread(p)]);
        break;
      }

      case ICONVG_PAINT_TYPE__RADIAL_GRADIENT: {
        fprintf(f,
                "%send_drawing(radial_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context.const_ptr3)),
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
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->end_drawing)(wrapped, p);
}

static const char*  //
iconvg_private_debug_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%sbegin_path(%g, %g)\n", ((const char*)(c->context.const_ptr3)),
            x0, y0);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->begin_path)(wrapped, x0, y0);
}

static const char*  //
iconvg_private_debug_canvas__end_path(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%send_path()\n", ((const char*)(c->context.const_ptr3)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->end_path)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%spath_line_to(%g, %g)\n",
            ((const char*)(c->context.const_ptr3)), x1, y1);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->path_line_to)(wrapped, x1, y1);
}

static const char*  //
iconvg_private_debug_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%spath_quad_to(%g, %g, %g, %g)\n",
            ((const char*)(c->context.const_ptr3)), x1, y1, x2, y2);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
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
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%spath_cube_to(%g, %g, %g, %g, %g, %g)\n",
            ((const char*)(c->context.const_ptr3)), x1, y1, x2, y2, x3, y3);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->path_cube_to)(wrapped, x1, y1, x2, y2, x3, y3);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                 iconvg_rectangle_f32 viewbox) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    fprintf(f, "%son_metadata_viewbox({%g, %g, %g, %g})\n",
            ((const char*)(c->context.const_ptr3)), viewbox.min_x,
            viewbox.min_y, viewbox.max_x, viewbox.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
  }
  return (*wrapped->vtable->on_metadata_viewbox)(wrapped, viewbox);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  FILE* f = (FILE*)(c->context.nonconst_ptr2);
  if (f) {
    int j = iconvg_private_last_color_that_isnt_opaque_black(suggested_palette);
    if (j < 0) {
      fprintf(f, "%son_metadata_suggested_palette(...)\n",
              ((const char*)(c->context.const_ptr3)));
    } else {
      fprintf(f, "%son_metadata_suggested_palette(",
              ((const char*)(c->context.const_ptr3)));
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
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context.nonconst_ptr1);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_invalid_vtable;
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
  memset(&c.context, 0, sizeof(c.context));
  c.context.nonconst_ptr1 = wrapped;
  c.context.nonconst_ptr2 = f;
  c.context.const_ptr3 = message_prefix ? message_prefix : "";
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
iconvg_private_decoder__decode_coordinates(iconvg_private_decoder* self,
                                           float* dst_ptr,
                                           size_t dst_len) {
  for (; dst_len > 0; dst_len--) {
    if (self->len == 0) {
      return false;
    }
    uint8_t v = self->ptr[0];
    if ((v & 0x01) != 0) {  // 1-byte encoding.
      int32_t i = (int32_t)(v >> 1);
      *dst_ptr++ = ((float)(i - 64));
      self->ptr += 1;
      self->len -= 1;

    } else if ((v & 0x02) != 0) {  // 2-byte encoding.
      if (self->len < 2) {
        return false;
      }
      int32_t i = (int32_t)(iconvg_private_peek_u16le(self->ptr) >> 2);
      *dst_ptr++ = ((float)(i - (128 * 64))) / 64.0f;
      self->ptr += 2;
      self->len -= 2;

    } else {  // 4-byte encoding.
      if (self->len < 4) {
        return false;
      }
      // TODO: reject NaNs?
      *dst_ptr++ = iconvg_private_reinterpret_from_u32_to_f32(
          iconvg_private_peek_u32le(self->ptr));
      self->ptr += 4;
      self->len -= 4;
    }
  }
  return true;
}

static bool  //
iconvg_private_decoder__decode_natural_number(iconvg_private_decoder* self,
                                              uint32_t* dst) {
  if (self->len == 0) {
    return false;
  }
  uint8_t v = self->ptr[0];
  if ((v & 0x01) != 0) {  // 1-byte encoding.
    *dst = v >> 1;
    self->ptr += 1;
    self->len -= 1;

  } else if ((v & 0x02) != 0) {  // 2-byte encoding.
    if (self->len < 2) {
      return false;
    }
    *dst = iconvg_private_peek_u16le(self->ptr) >> 2;
    self->ptr += 2;
    self->len -= 2;

  } else {  // 4-byte encoding.
    if (self->len < 4) {
      return false;
    }
    *dst = iconvg_private_peek_u32le(self->ptr) >> 2;
    self->ptr += 4;
    self->len -= 4;
  }
  return true;
}

static bool  //
iconvg_private_decoder__decode_float32(iconvg_private_decoder* self,
                                       float* dst) {
  if (self->len < 4) {
    return false;
  }
  // TODO: reject NaNs?
  *dst = iconvg_private_reinterpret_from_u32_to_f32(
      iconvg_private_peek_u32le(self->ptr));
  self->ptr += 4;
  self->len -= 4;
  return true;
}

// ----

static bool  //
iconvg_private_decoder__decode_magic_identifier(iconvg_private_decoder* self) {
  if ((self->len < 4) ||         //
      (self->ptr[0] != 0x8A) ||  //
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
  float a[2][2];
  if (iconvg_private_decoder__decode_coordinates(self, a[0], 4) &&
      (-INFINITY < a[0][0]) &&  //
      (a[0][0] <= a[1][0]) &&   //
      (a[1][0] < +INFINITY) &&  //
      (-INFINITY < a[0][1]) &&  //
      (a[0][1] <= a[1][1]) &&   //
      (a[1][1] < +INFINITY)) {
    dst->min_x = a[0][0];
    dst->min_y = a[0][1];
    dst->max_x = a[1][0];
    dst->max_y = a[1][1];
    return true;
  }
  return false;
}

static bool  //
iconvg_private_decoder__decode_metadata_suggested_palette(
    iconvg_private_decoder* self,
    iconvg_palette* dst) {
  if ((self->len == 0) || (self->ptr[0] >= 0x40)) {
    return false;
  }
  size_t n = 1 + self->ptr[0];
  self->ptr += 1;
  self->len -= 1;

  if (self->len != (n * 4)) {
    return false;
  }
  const uint8_t* ptr = self->ptr;
  self->ptr += self->len;
  self->len = 0;

  iconvg_premul_color* c = &dst->colors[0];
  for (; n > 0; n--) {
    c->rgba[0] = *ptr++;
    c->rgba[1] = *ptr++;
    c->rgba[2] = *ptr++;
    c->rgba[3] = *ptr++;
    c++;
  }
  return true;
}

// ----

static const char*  //
iconvg_private_expand_call(iconvg_canvas* c,
                           iconvg_private_decoder* d,
                           iconvg_paint* p,
                           uint8_t opcode) {
  // TODO: implement call ops. For now, just jump over them.

  // Handle the ATM (Alpha and Transform Matrix).
  if (opcode & 1) {
    if (d->len < 25) {
      return iconvg_error_bad_opcode_length;
    }
    d->ptr += 25;
    d->len -= 25;
  }

  if (opcode & 2) {
    // Absolute FileSegment.
    if (d->len < 8) {
      return iconvg_error_bad_opcode_length;
    }
    d->ptr += 8;
    d->len -= 8;
  } else {
    // Inline FileSegment.
    if (d->len < 4) {
      return iconvg_error_bad_opcode_length;
    }
    uint32_t n = 4 + (iconvg_private_peek_u32le(d->ptr) >> 8);
    if (d->len < n) {
      return iconvg_error_bad_opcode_length;
    }
    d->ptr += n;
    d->len -= n;
  }

  return NULL;
}

static const char*  //
iconvg_private_expand_ellipse_parallelogram(iconvg_canvas* c,
                                            iconvg_private_decoder* d,
                                            iconvg_paint* p,
                                            uint8_t opcode) {
  // Decode the two explicit coordinate pairs.
  if (!iconvg_private_decoder__decode_coordinates(d, p->coords[1], 4)) {
    return iconvg_error_bad_coordinate;
  }

  // The third coordinate pair is implicit.
  p->coords[3][0] = p->coords[0][0] - p->coords[1][0] + p->coords[2][0];
  p->coords[3][1] = p->coords[0][1] - p->coords[1][1] + p->coords[2][1];

  // Handle a Parallelogram opcode.
  if (opcode >= 0x34) {
    for (int i = 1; i <= 4; i++) {  // Loop 1 ..= 4, not 0 ..= 3.
      ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(
          c,                                                       //
          (p->coords[i & 3][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
          (p->coords[i & 3][1] * p->s2d_scale_y) + p->s2d_bias_y));
    }
    return NULL;
  }

  // The ellipse approximation's cubic Bézier points are described at
  // https://nigeltao.github.io/blog/2021/three-points-define-ellipse.html

  double center[2];
  center[0] = (p->coords[0][0] + p->coords[2][0]) / 2;
  center[1] = (p->coords[0][1] + p->coords[2][1]) / 2;
  const double k = 0.551784777779014;
  double kr[2];
  kr[0] = k * (p->coords[1][0] - center[0]);
  kr[1] = k * (p->coords[1][1] - center[1]);
  double ks[2];
  ks[0] = k * (p->coords[2][0] - center[0]);
  ks[1] = k * (p->coords[2][1] - center[1]);

  double imps[12][2];  // A+ B- B,   B+ C- C,   C+ D- D,   D+ A- A.
  imps[0][0] = p->coords[0][0] + kr[0];
  imps[0][1] = p->coords[0][1] + kr[1];
  imps[1][0] = p->coords[1][0] - ks[0];
  imps[1][1] = p->coords[1][1] - ks[1];
  imps[2][0] = p->coords[1][0];
  imps[2][1] = p->coords[1][1];
  imps[3][0] = p->coords[1][0] + ks[0];
  imps[3][1] = p->coords[1][1] + ks[1];
  imps[4][0] = p->coords[2][0] + kr[0];
  imps[4][1] = p->coords[2][1] + kr[1];
  imps[5][0] = p->coords[2][0];
  imps[5][1] = p->coords[2][1];
  imps[6][0] = p->coords[2][0] - kr[0];
  imps[6][1] = p->coords[2][1] - kr[1];
  imps[7][0] = p->coords[3][0] + ks[0];
  imps[7][1] = p->coords[3][1] + ks[1];
  imps[8][0] = p->coords[3][0];
  imps[8][1] = p->coords[3][1];
  imps[9][0] = p->coords[3][0] - ks[0];
  imps[9][1] = p->coords[3][1] - ks[1];
  imps[10][0] = p->coords[0][0] - kr[0];
  imps[10][1] = p->coords[0][1] - kr[1];
  imps[11][0] = p->coords[0][0];
  imps[11][1] = p->coords[0][1];

  for (size_t i = 0; i <= (opcode & 3); i++) {
    ICONVG_PRIVATE_TRY((*c->vtable->path_cube_to)(
        c,                                                        //
        (imps[(3 * i) + 0][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
        (imps[(3 * i) + 0][1] * p->s2d_scale_y) + p->s2d_bias_y,  //
        (imps[(3 * i) + 1][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
        (imps[(3 * i) + 1][1] * p->s2d_scale_y) + p->s2d_bias_y,  //
        (imps[(3 * i) + 2][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
        (imps[(3 * i) + 2][1] * p->s2d_scale_y) + p->s2d_bias_y));
    p->coords[0][0] = imps[(3 * i) + 2][0];
    p->coords[0][1] = imps[(3 * i) + 2][1];
  }
  return NULL;
}

static const char*  //
iconvg_private_expand_jump(iconvg_private_decoder* d,
                           iconvg_paint* p,
                           uint8_t opcode) {
  uint32_t jump_distance = 0;
  if (!iconvg_private_decoder__decode_natural_number(d, &jump_distance)) {
    return iconvg_error_bad_number;
  }

  if (opcode == 0x39) {  // Jump Feature-Bits.
    uint32_t feature_bits = 0;
    if (!iconvg_private_decoder__decode_natural_number(d, &feature_bits)) {
      return iconvg_error_bad_number;
    }
    // This decoder doesn't support any optional features (optional in terms of
    // the file format), so we always jump unless feature_bits is zero.
    if (feature_bits == 0) {
      return NULL;
    }

  } else if (opcode == 0x3A) {  // Jump Level-of-Detail.
    float lod[2] = {0};
    if (!iconvg_private_decoder__decode_coordinates(d, lod, 2)) {
      return iconvg_error_bad_number;
    }
    double h = p->height_in_pixels;
    if ((lod[0] <= h) && (h < lod[1])) {
      return NULL;
    }
  }

  for (; jump_distance > 0; jump_distance--) {
    if (d->len == 0) {
      return iconvg_error_bad_jump;
    }
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    uint32_t num_bytes = 0;
    uint64_t num_naturals = 0;
    bool inline_file_segment = false;

    if (opcode < 0x30) {
      uint32_t num_reps = opcode & 15;
      if (num_reps == 0) {
        if (!iconvg_private_decoder__decode_natural_number(d, &num_reps)) {
          return iconvg_error_bad_jump;
        }
        num_reps += 16;
      }
      uint64_t coordinate_pairs_per_rep = 1 + (opcode >> 4);
      num_naturals = ((uint64_t)num_reps) * 2 * coordinate_pairs_per_rep;

    } else if (opcode < 0x3C) {
      static const uint8_t nums[16] = {
          0x04, 0x04, 0x04, 0x04,  // Ellipse ops.
          0x04, 0x02, 0x10, 0x00,  // Parallelogram, MoveTo, SEL += arg, NOP.
          0x01, 0x02, 0x03, 0x00,  // Jump ops.
          0x00, 0x00, 0x00, 0x00,  // Call ops are handled separately, below.
      };
      num_bytes = nums[opcode & 15] >> 4;
      num_naturals = nums[opcode & 15] & 15;

    } else if (opcode < 0x40) {
      if (opcode & 1) {
        num_bytes += 25;
      }
      if (opcode & 2) {
        num_bytes += 8;
      } else {
        inline_file_segment = true;
      }

    } else if (opcode < 0x60) {
      num_bytes = 4;

    } else if (opcode < 0x70) {
      num_bytes = 8;

    } else if (opcode < 0x80) {
      num_bytes = 8 * (2 + (opcode & 15));

    } else if (opcode < 0x90) {
      continue;

    } else if (opcode < 0xA0) {
      num_bytes = 13;

    } else if (opcode < 0xB0) {
      num_bytes = 25;

    } else {
      if (!iconvg_private_decoder__decode_natural_number(d, &num_bytes)) {
        return iconvg_error_bad_jump;
      }
      if ((0xC0 <= opcode) && (opcode < 0xE0)) {
        num_naturals = 2;
      }
    }

    if (d->len < num_bytes) {
      return iconvg_error_bad_jump;
    }
    d->ptr += num_bytes;
    d->len -= num_bytes;

    for (; num_naturals > 0; num_naturals--) {
      uint32_t dummy;
      if (!iconvg_private_decoder__decode_natural_number(d, &dummy)) {
        return iconvg_error_bad_jump;
      }
    }

    if (inline_file_segment) {
      if (d->len < 4) {
        return iconvg_error_bad_jump;
      }
      uint32_t n = 4 + (iconvg_private_peek_u32le(d->ptr) >> 8);
      if (d->len < n) {
        return iconvg_error_bad_jump;
      }
      d->ptr += n;
      d->len -= n;
    }
  }

  return NULL;
}

// ----

static const char*  //
iconvg_private_execute_bytecode(iconvg_canvas* c,
                                iconvg_private_decoder* d,
                                iconvg_paint* p) {
  while (d->len > 0) {
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    switch (opcode >> 6) {
      case 0: {  // Path and miscellaneous ops.
        if (opcode >= 0x36) {
          if (opcode == 0x36) {  // SEL += arg.
            if (d->len == 0) {
              return iconvg_error_bad_number;
            }
            p->sel += d->ptr[0];
            d->ptr += 1;
            d->len -= 1;
            continue;
          } else if (opcode == 0x37) {  // NOP.
            continue;
          } else if (opcode < 0x3B) {  // Jump ops.
            ICONVG_PRIVATE_TRY(iconvg_private_expand_jump(d, p, opcode));
            continue;
          } else if (opcode > 0x3B) {  // Call ops.
            ICONVG_PRIVATE_TRY(iconvg_private_expand_call(c, d, p, opcode));
            continue;
          }
          // RET.
          return NULL;
        }

        if (!p->begun_drawing) {
          p->begun_drawing = true;
          ICONVG_PRIVATE_TRY((*c->vtable->begin_drawing)(c));
        }

        if (opcode == 0x35) {
          if (!iconvg_private_decoder__decode_coordinates(d, p->coords[0], 2)) {
            return iconvg_error_bad_coordinate;
          }
          if (p->begun_path) {
            ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
          } else {
            p->begun_path = true;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(
              c,                                                   //
              (p->coords[0][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[0][1] * p->s2d_scale_y) + p->s2d_bias_y));
          continue;
        }

        if (!p->begun_path) {
          p->begun_path = true;
          ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(
              c,                                                   //
              (p->coords[0][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[0][1] * p->s2d_scale_y) + p->s2d_bias_y));
        }

        if (opcode >= 0x30) {
          ICONVG_PRIVATE_TRY(
              iconvg_private_expand_ellipse_parallelogram(c, d, p, opcode));
          continue;
        }

        uint32_t num_reps = opcode & 15;
        if (num_reps == 0) {
          if (!iconvg_private_decoder__decode_natural_number(d, &num_reps)) {
            return iconvg_error_bad_number;
          }
          num_reps += 16;
        }

        switch (opcode >> 4) {
          case 0:
            for (; num_reps > 0; num_reps--) {
              if (!iconvg_private_decoder__decode_coordinates(d, p->coords[1],
                                                              2)) {
                return iconvg_error_bad_coordinate;
              }
              ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(
                  c,                                                   //
                  (p->coords[1][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
                  (p->coords[1][1] * p->s2d_scale_y) + p->s2d_bias_y));
            }
            p->coords[0][0] = p->coords[1][0];
            p->coords[0][1] = p->coords[1][1];
            continue;

          case 1:
            for (; num_reps > 0; num_reps--) {
              if (!iconvg_private_decoder__decode_coordinates(d, p->coords[1],
                                                              4)) {
                return iconvg_error_bad_coordinate;
              }
              ICONVG_PRIVATE_TRY((*c->vtable->path_quad_to)(
                  c,                                                   //
                  (p->coords[1][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
                  (p->coords[1][1] * p->s2d_scale_y) + p->s2d_bias_y,  //
                  (p->coords[2][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
                  (p->coords[2][1] * p->s2d_scale_y) + p->s2d_bias_y));
            }
            p->coords[0][0] = p->coords[2][0];
            p->coords[0][1] = p->coords[2][1];
            continue;
        }

        for (; num_reps > 0; num_reps--) {
          if (!iconvg_private_decoder__decode_coordinates(d, p->coords[1], 6)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->path_cube_to)(
              c,                                                   //
              (p->coords[1][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[1][1] * p->s2d_scale_y) + p->s2d_bias_y,  //
              (p->coords[2][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[2][1] * p->s2d_scale_y) + p->s2d_bias_y,  //
              (p->coords[3][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[3][1] * p->s2d_scale_y) + p->s2d_bias_y));
        }
        p->coords[0][0] = p->coords[3][0];
        p->coords[0][1] = p->coords[3][1];
        continue;
      }

      case 1: {  // Register ops.
        uint32_t adj = opcode & 15;
        switch ((opcode >> 4) & 3) {
          case 0:
            if (d->len < 4) {
              return iconvg_error_bad_number;
            }
            p->regs[(p->sel + adj) & 63] =
                ((uint64_t)iconvg_private_peek_u32le(d->ptr));
            d->ptr += 4;
            d->len -= 4;
            break;
          case 1:
            if (d->len < 4) {
              return iconvg_error_bad_number;
            }
            p->regs[(p->sel + adj) & 63] =
                ((uint64_t)iconvg_private_peek_u32le(d->ptr)) << 32;
            d->ptr += 4;
            d->len -= 4;
            break;
          case 2:
            if (d->len < 8) {
              return iconvg_error_bad_number;
            }
            p->regs[(p->sel + adj) & 63] = iconvg_private_peek_u64le(d->ptr);
            d->ptr += 8;
            d->len -= 8;
            break;
          default:
            adj += 2;
            p->sel -= adj;
            for (uint32_t i = 1; i <= adj; i++) {
              if (d->len < 8) {
                return iconvg_error_bad_number;
              }
              p->regs[(p->sel + i) & 63] = iconvg_private_peek_u64le(d->ptr);
              d->ptr += 8;
              d->len -= 8;
            }
            continue;
        }
        p->sel -= (adj == 0) ? 1 : 0;
        continue;
      }

      case 2: {  // Fill ops.
        uint32_t adj = opcode & 15;
        p->sel += (adj == 0) ? 1 : 0;
        uint32_t num_transforms = 0;

        switch ((opcode >> 4) & 3) {
          case 0:
            p->paint_type = (uint8_t)ICONVG_PAINT_TYPE__FLAT_COLOR;
            break;
          case 1:
            p->paint_type = (uint8_t)ICONVG_PAINT_TYPE__LINEAR_GRADIENT;
            p->transform[3] = 0.0f;
            p->transform[4] = 0.0f;
            p->transform[5] = 0.0f;
            num_transforms = 3;
            break;
          case 2:
            p->paint_type = (uint8_t)ICONVG_PAINT_TYPE__RADIAL_GRADIENT;
            num_transforms = 6;
            break;
          case 3: {
            p->paint_type = (uint8_t)ICONVG_PAINT_TYPE__FLAT_COLOR;
            uint32_t num_bytes = 0;
            if (!iconvg_private_decoder__decode_natural_number(d, &num_bytes)) {
              return iconvg_error_bad_number;
            }
            if (d->len < num_bytes) {
              return iconvg_error_bad_opcode_length;
            }
            d->ptr += num_bytes;
            d->len -= num_bytes;
            break;
          }
        }
        p->which_regs = (uint8_t)(p->sel + adj);

        if (num_transforms > 0) {
          if (d->len == 0) {
            return iconvg_error_bad_opcode_length;
          }
          p->num_stops = (d->ptr[0] & 63) + 2;
          p->spread = d->ptr[0] >> 6;
          d->ptr += 1;
          d->len -= 1;
          if (p->num_stops > 64) {
            return iconvg_error_bad_opcode_length;
          }
          for (uint32_t i = 0; i < num_transforms; i++) {
            if (!iconvg_private_decoder__decode_float32(d, &p->transform[i])) {
              return iconvg_error_bad_number;
            }
          }
        }

        if (p->begun_path) {
          p->begun_path = false;
          ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        }
        if (p->begun_drawing) {
          p->begun_drawing = false;
          ICONVG_PRIVATE_TRY((*c->vtable->end_drawing)(c, p));
        }
        continue;
      }

      case 3: {  // Reserved ops.
        uint32_t num_bytes = 0;
        if (!iconvg_private_decoder__decode_natural_number(d, &num_bytes)) {
          return iconvg_error_bad_number;
        }
        if (d->len < num_bytes) {
          return iconvg_error_bad_opcode_length;
        }
        d->ptr += num_bytes;
        d->len -= num_bytes;
        if (opcode < 0xE0) {
          if (!iconvg_private_decoder__decode_coordinates(d, p->coords[1], 2)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(
              c,                                                   //
              (p->coords[1][0] * p->s2d_scale_x) + p->s2d_bias_x,  //
              (p->coords[1][1] * p->s2d_scale_y) + p->s2d_bias_y));
          p->coords[0][0] = p->coords[1][0];
          p->coords[0][1] = p->coords[1][1];
        }
      }
    }
  }
  return NULL;
}

// ----

static void  //
iconvg_private_initialize_remaining_paint_fields(iconvg_paint* p,
                                                 iconvg_rectangle_f32 r) {
  double rw = iconvg_rectangle_f32__width_f64(&r);
  double rh = iconvg_rectangle_f32__height_f64(&r);
  double vw = iconvg_rectangle_f32__width_f64(&p->viewbox);
  double vh = iconvg_rectangle_f32__height_f64(&p->viewbox);
  if ((rw > 0) && (rh > 0) && (vw > 0) && (vh > 0)) {
    p->s2d_scale_x = rw / vw;
    p->s2d_scale_y = rh / vh;
    p->s2d_bias_x = r.min_x - (p->viewbox.min_x * p->s2d_scale_x);
    p->s2d_bias_y = r.min_y - (p->viewbox.min_y * p->s2d_scale_y);
  } else {
    p->s2d_scale_x = 1.0;
    p->s2d_bias_x = 0.0;
    p->s2d_scale_y = 1.0;
    p->s2d_bias_y = 0.0;
  }

  p->d2s_scale_x = 1.0 / p->s2d_scale_x;
  p->d2s_bias_x = -p->s2d_bias_x * p->d2s_scale_x;
  p->d2s_scale_y = 1.0 / p->s2d_scale_y;
  p->d2s_bias_y = -p->s2d_bias_y * p->d2s_scale_y;

  p->sel = 56;
  p->begun_drawing = false;
  p->begun_path = false;

  p->paint_type = 0;
  p->num_stops = 0;
  p->spread = 0;
  p->which_regs = 0;

  p->coords[0][0] = 0.0f;
  p->coords[0][1] = 0.0f;
  p->coords[1][0] = 0.0f;
  p->coords[1][1] = 0.0f;
  p->coords[2][0] = 0.0f;
  p->coords[2][1] = 0.0f;
  p->coords[3][0] = 0.0f;
  p->coords[3][1] = 0.0f;

  for (int i = 0; i < 64; i++) {
    uint32_t u =
        iconvg_private_peek_u32le(&p->custom_palette.colors[i].rgba[0]);
    p->regs[i] = ((uint64_t)u) << 32;
  }
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

    if (metadata_id == 8) {  // MID 8 (ViewBox).
      use_default_viewbox = false;
      iconvg_rectangle_f32 r;
      if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk, &r) ||
          (chunk.len != 0)) {
        return iconvg_error_bad_metadata_viewbox;
      } else if (dst_viewbox) {
        *dst_viewbox = r;
      }
      return NULL;
    } else if (metadata_id > 8) {
      // MIDs should appear in increasing order.
      break;
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
  iconvg_paint p;
  p.viewbox = iconvg_private_default_viewbox();
  if (options && options->height_in_pixels.has_value) {
    p.height_in_pixels = options->height_in_pixels.value;
  } else {
    double h = iconvg_rectangle_f32__height_f64(&r);
    // The 0x10_0000 = (1 << 20) = 1048576 limit is arbitrary but it's less
    // than MAX_INT32 and also ensures that conversion between integer and
    // float or double is lossless.
    if (h <= 0x100000) {
      p.height_in_pixels = (int64_t)h;
    } else {
      p.height_in_pixels = 0x100000;
    }
  }
  memcpy(&p.custom_palette, &iconvg_private_default_palette,
         sizeof(p.custom_palette));

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
      case 8:  // MID 8 (ViewBox).
        if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk,
                                                             &p.viewbox) ||
            (chunk.len != 0)) {
          return iconvg_error_bad_metadata_viewbox;
        }
        break;

      case 16:  // MID 16 (Suggested Palette).
        if (!iconvg_private_decoder__decode_metadata_suggested_palette(
                &chunk, &p.custom_palette) ||
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

  ICONVG_PRIVATE_TRY((*c->vtable->on_metadata_viewbox)(c, p.viewbox));
  ICONVG_PRIVATE_TRY(
      (*c->vtable->on_metadata_suggested_palette)(c, &p.custom_palette));

  if (options && options->palette) {
    memcpy(&p.custom_palette, options->palette, sizeof(p.custom_palette));
  }

  iconvg_private_initialize_remaining_paint_fields(&p, r);
  return iconvg_private_execute_bytecode(c, d, &p);
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
    return iconvg_error_invalid_vtable;
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

const char iconvg_error_bad_coordinate[] =  //
    "iconvg: bad coordinate";
const char iconvg_error_bad_jump[] =  //
    "iconvg: bad jump";
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
const char iconvg_error_bad_opcode_length[] =  //
    "iconvg: bad opcode length";

const char iconvg_error_system_failure_out_of_memory[] =  //
    "iconvg: system failure: out of memory";

const char iconvg_error_invalid_backend_not_enabled[] =  //
    "iconvg: invalid backend (not enabled)";
const char iconvg_error_invalid_constructor_argument[] =  //
    "iconvg: invalid constructor argument";
const char iconvg_error_invalid_paint_type[] =  //
    "iconvg: invalid paint type";
const char iconvg_error_invalid_vtable[] =  //
    "iconvg: invalid vtable";

// ----

bool  //
iconvg_error_is_file_format_error(const char* err_msg) {
  return (err_msg == iconvg_error_bad_coordinate) ||
         (err_msg == iconvg_error_bad_jump) ||
         (err_msg == iconvg_error_bad_magic_identifier) ||
         (err_msg == iconvg_error_bad_metadata) ||
         (err_msg == iconvg_error_bad_metadata_id_order) ||
         (err_msg == iconvg_error_bad_metadata_suggested_palette) ||
         (err_msg == iconvg_error_bad_metadata_viewbox) ||
         (err_msg == iconvg_error_bad_number) ||
         (err_msg == iconvg_error_bad_opcode_length);
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
  return self ? ((iconvg_paint_type)(self->paint_type))
              : ICONVG_PAINT_TYPE__INVALID;
}

// ----

static inline iconvg_nonpremul_color  //
iconvg_private_flat_color_as_nonpremul_color(uint32_t u) {
  uint32_t ur = 0xFF & (u >> 0);
  uint32_t ug = 0xFF & (u >> 8);
  uint32_t ub = 0xFF & (u >> 16);
  uint32_t ua = 0xFF & (u >> 24);
  iconvg_nonpremul_color k;
  if (ua == 0) {
    k.rgba[0] = 0;
    k.rgba[1] = 0;
    k.rgba[2] = 0;
    k.rgba[3] = 0;
  } else if (ua == 0xFF) {
    k.rgba[0] = (uint8_t)ur;
    k.rgba[1] = (uint8_t)ug;
    k.rgba[2] = (uint8_t)ub;
    k.rgba[3] = (uint8_t)ua;
  } else {
    k.rgba[0] = (uint8_t)((ur * 0xFF) / ua);
    k.rgba[1] = (uint8_t)((ug * 0xFF) / ua);
    k.rgba[2] = (uint8_t)((ub * 0xFF) / ua);
    k.rgba[3] = (uint8_t)(ua);
  }
  return k;
}

static inline iconvg_premul_color  //
iconvg_private_flat_color_as_premul_color(uint32_t u) {
  iconvg_premul_color k;
  k.rgba[0] = (uint8_t)(u >> 0);
  k.rgba[1] = (uint8_t)(u >> 8);
  k.rgba[2] = (uint8_t)(u >> 16);
  k.rgba[3] = (uint8_t)(u >> 24);
  return k;
}

// ----

static uint32_t  //
iconvg_private_paint__resolve_nonrecursive(const iconvg_paint* self,
                                           uint32_t i) {
  uint32_t u = (uint32_t)(self->regs[i & 63] >> 32);
  uint32_t ur = 0xFF & (u >> 0);
  uint32_t ug = 0xFF & (u >> 8);
  uint32_t ub = 0xFF & (u >> 16);
  uint32_t ua = 0xFF & (u >> 24);
  if ((ur <= ua) && (ug <= ua) && (ub <= ua)) {
    return u;
  }
  return 0;
}

static uint32_t  //
iconvg_private_paint__one_byte_color(const iconvg_paint* self,
                                     uint32_t i,
                                     uint32_t u) {
  if (u < 0x80) {
    return iconvg_private_one_byte_colors[u];
  } else if (u < 0xC0) {
    return iconvg_private_peek_u32le(
        &self->custom_palette.colors[u & 63].rgba[0]);
  }
  return iconvg_private_paint__resolve_nonrecursive(self, i + u);
}

static uint32_t  //
iconvg_private_paint__resolve(const iconvg_paint* self, uint32_t i) {
  uint32_t u = (uint32_t)(self->regs[i & 63] >> 32);
  uint32_t ur = 0xFF & (u >> 0);
  uint32_t ug = 0xFF & (u >> 8);
  uint32_t ub = 0xFF & (u >> 16);
  uint32_t ua = 0xFF & (u >> 24);
  if ((ur <= ua) && (ug <= ua) && (ub <= ua)) {
    return u;
  } else if (ua != 0) {
    return 0;
  }

  uint32_t p_blend = 255 - ur;
  uint32_t p = iconvg_private_paint__one_byte_color(self, i, ug);
  uint32_t pr = 0xFF & (p >> 0);
  uint32_t pg = 0xFF & (p >> 8);
  uint32_t pb = 0xFF & (p >> 16);
  uint32_t pa = 0xFF & (p >> 24);

  uint32_t q_blend = ur;
  uint32_t q = iconvg_private_paint__one_byte_color(self, i, ub);
  uint32_t qr = 0xFF & (q >> 0);
  uint32_t qg = 0xFF & (q >> 8);
  uint32_t qb = 0xFF & (q >> 16);
  uint32_t qa = 0xFF & (q >> 24);

  ur = ((p_blend * pr) + (q_blend * qr) + 128) / 255;
  ug = ((p_blend * pg) + (q_blend * qg) + 128) / 255;
  ub = ((p_blend * pb) + (q_blend * qb) + 128) / 255;
  ua = ((p_blend * pa) + (q_blend * qa) + 128) / 255;

  return (ur << 0) | (ug << 8) | (ub << 16) | (ua << 24);
}

// ----

iconvg_nonpremul_color  //
iconvg_paint__flat_color_as_nonpremul_color(const iconvg_paint* self) {
  return iconvg_private_flat_color_as_nonpremul_color(
      self ? iconvg_private_paint__resolve(self, self->which_regs) : 0);
}

iconvg_premul_color  //
iconvg_paint__flat_color_as_premul_color(const iconvg_paint* self) {
  return iconvg_private_flat_color_as_premul_color(
      self ? iconvg_private_paint__resolve(self, self->which_regs) : 0);
}

// ----

iconvg_gradient_spread  //
iconvg_paint__gradient_spread(const iconvg_paint* self) {
  return self ? ((iconvg_gradient_spread)(self->spread)) : 0;
}

uint32_t  //
iconvg_paint__gradient_number_of_stops(const iconvg_paint* self) {
  return self ? self->num_stops : 0;
}

iconvg_nonpremul_color  //
iconvg_paint__gradient_stop_color_as_nonpremul_color(const iconvg_paint* self,
                                                     uint32_t which_stop) {
  uint32_t i = self->which_regs + which_stop;
  uint32_t u = ((uint32_t)(self->regs[i & 63] >> 32));
  return iconvg_private_flat_color_as_nonpremul_color(u);
}

iconvg_premul_color  //
iconvg_paint__gradient_stop_color_as_premul_color(const iconvg_paint* self,
                                                  uint32_t which_stop) {
  uint32_t i = self->which_regs + which_stop;
  uint32_t u = ((uint32_t)(self->regs[i & 63] >> 32));
  return iconvg_private_flat_color_as_premul_color(u);
}

float  //
iconvg_paint__gradient_stop_offset(const iconvg_paint* self,
                                   uint32_t which_stop) {
  if (!self) {
    return 0.0f;
  }
  uint32_t i = self->which_regs + which_stop;
  uint32_t u = ((uint32_t)(self->regs[i & 63]));
  return (u >= 0x10000) ? 1.0f : (((float)u) / 0x10000);
}

iconvg_matrix_2x3_f64  //
iconvg_paint__gradient_transformation_matrix(const iconvg_paint* self) {
  if (!self) {
    return iconvg_matrix_2x3_f64__make(1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  }

  double s00 = self->transform[0];
  double s01 = self->transform[1];
  double s02 = self->transform[2];
  double s10 = self->transform[3];
  double s11 = self->transform[4];
  double s12 = self->transform[5];

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
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
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
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr2 = NULL;
  }
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
  sk_canvas_restore(sc);
  return err_msg;
}

static const char*  //
iconvg_private_skia_canvas__begin_drawing(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr2 = NULL;
  }
  spb = sk_pathbuilder_new();
  if (!spb) {
    return iconvg_error_system_failure_out_of_memory;
  }
  c->context_nonconst_ptr2 = spb;
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_drawing(iconvg_canvas* c,
                                        const iconvg_paint* p) {
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);

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
    c->context_nonconst_ptr2 = NULL;
  }
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_move_to(spb, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_path(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_close(spb);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_line_to(iconvg_canvas* c, float x1, float y1) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_line_to(spb, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_quad_to(iconvg_canvas* c,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
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
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
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
  memset(&c.context, 0, sizeof(c.context));
  c.context.nonconst_ptr1 = sc;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_SKIA_BACKEND

#endif  // ICONVG_IMPLEMENTATION

#endif  // ICONVG_INCLUDE_GUARD
