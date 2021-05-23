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

static const char*  //
iconvg_private_broken_canvas__begin_decode(iconvg_canvas* c,
                                           iconvg_rectangle_f32 dst_rect) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_decode(iconvg_canvas* c,
                                         const char* err_msg,
                                         size_t num_bytes_consumed,
                                         size_t num_bytes_remaining) {
  return err_msg ? err_msg : ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__begin_drawing(iconvg_canvas* c) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_drawing(iconvg_canvas* c,
                                          const iconvg_paint* p) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__end_path(iconvg_canvas* c) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_line_to(iconvg_canvas* c,
                                           float x1,
                                           float y1) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_quad_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_cube_to(iconvg_canvas* c,
                                           float x1,
                                           float y1,
                                           float x2,
                                           float y2,
                                           float x3,
                                           float y3) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__path_arc_to(iconvg_canvas* c,
                                          float radius_x,
                                          float radius_y,
                                          float x_axis_rotation,
                                          bool large_arc,
                                          bool sweep,
                                          float final_x,
                                          float final_y) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_viewbox(
    iconvg_canvas* c,
    iconvg_rectangle_f32 viewbox) {
  return ((const char*)(c->context_const_ptr));
}

static const char*  //
iconvg_private_broken_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return ((const char*)(c->context_const_ptr));
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
        &iconvg_private_broken_canvas__path_arc_to,
        &iconvg_private_broken_canvas__on_metadata_viewbox,
        &iconvg_private_broken_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_broken_canvas(const char* err_msg) {
  iconvg_canvas c;
  c.vtable = &iconvg_private_broken_canvas_vtable;
  c.context_nonconst_ptr0 = NULL;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = err_msg;
  c.context_extra = 0;
  return c;
}

bool  //
iconvg_canvas__does_nothing(const iconvg_canvas* self) {
  return self && (self->vtable != NULL) &&
         (self->vtable != &iconvg_private_broken_canvas_vtable);
}
