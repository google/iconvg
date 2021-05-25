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

iconvg_matrix_2x3_f64  //
iconvg_matrix_2x3_f64__inverse(iconvg_matrix_2x3_f64* self) {
  double inv = 1.0 / iconvg_matrix_2x3_f64__determinant(self);
  if (isinf(inv) || isnan(inv)) {
    return iconvg_make_matrix_2x3_f64(1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  }

  // https://ardoris.wordpress.com/2008/07/18/general-formula-for-the-inverse-of-a-3x3-matrix/
  // recalling that self's implicit bottom row is [0, 0, 1].
  double e02 = (self->elems[0][1] * self->elems[1][2]) -
               (self->elems[0][2] * self->elems[1][1]);
  double e12 = (self->elems[0][0] * self->elems[1][2]) -
               (self->elems[0][2] * self->elems[1][0]);
  return iconvg_make_matrix_2x3_f64(+inv * self->elems[1][1],  //
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
