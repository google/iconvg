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

#include "./aaa_private.h"

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
