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
iconvg_private_debug_canvas__begin_decode(iconvg_canvas* c,
                                          iconvg_rectangle_f32 dst_rect) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_decode({%g, %g, %g, %g})\n",
            ((const char*)(c->context_const_ptr)), dst_rect.min_x,
            dst_rect.min_y, dst_rect.max_x, dst_rect.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_decode)(wrapped, dst_rect);
}

static const char*  //
iconvg_private_debug_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    const char* quote = err_msg ? "\"" : "";
    fprintf(f, "%send_decode(%s%s%s, %zu, %zu)\n",
            ((const char*)(c->context_const_ptr)), quote,
            err_msg ? err_msg : "NULL", quote, num_bytes_consumed,
            num_bytes_remaining);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return err_msg;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_decode)(wrapped, err_msg, num_bytes_consumed,
                                        num_bytes_remaining);
}

static const char*  //
iconvg_private_debug_canvas__begin_drawing(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_drawing()\n", ((const char*)(c->context_const_ptr)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_drawing)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  static const char* spread_names[4] = {"none", "pad", "reflect", "repeat"};

  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    switch (iconvg_paint__type(p)) {
      case ICONVG_PAINT_TYPE__FLAT_COLOR: {
        iconvg_premul_color k = iconvg_paint__flat_color_as_premul_color(p);
        fprintf(f, "%send_drawing(flat_color{%02X:%02X:%02X:%02X})\n",
                ((const char*)(c->context_const_ptr)), ((int)(k.rgba[0])),
                ((int)(k.rgba[1])), ((int)(k.rgba[2])), ((int)(k.rgba[3])));
        break;
      }

      case ICONVG_PAINT_TYPE__LINEAR_GRADIENT: {
        fprintf(f,
                "%send_drawing(linear_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context_const_ptr)),
                ((int)(iconvg_paint__gradient_number_of_stops(p))),
                spread_names[iconvg_paint__gradient_spread(p)]);
        break;
      }

      case ICONVG_PAINT_TYPE__RADIAL_GRADIENT: {
        fprintf(f,
                "%send_drawing(radial_gradient{nstops=%d, spread=%s, ...})\n",
                ((const char*)(c->context_const_ptr)),
                ((int)(iconvg_paint__gradient_number_of_stops(p))),
                spread_names[iconvg_paint__gradient_spread(p)]);
        break;
      }

      case ICONVG_PAINT_TYPE__INVALID:
      default: {
        return iconvg_error_invalid_paint_type;
      }
    }
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_drawing)(wrapped, p);
}

static const char*  //
iconvg_private_debug_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%sbegin_path(%g, %g)\n", ((const char*)(c->context_const_ptr)),
            x0, y0);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->begin_path)(wrapped, x0, y0);
}

static const char*  //
iconvg_private_debug_canvas__end_path(iconvg_canvas* c) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%send_path()\n", ((const char*)(c->context_const_ptr)));
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->end_path)(wrapped);
}

static const char*  //
iconvg_private_debug_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_line_to(%g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_line_to)(wrapped, x1, y1);
}

static const char*  //
iconvg_private_debug_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_quad_to(%g, %g, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1, x2, y2);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_quad_to)(wrapped, x1, y1, x2, y2);
}

static const char*  //
iconvg_private_debug_canvas__path_cube_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2,
                                          float x3,
                                          float y3) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_cube_to(%g, %g, %g, %g, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), x1, y1, x2, y2, x3, y3);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_cube_to)(wrapped, x1, y1, x2, y2, x3, y3);
}

static const char*  //
iconvg_private_debug_canvas__path_arc_to(iconvg_canvas* c,
                                         float radius_x,
                                         float radius_y,
                                         float x_axis_rotation,
                                         bool large_arc,
                                         bool sweep,
                                         float final_x,
                                         float final_y) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%spath_arc_to(%g, %g, %g, %d, %d, %g, %g)\n",
            ((const char*)(c->context_const_ptr)), radius_x, radius_y,
            x_axis_rotation, (int)large_arc, (int)sweep, final_x, final_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->path_arc_to)(wrapped, radius_x, radius_y,
                                         x_axis_rotation, large_arc, sweep,
                                         final_x, final_y);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                 iconvg_rectangle_f32 viewbox) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    fprintf(f, "%son_metadata_viewbox({%g, %g, %g, %g})\n",
            ((const char*)(c->context_const_ptr)), viewbox.min_x, viewbox.min_y,
            viewbox.max_x, viewbox.max_y);
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->on_metadata_viewbox)(wrapped, viewbox);
}

static const char*  //
iconvg_private_debug_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  FILE* f = (FILE*)(c->context_nonconst_ptr1);
  if (f) {
    int j = iconvg_private_last_color_that_isnt_opaque_black(suggested_palette);
    if (j < 0) {
      fprintf(f, "%son_metadata_suggested_palette(...)\n",
              ((const char*)(c->context_const_ptr)));
    } else {
      fprintf(f, "%son_metadata_suggested_palette(",
              ((const char*)(c->context_const_ptr)));
      for (int i = 0; i <= j; i++) {
        fprintf(f, "%02X:%02X:%02X:%02X%s",
                ((int)(suggested_palette->colors[i].rgba[0])),
                ((int)(suggested_palette->colors[i].rgba[1])),
                ((int)(suggested_palette->colors[i].rgba[2])),
                ((int)(suggested_palette->colors[i].rgba[3])),
                (i < 63) ? ", " : ")\n");
      }
      if (j < 63) {
        fprintf(f, "...)\n");
      } else {
        fprintf(f, ")\n");
      }
    }
  }
  iconvg_canvas* wrapped = (iconvg_canvas*)(c->context_nonconst_ptr0);
  if (!wrapped) {
    return NULL;
  } else if (iconvg_private_canvas_sizeof_vtable(wrapped) <
             sizeof(iconvg_canvas_vtable)) {
    return iconvg_error_unsupported_vtable;
  }
  return (*wrapped->vtable->on_metadata_suggested_palette)(wrapped,
                                                           suggested_palette);
}

static const iconvg_canvas_vtable  //
    iconvg_private_debug_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_debug_canvas__begin_decode,
        &iconvg_private_debug_canvas__end_decode,
        &iconvg_private_debug_canvas__begin_drawing,
        &iconvg_private_debug_canvas__end_drawing,
        &iconvg_private_debug_canvas__begin_path,
        &iconvg_private_debug_canvas__end_path,
        &iconvg_private_debug_canvas__path_line_to,
        &iconvg_private_debug_canvas__path_quad_to,
        &iconvg_private_debug_canvas__path_cube_to,
        &iconvg_private_debug_canvas__path_arc_to,
        &iconvg_private_debug_canvas__on_metadata_viewbox,
        &iconvg_private_debug_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_debug_canvas(FILE* f,
                         const char* message_prefix,
                         iconvg_canvas* wrapped) {
  if (wrapped && !wrapped->vtable) {
    wrapped = NULL;
  }
  iconvg_canvas c;
  c.vtable = &iconvg_private_debug_canvas_vtable;
  c.context_nonconst_ptr0 = wrapped;
  c.context_nonconst_ptr1 = f;
  c.context_const_ptr = message_prefix ? message_prefix : "";
  c.context_extra = 0;
  return c;
}
