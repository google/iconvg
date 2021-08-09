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
