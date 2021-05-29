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

// iconvg_private_angle returns the angle between two vectors u and v.
static inline double  //
iconvg_private_angle(double ux, double uy, double vx, double vy) {
  const double pi = 3.1415926535897932384626433832795028841972;  // π = τ/2

  double u_norm = sqrt((ux * ux) + (uy * uy));
  double v_norm = sqrt((vx * vx) + (vy * vy));
  double norm = u_norm * v_norm;
  double cosine = (ux * vx + uy * vy) / norm;
  double ret = 0.0;
  if (cosine <= -1.0) {
    ret = pi;
  } else if (cosine >= +1.0) {
    ret = 0.0;
  } else {
    ret = acos(cosine);
  }
  if ((ux * vy) < (uy * vx)) {
    return -ret;
  }
  return +ret;
}

static inline const char*  //
iconvg_private_path_arc_segment_to(iconvg_canvas* c,
                                   double scale_x,
                                   double bias_x,
                                   double scale_y,
                                   double bias_y,
                                   double cx,
                                   double cy,
                                   double theta1,
                                   double theta2,
                                   double rx,
                                   double ry,
                                   double cos_phi,
                                   double sin_phi) {
  double half_delta_theta = (theta2 - theta1) * 0.5;
  double q = sin(half_delta_theta * 0.5);
  double t = (8 * q * q) / (3 * sin(half_delta_theta));
  double cos1 = cos(theta1);
  double sin1 = sin(theta1);
  double cos2 = cos(theta2);
  double sin2 = sin(theta2);

  double ix1 = rx * (+cos1 - (t * sin1));
  double iy1 = ry * (+sin1 + (t * cos1));
  double ix2 = rx * (+cos2 + (t * sin2));
  double iy2 = ry * (+sin2 - (t * cos2));
  double ix3 = rx * (+cos2);
  double iy3 = ry * (+sin2);

  double jx1 = cx + (cos_phi * ix1) - (sin_phi * iy1);
  double jy1 = cy + (sin_phi * ix1) + (cos_phi * iy1);
  double jx2 = cx + (cos_phi * ix2) - (sin_phi * iy2);
  double jy2 = cy + (sin_phi * ix2) + (cos_phi * iy2);
  double jx3 = cx + (cos_phi * ix3) - (sin_phi * iy3);
  double jy3 = cy + (sin_phi * ix3) + (cos_phi * iy3);

  return (*c->vtable->path_cube_to)(c,                         //
                                    (jx1 * scale_x) + bias_x,  //
                                    (jy1 * scale_y) + bias_y,  //
                                    (jx2 * scale_x) + bias_x,  //
                                    (jy2 * scale_y) + bias_y,  //
                                    (jx3 * scale_x) + bias_x,  //
                                    (jy3 * scale_y) + bias_y);
}

const char*  //
iconvg_private_path_arc_to(iconvg_canvas* c,
                           double scale_x,
                           double bias_x,
                           double scale_y,
                           double bias_y,
                           float initial_x,
                           float initial_y,
                           float radius_x,
                           float radius_y,
                           float x_axis_rotation,
                           bool large_arc,
                           bool sweep,
                           float final_x,
                           float final_y) {
  const double pi = 3.1415926535897932384626433832795028841972;   // π = τ/2
  const double tau = 6.2831853071795864769252867665590057683943;  // τ = 2*π

  // "Conversion from endpoint to center parameterization" per
  // https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
  //
  // There seems to be a bug in the spec's "implementation notes". Actual
  // implementations, such as those below, do something slightly different
  // (marked with a †).
  //
  // https://gitlab.gnome.org/GNOME/librsvg/-/blob/8d13da3c5f5b2442b980d38d469923f926c27c11/src/path_builder.rs
  //
  // http://svn.apache.org/repos/asf/xmlgraphics/batik/branches/svg11/sources/org/apache/batik/ext/awt/geom/ExtendedGeneralPath.java
  //
  // https://github.com/blackears/svgSalamander/blob/a679f7cbd14703d95b61878f107bc52688a9e91d/svg-core/src/main/java/com/kitfox/svg/pathcmd/Arc.java
  //
  // https://github.com/millermedeiros/SVGParser/blob/e7f80a0810f6e2abe0db5e8e6a004c4cfd7f83ae/com/millermedeiros/geom/SVGArc.as

  // (†) The abs isn't part of the spec. Neither is checking that rx and ry are
  // non-zero (and non-NaN).
  double rx = fabs((double)radius_x);
  double ry = fabs((double)radius_y);
  if (!(rx > 0) || !(ry > 0)) {
    return (*c->vtable->path_line_to)(c,                             //
                                      (final_x * scale_x) + bias_x,  //
                                      (final_y * scale_y) + bias_y);
  }

  double x1 = (double)initial_x;
  double y1 = (double)initial_y;
  double x2 = (double)final_x;
  double y2 = (double)final_y;
  double phi = tau * ((double)x_axis_rotation);

  // Step 1: Compute (x1′, y1′)

  double half_dx = (x1 - x2) / 2;
  double half_dy = (y1 - y2) / 2;
  double cos_phi = cos(phi);
  double sin_phi = sin(phi);
  double x1_prime = +(cos_phi * half_dx) + (sin_phi * half_dy);
  double y1_prime = -(sin_phi * half_dx) + (cos_phi * half_dy);

  // Step 2: Compute (cx′, cy′)

  double rx_sq = rx * rx;
  double ry_sq = ry * ry;
  double x1_prime_sq = x1_prime * x1_prime;
  double y1_prime_sq = y1_prime * y1_prime;

  // (†) Check that the radii are large enough.
  double radii_check = (x1_prime_sq / rx_sq) + (y1_prime_sq / ry_sq);
  if (radii_check > 1) {
    double s = sqrt(radii_check);
    rx *= s;
    ry *= s;
    rx_sq = rx * rx;
    ry_sq = ry * ry;
  }

  double denom = (rx_sq * y1_prime_sq) + (ry_sq * x1_prime_sq);
  double step2 = 0.0;
  double a = ((rx_sq * ry_sq) / denom) - 1.0;
  if (a > 0.0) {
    step2 = sqrt(a);
  }
  if (large_arc == sweep) {
    step2 = -step2;
  }
  double cx_prime = +(step2 * rx * y1_prime) / ry;
  double cy_prime = -(step2 * ry * x1_prime) / rx;

  // Step 3: Compute (cx, cy) from (cx′, cy′)

  double cx = +(cos_phi * cx_prime) - (sin_phi * cy_prime) + ((x1 + x2) / 2);
  double cy = +(sin_phi * cx_prime) + (cos_phi * cy_prime) + ((y1 + y2) / 2);

  // Step 4: Compute θ1 and Δθ

  double ax = (+x1_prime - cx_prime) / rx;
  double ay = (+y1_prime - cy_prime) / ry;
  double bx = (-x1_prime - cx_prime) / rx;
  double by = (-y1_prime - cy_prime) / ry;
  double theta1 = iconvg_private_angle(1.0, 0.0, ax, ay);
  double delta_theta = iconvg_private_angle(ax, ay, bx, by);
  if (sweep) {
    if (delta_theta < 0.0) {
      delta_theta += tau;
    }
  } else {
    if (delta_theta > 0.0) {
      delta_theta -= tau;
    }
  }

  // This ends the
  // https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
  // algorithm. What follows below is specific to this implementation.

  // We approximate an arc by one or more cubic Bézier curves.
  int n = (int)(ceil(fabs(delta_theta) / ((pi / 2) + 0.001)));
  double inv_n = 1.0 / ((double)n);
  for (int i = 0; i < n; i++) {
    ICONVG_PRIVATE_TRY(iconvg_private_path_arc_segment_to(
        c, scale_x, bias_x, scale_y, bias_y, cx, cy,         //
        theta1 + (delta_theta * ((double)(i + 0)) * inv_n),  //
        theta1 + (delta_theta * ((double)(i + 1)) * inv_n),  //
        rx, ry, cos_phi, sin_phi));
  }
  return NULL;
}
