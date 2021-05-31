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

#if !defined(ICONVG_CONFIG__ENABLE_SKIA_BACKEND)

iconvg_canvas  //
iconvg_canvas__make_skia(sk_canvas_t* sc) {
  return iconvg_canvas__make_broken(iconvg_error_invalid_backend_not_enabled);
}

#else  // ICONVG_CONFIG__ENABLE_SKIA_BACKEND

#include "include/c/sk_canvas.h"
#include "include/c/sk_matrix.h"
#include "include/c/sk_paint.h"
#include "include/c/sk_path.h"
#include "include/c/sk_shader.h"

static const sk_shader_tilemode_t
    iconvg_private_gradient_spread_as_sk_shader_tilemode_t[4] = {
        CLAMP_SK_SHADER_TILEMODE,   //
        CLAMP_SK_SHADER_TILEMODE,   //
        MIRROR_SK_SHADER_TILEMODE,  //
        REPEAT_SK_SHADER_TILEMODE   //
};

// iconvg_private_skia_set_gradient_stops sets the Skia gradient stop colors
// given the IconVG gradient stop colors.
//
// Like iconvg_private_cairo_set_gradient_stops, the complexity is due to
// premultiplied versus non-premultiplied alpha.
//
// It returns the number of Skia stops added.
static uint32_t  //
iconvg_private_skia_set_gradient_stops(sk_color_t* gcol,
                                       float* goff,
                                       const iconvg_paint* p) {
  uint32_t ret = 0;

  // foo0 and foo2 are the previous and current gradient stop. Sometimes we
  // need to synthesize additional stops in between them, whose variables are
  // named foo1.
  double offset0 = 0.0;
  uint8_t r0 = 0x00;
  uint8_t g0 = 0x00;
  uint8_t b0 = 0x00;
  uint8_t a0 = 0x00;

  uint32_t num_stops = iconvg_paint__gradient_number_of_stops(p);
  for (uint32_t i = 0; i < num_stops; i++) {
    // Calculate offset and color for the current stop.
    double offset2 = iconvg_paint__gradient_stop_offset(p, i);
    iconvg_premul_color k =
        iconvg_paint__gradient_stop_color_as_premul_color(p, i);
    uint8_t r2 = k.rgba[0];
    uint8_t g2 = k.rgba[1];
    uint8_t b2 = k.rgba[2];
    uint8_t a2 = k.rgba[3];

    if ((i == 0) ||                        //
        ((a0 == 0xFF) && (a2 == 0xFF)) ||  //
        ((a0 == 0x00) && (a2 == 0x00))) {
      // If it's the first stop, or if we're interpolating from 100% to 100%
      // opaque or from 0% to 0% opaque, we don't have to worry about
      // premultiplied versus non-premultiplied alpha.
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret++;

    } else if (a0 == 0x00) {
      // If we're blending e.g. from transparent black to (partially) opaque
      // blue, insert "transparent blue" immediately after the previous
      // "transparent black".
      *gcol++ = sk_color_set_argb(0x00, r2, g2, b2);
      *goff++ = offset0;
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret += 2;

    } else if (a2 == 0x00) {
      // If we're blending e.g. from (partially) opaque blue to transparent
      // black, insert "transparent blue" immediately before the current
      // "transparent black".
      *gcol++ = sk_color_set_argb(0x00, r0, g0, b0);
      *goff++ = offset2;
      *gcol++ = sk_color_set_argb(a2, r2, g2, b2);
      *goff++ = offset2;
      ret += 2;

    } else {
      // Otherwise, fake "interpolate with premultiplied alpha" like
      // iconvg_private_cairo_set_gradient_stops does.
      const int32_t n = 16;
      for (int32_t i = (n - 1); i >= 0; i--) {
        int32_t j = n - i;
        double offset1 = ((i * offset0) + (j * offset2)) / n;
        uint8_t r1 = ((i * r0) + (j * r2)) / n;
        uint8_t g1 = ((i * g0) + (j * g2)) / n;
        uint8_t b1 = ((i * b0) + (j * b2)) / n;
        uint8_t a1 = ((i * a0) + (j * a2)) / n;
        if (a1 == 0x00) {
          *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
          *goff++ = offset1;
          ret++;
        } else {
          *gcol++ = sk_color_set_argb(a1,                 //
                                      (0xFF * r1) / a1,   //
                                      (0xFF * g1) / a1,   //
                                      (0xFF * b1) / a1);  //
          *goff++ = offset1;
          ret++;
        }
      }
    }

    // Update offset and color for the previous stop.
    offset0 = offset2;
    r0 = r2;
    g0 = g2;
    b0 = b2;
    a0 = a2;
  }

  return ret;
}

static const char*  //
iconvg_private_skia_canvas__begin_decode(iconvg_canvas* c,
                                         iconvg_rectangle_f32 dst_rect) {
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
  sk_canvas_save(sc);

  sk_rect_t rect;
  rect.left = dst_rect.min_x;
  rect.top = dst_rect.min_y;
  rect.right = dst_rect.max_x;
  rect.bottom = dst_rect.max_y;
  sk_canvas_clip_rect(sc, &rect);

  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_decode(iconvg_canvas* c,
                                       const char* err_msg,
                                       size_t num_bytes_consumed,
                                       size_t num_bytes_remaining) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr2 = NULL;
  }
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
  sk_canvas_restore(sc);
  return err_msg;
}

static const char*  //
iconvg_private_skia_canvas__begin_drawing(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr2 = NULL;
  }
  spb = sk_pathbuilder_new();
  if (!spb) {
    return iconvg_error_system_failure_out_of_memory;
  }
  c->context_nonconst_ptr2 = spb;
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_drawing(iconvg_canvas* c,
                                        const iconvg_paint* p) {
  sk_canvas_t* sc = (sk_canvas_t*)(c->context.nonconst_ptr1);
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);

  iconvg_paint_type paint_type = iconvg_paint__type(p);
  switch (paint_type) {
    case ICONVG_PAINT_TYPE__FLAT_COLOR: {
      iconvg_nonpremul_color k = iconvg_paint__flat_color_as_nonpremul_color(p);
      sk_path_t* path = sk_pathbuilder_detach_path(spb);
      sk_paint_t* paint = sk_paint_new();
      sk_paint_set_antialias(paint, true);
      sk_paint_set_color(
          paint, sk_color_set_argb(k.rgba[3], k.rgba[0], k.rgba[1], k.rgba[2]));
      sk_canvas_draw_path(sc, path, paint);
      sk_paint_delete(paint);
      sk_path_delete(path);
      return NULL;
    }
    case ICONVG_PAINT_TYPE__LINEAR_GRADIENT:
    case ICONVG_PAINT_TYPE__RADIAL_GRADIENT:
      break;
    default:
      return iconvg_error_invalid_paint_type;
  }

  // The matrix in IconVG's API converts from dst coordinate space to pattern
  // coordinate space. Skia's API is the other way around (matrix inversion).
  iconvg_matrix_2x3_f64 im = iconvg_paint__gradient_transformation_matrix(p);
  if (paint_type == ICONVG_PAINT_TYPE__LINEAR_GRADIENT) {
    iconvg_matrix_2x3_f64__override_second_row(&im);
  }
  im = iconvg_matrix_2x3_f64__inverse(&im);
  sk_matrix_t sm;
  sm.mat[0] = im.elems[0][0];
  sm.mat[1] = im.elems[0][1];
  sm.mat[2] = im.elems[0][2];
  sm.mat[3] = im.elems[1][0];
  sm.mat[4] = im.elems[1][1];
  sm.mat[5] = im.elems[1][2];
  sm.mat[6] = 0.0f;
  sm.mat[7] = 0.0f;
  sm.mat[8] = 1.0f;

  // The gradient is either:
  //   - linear, from (0, 0) to (1, 0), or
  //   - radial, centered at (0, 0).
  sk_point_t gradient_points[2];
  gradient_points[0].x = 0;
  gradient_points[0].y = 0;
  gradient_points[1].x = 1;
  gradient_points[1].y = 0;

  // Configure the gradient stops.
  //
  // Skia doesn't have NONE_SK_SHADER_TILEMODE. Use CLAMP_SK_SHADER_TILEMODE
  // instead, for IconVG's ICONVG_GRADIENT_SPREAD__NONE, adding a transparent
  // black gradient stop at both ends.
  //
  // 1010 equals ((63 * 16) + 2). 63 is the maximum (inclusive) number of
  // gradient stops. iconvg_private_skia_set_gradient_stops can expand each
  // IconVG stop to up to 16 Skia stops. There's also 2 extra stops if we
  // use the ICONVG_GRADIENT_SPREAD__NONE workaround.
  sk_color_t gradient_colors[1010];
  float gradient_offsets[1010];
  sk_color_t* gcol = &gradient_colors[0];
  float* goff = &gradient_offsets[0];
  iconvg_gradient_spread gradient_spread = iconvg_paint__gradient_spread(p);
  uint32_t gradient_num_stops = 0;
  if (gradient_spread == ICONVG_GRADIENT_SPREAD__NONE) {
    *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
    *goff++ = 0.0f;
    gradient_num_stops++;
  }
  {
    uint32_t additional_stops =
        iconvg_private_skia_set_gradient_stops(gcol, goff, p);
    gcol += additional_stops;
    goff += additional_stops;
    gradient_num_stops += additional_stops;
  }
  if (gradient_spread == ICONVG_GRADIENT_SPREAD__NONE) {
    *gcol++ = sk_color_set_argb(0x00, 0x00, 0x00, 0x00);
    *goff++ = 0.0f;
    gradient_num_stops++;
  }

  // Make the Skia shader.
  sk_shader_t* shader = NULL;
  if (paint_type == ICONVG_PAINT_TYPE__LINEAR_GRADIENT) {
    shader = sk_shader_new_linear_gradient(
        gradient_points, gradient_colors, gradient_offsets, gradient_num_stops,
        iconvg_private_gradient_spread_as_sk_shader_tilemode_t[gradient_spread],
        &sm);
  } else {
    static const float radius = 1.0f;
    shader = sk_shader_new_radial_gradient(
        gradient_points, radius, gradient_colors, gradient_offsets,
        gradient_num_stops,
        iconvg_private_gradient_spread_as_sk_shader_tilemode_t[gradient_spread],
        &sm);
  }

  // Use the Skia shader.
  if (shader) {
    sk_path_t* path = sk_pathbuilder_detach_path(spb);
    sk_paint_t* paint = sk_paint_new();
    sk_paint_set_antialias(paint, true);
    sk_paint_set_shader(paint, shader);
    sk_shader_unref(shader);
    sk_canvas_draw_path(sc, path, paint);
    sk_paint_delete(paint);
    sk_path_delete(path);
  }

  // Clean up.
  if (spb) {
    sk_pathbuilder_delete(spb);
    c->context_nonconst_ptr2 = NULL;
  }
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_move_to(spb, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__end_path(iconvg_canvas* c) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_close(spb);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_line_to(iconvg_canvas* c, float x1, float y1) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_line_to(spb, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_quad_to(iconvg_canvas* c,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_quad_to(spb, x1, y1, x2, y2);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__path_cube_to(iconvg_canvas* c,
                                         float x1,
                                         float y1,
                                         float x2,
                                         float y2,
                                         float x3,
                                         float y3) {
  sk_pathbuilder_t* spb = (sk_pathbuilder_t*)(c->context_nonconst_ptr2);
  sk_pathbuilder_cubic_to(spb, x1, y1, x2, y2, x3, y3);
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                iconvg_rectangle_f32 viewbox) {
  return NULL;
}

static const char*  //
iconvg_private_skia_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return NULL;
}

static const iconvg_canvas_vtable  //
    iconvg_private_skia_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_skia_canvas__begin_decode,
        &iconvg_private_skia_canvas__end_decode,
        &iconvg_private_skia_canvas__begin_drawing,
        &iconvg_private_skia_canvas__end_drawing,
        &iconvg_private_skia_canvas__begin_path,
        &iconvg_private_skia_canvas__end_path,
        &iconvg_private_skia_canvas__path_line_to,
        &iconvg_private_skia_canvas__path_quad_to,
        &iconvg_private_skia_canvas__path_cube_to,
        &iconvg_private_skia_canvas__on_metadata_viewbox,
        &iconvg_private_skia_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_canvas__make_skia(sk_canvas_t* sc) {
  if (!sc) {
    return iconvg_canvas__make_broken(
        iconvg_error_invalid_constructor_argument);
  }
  iconvg_canvas c;
  c.vtable = &iconvg_private_skia_canvas_vtable;
  memset(&c.context, 0, sizeof(c.context));
  c.context.nonconst_ptr1 = sc;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_SKIA_BACKEND
