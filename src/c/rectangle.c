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

float  //
iconvg_rectangle__width(const iconvg_rectangle* self) {
  // Note that max_x or min_x may be NaN.
  if (self && (self->max_x > self->min_x)) {
    return self->max_x - self->min_x;
  }
  return 0.0f;
}

float  //
iconvg_rectangle__height(const iconvg_rectangle* self) {
  // Note that max_y or min_y may be NaN.
  if (self && (self->max_y > self->min_y)) {
    return self->max_y - self->min_y;
  }
  return 0.0f;
}
