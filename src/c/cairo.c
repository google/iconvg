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

#if !defined(ICONVG_CONFIG__ENABLE_CAIRO_BACKEND)

iconvg_canvas  //
iconvg_make_cairo_canvas(cairo_t* cr) {
  return iconvg_make_broken_canvas(iconvg_error_invalid_backend_not_enabled);
}

#else  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND

#include <cairo/cairo.h>

static const cairo_extend_t
    iconvg_private_gradient_spread_as_cairo_extend_t[4] = {
        CAIRO_EXTEND_NONE,     //
        CAIRO_EXTEND_PAD,      //
        CAIRO_EXTEND_REFLECT,  //
        CAIRO_EXTEND_REPEAT    //
};

static inline cairo_matrix_t  //
iconvg_private_matrix_2x3_f64_as_cairo_matrix_t(iconvg_matrix_2x3_f64 i) {
  cairo_matrix_t c;
  c.xx = i.elems[0][0];
  c.xy = i.elems[0][1];
  c.x0 = i.elems[0][2];
  c.yx = i.elems[1][0];
  c.yy = i.elems[1][1];
  c.y0 = i.elems[1][2];
  return c;
}

// iconvg_private_matrix_2x3_f64_as_cairo_matrix_t_override_bottom_row is like
// iconvg_private_matrix_2x3_f64_as_cairo_matrix_t but overrides the bottom row
// of the 2x3 transformation matrix.
//
// IconVG linear gradients range from x=0 to x=1 in pattern space, independent
// of y. The bottom row therefore doesn't matter (because it's "independent of
// y") and can be [0, 0, 0] in the IconVG file format. However, Cairo needs the
// matrix to be invertible, so we override the bottom row with dummy values,
// like [1, 0, 0] or [0, 1, 0], so that the matrix determinant ((c.xx * c.yy) -
// (c.xy * c.yx)) is non-zero.
static inline cairo_matrix_t  //
iconvg_private_matrix_2x3_f64_as_cairo_matrix_t_override_bottom_row(
    iconvg_matrix_2x3_f64 i) {
  cairo_matrix_t c;
  c.xx = i.elems[0][0];
  c.xy = i.elems[0][1];
  c.x0 = i.elems[0][2];

  if (c.xx != 0) {
    c.yx = 0.0;
    c.yy = 1.0;
    c.y0 = 0.0;
  } else if (c.xy != 0) {
    c.yx = 1.0;
    c.yy = 0.0;
    c.y0 = 0.0;
  } else {
    // 1e-10 is arbitrary but very small and squaring it still gives
    // something larger than FLT_MIN, approximately 1.175494e-38.
    c.xx = 1e-10;
    c.xy = 0.0;
    // cx0 is unchanged.
    c.yx = 0.0;
    c.yy = 1e-10;
    c.y0 = 0.0;
  }
  return c;
}

// iconvg_private_cairo_set_gradient_stops sets the Cairo gradient stop colors
// given the IconVG gradient stop colors.
//
// Unlike SVG, IconVG works solely with premultiplied alpha. In contrast,
// https://lists.freedesktop.org/archives/cairo/2006-June/007203.html says that
// Cairo accepts "both pre- and non-premultiplied colors in different parts of
// the API". Specifically, while CAIRO_FORMAT_ARGB32 is premultiplied, both
// cairo_set_source_rgba and cairo_pattern_add_color_stop_rgba are
// non-premultiplied.
//
// For flat colors, we can simply convert IconVG colors to non-premultiplied
// colors. Gradients are trickier (and hence this function is non-trivial)
// because IconVG interpolation should also happen in premultiplied alpha space
// (but Cairo interpolates in non-premultiplied alpha space). The mathematical
// halfway color between opaque bright red = RGBA(1, 0, 0, 1) and transparent
// black = RGBA(0, 0, 0, 0) is RGBA(½, 0, 0, ½). IconVG (premultiplied alpha)
// semantics are that this is a 50% opaque bright red, not a 50% opaque dark
// red. The halfway point still has 100% Saturation and 100% Value (in the HSV
// Hue Saturation Value sense). It just has smaller alpha.
//
// Some more discussion is at
// https://lists.freedesktop.org/archives/cairo/2021-May/029252.html
static void  //
iconvg_private_cairo_set_gradient_stops(cairo_pattern_t* cp,
                                        const iconvg_paint* p) {
  // foo0 and foo2 are the previous and current gradient stop. Sometimes we
  // need to synthesize additional stops in between them, whose variables are
  // named foo1.
  double offset0 = 0.0;
  double r0 = 0.0;
  double g0 = 0.0;
  double b0 = 0.0;
  double a0 = 0.0;

  uint32_t n = iconvg_paint__gradient_number_of_stops(p);
  for (uint32_t i = 0; i < n; i++) {
    // Calculate offset and color for the current stop.
    double offset2 = iconvg_paint__gradient_stop_offset(p, i);
    iconvg_premul_color k =
        iconvg_paint__gradient_stop_color_as_premul_color(p, i);
    double r2 = k.rgba[0] / 255.0;
    double g2 = k.rgba[1] / 255.0;
    double b2 = k.rgba[2] / 255.0;
    double a2 = k.rgba[3] / 255.0;

    if ((i == 0) ||                      //
        ((a0 == 1.0) && (a2 == 1.0)) ||  //
        ((a0 == 0.0) && (a2 == 0.0))) {
      // If it's the first stop, or if we're interpolating from 100% to 100%
      // opaque or from 0% to 0% opaque, we don't have to worry about
      // premultiplied versus non-premultiplied alpha.
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else if (a0 == 0.0) {
      // If we're blending e.g. from transparent black to (partially) opaque
      // blue, insert "transparent blue" immediately after the previous
      // "transparent black".
      cairo_pattern_add_color_stop_rgba(cp, offset0, r2, g2, b2, 0.0);
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else if (a2 == 0.0) {
      // If we're blending e.g. from (partially) opaque blue to transparent
      // black, insert "transparent blue" immediately before the current
      // "transparent black".
      cairo_pattern_add_color_stop_rgba(cp, offset2, r0, g0, b0, 0.0);
      cairo_pattern_add_color_stop_rgba(cp, offset2, r2, g2, b2, a2);

    } else {
      // Otherwise, fake "interpolate with premultiplied alpha" by synthesizing
      // n Cairo stops for this 1 IconVG stop. The n stops' colors are
      // calculated explicitly here, in premultiplied alpha space. We then let
      // Cairo do its thing in non-premultiplied alpha space. The difference
      // between n stops (interpolating non-premultiplied) and 1 stop
      // (interpolating premultiplied) will hopefully be imperceivable.
      const int32_t n = 16;
      for (int32_t i = (n - 1); i >= 0; i--) {
        int32_t j = n - i;
        double offset1 = ((i * offset0) + (j * offset2)) / n;
        double r1 = ((i * r0) + (j * r2)) / n;
        double g1 = ((i * g0) + (j * g2)) / n;
        double b1 = ((i * b0) + (j * b2)) / n;
        double a1 = ((i * a0) + (j * a2)) / n;
        if (a1 == 0.0) {
          cairo_pattern_add_color_stop_rgba(cp, offset1, 0, 0, 0, 0);
        } else {
          cairo_pattern_add_color_stop_rgba(cp, offset1,  //
                                            r1 / a1, g1 / a1, b1 / a1, a1);
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
}

static const char*  //
iconvg_private_cairo_canvas__begin_decode(iconvg_canvas* c,
                                          iconvg_rectangle_f32 dst_rect) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_save(cr);
  cairo_rectangle(cr, dst_rect.min_x, dst_rect.min_y,
                  iconvg_rectangle_f32__width_f64(&dst_rect),
                  iconvg_rectangle_f32__height_f64(&dst_rect));
  cairo_clip(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_decode(iconvg_canvas* c,
                                        const char* err_msg,
                                        size_t num_bytes_consumed,
                                        size_t num_bytes_remaining) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_restore(cr);
  return err_msg;
}

static const char*  //
iconvg_private_cairo_canvas__begin_drawing(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_new_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_drawing(iconvg_canvas* c,
                                         const iconvg_paint* p) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_pattern_t* cp = NULL;
  cairo_matrix_t cm = {0};

  switch (iconvg_paint__type(p)) {
    case ICONVG_PAINT_TYPE__FLAT_COLOR: {
      iconvg_nonpremul_color k = iconvg_paint__flat_color_as_nonpremul_color(p);
      cairo_set_source_rgba(cr, k.rgba[0] / 255.0, k.rgba[1] / 255.0,
                            k.rgba[2] / 255.0, k.rgba[3] / 255.0);
      cairo_fill(cr);
      return NULL;
    }

    case ICONVG_PAINT_TYPE__LINEAR_GRADIENT: {
      cp = cairo_pattern_create_linear(0, 0, 1, 0);
      cm = iconvg_private_matrix_2x3_f64_as_cairo_matrix_t_override_bottom_row(
          iconvg_paint__gradient_transformation_matrix(p));
      break;
    }

    case ICONVG_PAINT_TYPE__RADIAL_GRADIENT: {
      cp = cairo_pattern_create_radial(0, 0, 0, 0, 0, 1);
      cm = iconvg_private_matrix_2x3_f64_as_cairo_matrix_t(
          iconvg_paint__gradient_transformation_matrix(p));
      break;
    }

    default:
      return iconvg_error_invalid_paint_type;
  }

  cairo_pattern_set_matrix(cp, &cm);
  cairo_pattern_set_extend(cp, iconvg_private_gradient_spread_as_cairo_extend_t
                                   [iconvg_paint__gradient_spread(p)]);
  iconvg_private_cairo_set_gradient_stops(cp, p);
  if (cairo_pattern_status(cp) == CAIRO_STATUS_SUCCESS) {
    cairo_set_source(cr, cp);
  } else {
    // Substitute in a 50% transparent grayish purple so that "something is
    // wrong with the Cairo pattern" is hopefully visible without abandoning
    // the graphic entirely.
    cairo_set_source_rgba(cr, 0.75, 0.25, 0.75, 0.5);
  }

  cairo_fill(cr);
  cairo_pattern_destroy(cp);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__begin_path(iconvg_canvas* c, float x0, float y0) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_move_to(cr, x0, y0);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__end_path(iconvg_canvas* c) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_close_path(cr);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_line_to(iconvg_canvas* c,
                                          float x1,
                                          float y1) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_line_to(cr, x1, y1);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_quad_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  // Cairo doesn't have explicit support for quadratic Bézier curves, only
  // linear and cubic ones. However, a "Bézier curve of degree n can be
  // converted into a Bézier curve of degree n + 1 with the same shape", per
  // https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Degree_elevation
  //
  // Here, we perform "degree elevation" from [x0, x1, x2] to [X0, X1, X2, X3]
  // = [x0, ((⅓ * x0) + (⅔ * x1)), ((⅔ * x1) + (⅓ * x2)), c2] and likewise for
  // the y dimension.
  double X0;
  double Y0;
  cairo_get_current_point(cr, &X0, &Y0);
  double twice_x1 = ((double)x1) * 2;
  double twice_y1 = ((double)y1) * 2;
  double X3 = ((double)x2);
  double Y3 = ((double)y2);
  double X1 = (X0 + twice_x1) / 3;
  double Y1 = (Y0 + twice_y1) / 3;
  double X2 = (X3 + twice_x1) / 3;
  double Y2 = (Y3 + twice_y1) / 3;
  cairo_curve_to(cr, X1, Y1, X2, Y2, X3, Y3);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_cube_to(iconvg_canvas* c,
                                          float x1,
                                          float y1,
                                          float x2,
                                          float y2,
                                          float x3,
                                          float y3) {
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__path_arc_to(iconvg_canvas* c,
                                         float radius_x,
                                         float radius_y,
                                         float x_axis_rotation,
                                         bool large_arc,
                                         bool sweep,
                                         float final_x,
                                         float final_y) {
  // TODO: convert from SVG's parameterization to Cairo's. Until then, we
  // substitute in a placeholder cairo_line_to.
  cairo_t* cr = (cairo_t*)(c->context_nonconst_ptr0);
  cairo_line_to(cr, final_x, final_y);
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__on_metadata_viewbox(iconvg_canvas* c,
                                                 iconvg_rectangle_f32 viewbox) {
  return NULL;
}

static const char*  //
iconvg_private_cairo_canvas__on_metadata_suggested_palette(
    iconvg_canvas* c,
    const iconvg_palette* suggested_palette) {
  return NULL;
}

static const iconvg_canvas_vtable  //
    iconvg_private_cairo_canvas_vtable = {
        sizeof(iconvg_canvas_vtable),
        &iconvg_private_cairo_canvas__begin_decode,
        &iconvg_private_cairo_canvas__end_decode,
        &iconvg_private_cairo_canvas__begin_drawing,
        &iconvg_private_cairo_canvas__end_drawing,
        &iconvg_private_cairo_canvas__begin_path,
        &iconvg_private_cairo_canvas__end_path,
        &iconvg_private_cairo_canvas__path_line_to,
        &iconvg_private_cairo_canvas__path_quad_to,
        &iconvg_private_cairo_canvas__path_cube_to,
        &iconvg_private_cairo_canvas__path_arc_to,
        &iconvg_private_cairo_canvas__on_metadata_viewbox,
        &iconvg_private_cairo_canvas__on_metadata_suggested_palette,
};

iconvg_canvas  //
iconvg_make_cairo_canvas(cairo_t* cr) {
  if (!cr) {
    return iconvg_make_broken_canvas(iconvg_error_invalid_constructor_argument);
  }
  iconvg_canvas c;
  c.vtable = &iconvg_private_cairo_canvas_vtable;
  c.context_nonconst_ptr0 = cr;
  c.context_nonconst_ptr1 = NULL;
  c.context_const_ptr = NULL;
  c.context_extra = 0;
  return c;
}

#endif  // ICONVG_CONFIG__ENABLE_CAIRO_BACKEND
