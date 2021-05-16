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
    return iconvg_make_matrix_2x3_f64(1, 0, 0, 0, 1, 0);
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
  // gradients are always centre=(0,0) and radius=1. We can't just return this
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

  return iconvg_make_matrix_2x3_f64(d00, d01, d02, d10, d11, d12);
}
