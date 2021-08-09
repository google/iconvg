#ifndef ICONVG_PRIVATE_INCLUDE_GUARD
#define ICONVG_PRIVATE_INCLUDE_GUARD

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
#include <string.h>

#include "./aaa_public.h"

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

extern const uint8_t iconvg_private_one_byte_colors[512];
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

#endif  // ICONVG_PRIVATE_INCLUDE_GUARD
