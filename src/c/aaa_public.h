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
// Other errors (invalid_etc, null_etc, unsupported_etc) are typically
// programming errors instead of file format errors.

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
  const char* (*paint)(struct iconvg_canvas_struct* c, const iconvg_paint* p);
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
