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

bool  //
iconvg_paint__is_flat_color(const iconvg_paint* self) {
  if (!self) {
    return true;
  }
  const uint8_t* rgba = &self->paint_rgba[0];
  return (rgba[0] <= rgba[3]) &&  //
         (rgba[1] <= rgba[3]) &&  //
         (rgba[2] <= rgba[3]);
}

iconvg_nonpremul_color  //
iconvg_paint__flat_color_as_nonpremul_color(const iconvg_paint* self) {
  iconvg_nonpremul_color k;
  if (!self || (self->paint_rgba[3] == 0x00)) {
    memset(&k.rgba[0], 0, 4);
  } else if (self->paint_rgba[3] == 0xFF) {
    memcpy(&k.rgba[0], &self->paint_rgba[0], 4);
  } else {
    uint32_t a = self->paint_rgba[3];
    k.rgba[0] = ((uint8_t)(((uint32_t)(self->paint_rgba[0])) * 0xFF / a));
    k.rgba[1] = ((uint8_t)(((uint32_t)(self->paint_rgba[1])) * 0xFF / a));
    k.rgba[2] = ((uint8_t)(((uint32_t)(self->paint_rgba[2])) * 0xFF / a));
    k.rgba[3] = ((uint8_t)a);
  }
  return k;
}

iconvg_premul_color  //
iconvg_paint__flat_color_as_premul_color(const iconvg_paint* self) {
  iconvg_premul_color k;
  if (!self) {
    memset(&k.rgba[0], 0, 4);
  } else {
    memcpy(&k.rgba[0], &self->paint_rgba[0], 4);
  }
  return k;
}
