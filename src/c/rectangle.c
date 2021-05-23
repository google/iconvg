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
