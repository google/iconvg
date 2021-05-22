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

// ----------------

// iconvg-viewer is a simple GUI program for viewing IconVG images. On Linux,
// GUI means X11.
//
// See the top-level build-example-etc.sh scripts for build parameters.
//
// Usage: iconvg-viewer *.ivg
//
// The Space and BackSpace keys cycle through the IconVG files, if more than
// one was given as command line arguments. If none were given, the program
// fails.
//
// The Return key is equivalent to the Space key.
//
// The , and . keys cycle through background checkerboard colors.
//
// The Escape key quits.

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IconVG ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// To use that single file as a "foo.c"-like implementation, instead of a
// "foo.h"-like header, #define ICONVG_IMPLEMENTATION before #include'ing or
// compiling it.
#define ICONVG_IMPLEMENTATION
#include "../../release/c/iconvg-unsupported-snapshot.c"

// SRC_BUFFER_ARRAY_SIZE is the largest size (in bytes) for .ivg files
// supported by this program.
//
// This is 1 MiB (1024 * 1024 = 1048576 bytes) by default, but can be
// configured by compiling with -DSRC_BUFFER_ARRAY_SIZE=etc.
#ifndef SRC_BUFFER_ARRAY_SIZE
#define SRC_BUFFER_ARRAY_SIZE 1048576
#endif
uint8_t g_src_buffer_array[SRC_BUFFER_ARRAY_SIZE];
size_t g_src_len = 0;

// g_background_colors' 6 elements are two checkerboard colors: [R0, G0, B0,
// R1, G1, B1].
#define NUM_BACKGROUND_COLORS 3
const double g_background_colors[NUM_BACKGROUND_COLORS][6] = {
    {0.20, 0.20, 0.20, 0.25, 0.25, 0.25},
    {0.75, 0.75, 0.75, 0.80, 0.80, 0.80},
    {0.80, 0.25, 0.70, 0.70, 0.25, 0.80},
};
uint32_t g_background_color_index = 0;

typedef struct {
  uint8_t* data;
  uint32_t width;
  uint32_t height;
  iconvg_canvas canvas;
  void* extra0;
  void* extra1;
} pixel_buffer;

// ----

#if defined(ICONVG_CONFIG__ENABLE_CAIRO_BACKEND)

#include <cairo/cairo.h>

const char*  //
initialize_pixel_buffer(pixel_buffer* pb, uint32_t width, uint32_t height) {
  if (!pb) {
    return "main: NULL pixel_buffer";
  } else if ((((int)width) < 0) || (((int)height) < 0)) {
    return "main: dimensions are too large";
  }
  cairo_surface_t* cs =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
  cairo_status_t cs_status = cairo_surface_status(cs);
  if (cs_status != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(cs);
    return cairo_status_to_string(cs_status);
  }
  cairo_t* cr = cairo_create(cs);
  cairo_status_t cr_status = cairo_status(cr);
  if (cr_status != CAIRO_STATUS_SUCCESS) {
    cairo_destroy(cr);
    cairo_surface_destroy(cs);
    return cairo_status_to_string(cr_status);
  }

  // Draw the checkerboard background.
  for (uint32_t y = 0; y < height; y += 64) {
    for (uint32_t x = 0; x < width; x += 64) {
      uint32_t xor = ((x ^ y) >> 6) & 1;
      uint32_t base = 3 * xor;
      cairo_set_source_rgb(
          cr,  //
          g_background_colors[g_background_color_index][base + 0],
          g_background_colors[g_background_color_index][base + 1],
          g_background_colors[g_background_color_index][base + 2]);
      cairo_rectangle(cr, x, y, 64, 64);
      cairo_fill(cr);
    }
  }

  *pb = ((pixel_buffer){0});
  pb->canvas = iconvg_make_cairo_canvas(cr);
  pb->extra0 = cs;
  pb->extra1 = cr;
  return NULL;
}

const char*  //
flush_pixel_buffer(pixel_buffer* pb, uint32_t width, uint32_t height) {
  if (!pb) {
    return "main: NULL pixel_buffer";
  }
  cairo_surface_t* cs = (cairo_surface_t*)(pb->extra0);
  if (!cs) {
    return "main: NULL cairo_surface_t";
  }
  cairo_surface_flush(cs);
  pb->data = cairo_image_surface_get_data(cs);
  pb->width = width;
  pb->height = height;
  return NULL;
}

const char*  //
finalize_pixel_buffer(pixel_buffer* pb) {
  if (!pb) {
    return "main: NULL pixel_buffer";
  }
  if (pb->extra1) {
    cairo_destroy((cairo_t*)(pb->extra1));
  }
  if (pb->extra0) {
    cairo_surface_destroy((cairo_surface_t*)(pb->extra0));
  }
  return NULL;
}

#else  //  ICONVG_CONFIG__ETC

const char*  //
initialize_pixel_buffer(pixel_buffer* pb, uint32_t width, uint32_t height) {
  if (!pb) {
    return "main: NULL pixel_buffer";
  }
  *pb = ((pixel_buffer){0});
  pb->canvas = iconvg_make_broken_canvas("main: no IconVG backend configured");
  return NULL;
}

const char*  //
flush_pixel_buffer(pixel_buffer* pb, uint32_t width, uint32_t height) {
  return "main: no IconVG backend configured";
}

const char*  //
finalize_pixel_buffer(pixel_buffer* pb) {
  return "main: no IconVG backend configured";
}

#endif  //  ICONVG_CONFIG__ETC

// ----

bool  //
read_file(size_t* dst_num_bytes_read,
          uint8_t* dst_buffer_ptr,
          size_t dst_buffer_len,
          FILE* src_file,
          const char* src_filename) {
  if (!dst_num_bytes_read || !src_file || !src_filename) {
    return false;
  }
  *dst_num_bytes_read = 0;
  uint8_t placeholder[1];
  uint8_t* ptr = dst_buffer_ptr;
  size_t len = dst_buffer_len;
  while (true) {
    if (!len) {
      // We have read all that dst can hold. Check that we have read the full
      // file by trying to read one more byte, which should fail with EOF.
      ptr = placeholder;
      len = 1;
    }
    size_t n = fread(ptr, 1, len, src_file);
    if (ptr != placeholder) {
      ptr += n;
      len -= n;
      *dst_num_bytes_read += n;
    } else if (n) {
      printf("%s: main: file size (in bytes) is too large\n", src_filename);
      return false;
    }
    if (feof(src_file)) {
      break;
    }
    int err = ferror(src_file);
    if (!err) {
      continue;
    } else if (err == EINTR) {
      clearerr(src_file);
      continue;
    }
    printf("%s: main: I/O error: %s\n", src_filename, strerror(err));
    return false;
  }
  return true;
}

bool  //
load(const char* filename) {
  if (!filename) {
    return false;
  }
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    printf("%s: main: could not open file\n", filename);
    return false;
  }
  g_src_len = 0;
  bool ret = read_file(&g_src_len, &g_src_buffer_array[0],
                       SRC_BUFFER_ARRAY_SIZE, f, filename);
  fclose(f);
  return ret;
}

// ----

bool  //
render(void* ctx,
       uint32_t window_width,
       uint32_t window_height,
       const char* filename,
       const char* (*upload_pixel_buffer)(void* ctx, pixel_buffer* pb)) {
  if (!filename) {
    return false;
  }
  uint8_t const* src_ptr = &g_src_buffer_array[0];
  const size_t src_len = g_src_len;

  // Decode the IconVG viewbox.
  double vw = 0.0;
  double vh = 0.0;
  iconvg_rectangle_f32 dst_rect = {0};
  {
    iconvg_rectangle_f32 viewbox = {0};
    const char* err_msg = iconvg_decode_viewbox(&viewbox, src_ptr, src_len);
    if (err_msg) {
      printf("%s: iconvg_decode_viewbox: %s\n", filename, err_msg);
      return false;
    }
    int32_t dr_width = 0;
    int32_t dr_height = 0;
    vw = iconvg_rectangle_f32__width_f64(&viewbox);
    vh = iconvg_rectangle_f32__height_f64(&viewbox);
    if ((vw <= 0) || (vh <= 0)) {
      dr_width = 1;
      dr_height = 1;
    } else if ((vw * window_height) < (vh * window_width)) {
      dr_width = (int32_t)(window_height * vw / vh);
      dr_height = window_height;
    } else {
      dr_width = window_width;
      dr_height = (int32_t)(window_width * vh / vw);
    }

    // An 0x3FFF = 16383 pixel width or height upper bound is somewhat
    // arbitrary, but it simplifies any int32_t overflow concerns about
    // (pixel_width * pixel_height * bytes_per_pixel).
    if (dr_width > 0x3FFF) {
      dr_width = 0x3FFF;
    }
    if (dr_height > 0x3FFF) {
      dr_height = 0x3FFF;
    }

    int32_t min_x = (window_width - dr_width) / 2;
    int32_t min_y = (window_height - dr_height) / 2;
    dst_rect = iconvg_make_rectangle_f32(min_x,               //
                                         min_y,               //
                                         min_x + dr_width,    //
                                         min_y + dr_height);  //
  }

  // Initialize the pixel buffer.
  pixel_buffer pb = {0};
  {
    const char* err_msg =
        initialize_pixel_buffer(&pb, window_width, window_height);
    if (err_msg) {
      printf("%s: initialize_pixel_buffer: %s\n", filename, err_msg);
      return false;
    }
  }

  // Decode the IconVG.
  if ((vw > 0.0) && (vh > 0.0)) {
    const char* err_msg =
        iconvg_decode(&pb.canvas, dst_rect, src_ptr, src_len, NULL);
    if (err_msg) {
      printf("%s: iconvg_decode: %s\n", filename, err_msg);
      goto clean_up_and_fail;
    }
  }

  // Flush the backend-specific drawing ops to the pixel buffer.
  {
    const char* err_msg = flush_pixel_buffer(&pb, window_width, window_height);
    if (err_msg) {
      printf("%s: flush_pixel_buffer: %s\n", filename, err_msg);
      goto clean_up_and_fail;
    }
  }

  // Upload the pixel buffer to the screen, in an OS-specific way.
  {
    const char* err_msg = (*upload_pixel_buffer)(ctx, &pb);
    if (err_msg) {
      printf("%s: upload_pixel_buffer: %s\n", filename, err_msg);
      goto clean_up_and_fail;
    }
  }

  // Finalize the pixel buffer.
  {
    const char* err_msg = finalize_pixel_buffer(&pb);
    if (err_msg) {
      printf("%s: finalize_pixel_buffer: %s\n", filename, err_msg);
      return false;
    }
  }

  printf("%s: ok (%g x %g)\n", filename, vw, vh);
  return true;
clean_up_and_fail:
  finalize_pixel_buffer(&pb);
  return false;
}

// --------

#if defined(__linux__)
#define SUPPORTED_OPERATING_SYSTEM

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#define XK_BackSpace 0xFF08
#define XK_Escape 0xFF1B
#define XK_Return 0xFF0D

xcb_atom_t g_atom_net_wm_name = XCB_NONE;
xcb_atom_t g_atom_utf8_string = XCB_NONE;
xcb_atom_t g_atom_wm_protocols = XCB_NONE;
xcb_atom_t g_atom_wm_delete_window = XCB_NONE;
xcb_pixmap_t g_pixmap = XCB_NONE;
uint32_t g_pixmap_width = 0;
uint32_t g_pixmap_height = 0;
xcb_keysym_t* g_keysyms = NULL;
xcb_get_keyboard_mapping_reply_t* g_keyboard_mapping = NULL;

void  //
init_keymap(xcb_connection_t* c, const xcb_setup_t* z) {
  xcb_get_keyboard_mapping_cookie_t cookie = xcb_get_keyboard_mapping(
      c, z->min_keycode, z->max_keycode - z->min_keycode + 1);
  g_keyboard_mapping = xcb_get_keyboard_mapping_reply(c, cookie, NULL);
  g_keysyms = (xcb_keysym_t*)(g_keyboard_mapping + 1);
}

xcb_window_t  //
make_window(xcb_connection_t* c, xcb_screen_t* s) {
  xcb_window_t w = xcb_generate_id(c);
  uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t value_list[2];
  value_list[0] = s->black_pixel;
  value_list[1] = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_EXPOSURE |
                  XCB_EVENT_MASK_KEY_PRESS;
  xcb_create_window(c, 0, w, s->root, 0, 0, 1024, 768, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual, value_mask,
                    value_list);
  xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, g_atom_net_wm_name,
                      g_atom_utf8_string, 8, 13, "IconVG Viewer");
  xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, g_atom_wm_protocols,
                      XCB_ATOM_ATOM, 32, 1, &g_atom_wm_delete_window);
  xcb_map_window(c, w);
  return w;
}

typedef struct {
  xcb_connection_t* c;
  xcb_screen_t* s;
  xcb_window_t w;
  xcb_gcontext_t g;
} my_context;

const char*  //
upload_pixel_buffer(void* ctx_as_void_star, pixel_buffer* pb) {
  my_context* ctx = (my_context*)ctx_as_void_star;
  if ((g_pixmap_width > 0) && (g_pixmap_height > 0)) {
    xcb_free_pixmap(ctx->c, g_pixmap);
    g_pixmap_width = 0;
    g_pixmap_height = 0;
  }
  if ((pb->width == 0) || (pb->height == 0)) {
    return NULL;
  } else if ((pb->width > 0x3FFF) || (pb->height > 0x3FFF)) {
    return "main: pixel buffer is too large";
  }

  // Calculate max_h, the largest number of rows we can issue in a single
  // xcb_image_put call without exceeding the XCB request length limit. This
  // number depends on pb->width, the width of the pixel buffer.
  //
  // The xcb_get_maximum_request_length documentation says that "this length
  // is measured in four-byte units". Coincidentally, our RGBA pixels are
  // also four bytes per pixel, so max_h is basically the mrl divided by the
  // width, also adjusting for the xcb_put_image_request_t header length
  // (measured in four-byte units).
  uint32_t mrl = xcb_get_maximum_request_length(ctx->c);
  uint32_t header_length = (sizeof(xcb_put_image_request_t) + 3) / 4;
  if (mrl < header_length) {
    return "main: XCB request length is too short";
  }
  uint32_t max_h = (mrl - header_length) / pb->width;
  if (max_h == 0) {
    return "main: XCB request length is too short";
  }

  g_pixmap_width = pb->width;
  g_pixmap_height = pb->height;
  xcb_create_pixmap(ctx->c, ctx->s->root_depth, g_pixmap, ctx->w,
                    g_pixmap_width, g_pixmap_height);
  xcb_image_t* image = xcb_image_create_native(
      ctx->c, pb->width, pb->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
      ctx->s->root_depth, NULL, pb->width * pb->height * 4, pb->data);
  if (pb->height <= max_h) {
    xcb_image_put(ctx->c, g_pixmap, ctx->g, image, 0, 0, 0);
  } else {
    int y = 0;
    while (y < pb->height) {
      uint32_t h = pb->height - y;
      if (h > max_h) {
        h = max_h;
      }
      xcb_image_t* sub = xcb_image_subimage(image, 0, y, pb->width, h, 0, 0, 0);
      xcb_image_put(ctx->c, g_pixmap, ctx->g, sub, 0, y, 0);
      xcb_image_destroy(sub);
      y += h;
    }
  }
  xcb_image_destroy(image);
  return NULL;
}

int  //
main(int argc, char** argv) {
  if (argc <= 1) {
    printf("main: no input files given\n");
    return 1;
  }

  xcb_connection_t* c = xcb_connect(NULL, NULL);
  const xcb_setup_t* z = xcb_get_setup(c);
  xcb_screen_t* s = xcb_setup_roots_iterator(z).data;

  {
    xcb_intern_atom_cookie_t cookie0 =
        xcb_intern_atom(c, 1, 12, "_NET_WM_NAME");
    xcb_intern_atom_cookie_t cookie1 = xcb_intern_atom(c, 1, 11, "UTF8_STRING");
    xcb_intern_atom_cookie_t cookie2 =
        xcb_intern_atom(c, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_cookie_t cookie3 =
        xcb_intern_atom(c, 1, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* reply0 = xcb_intern_atom_reply(c, cookie0, NULL);
    xcb_intern_atom_reply_t* reply1 = xcb_intern_atom_reply(c, cookie1, NULL);
    xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(c, cookie2, NULL);
    xcb_intern_atom_reply_t* reply3 = xcb_intern_atom_reply(c, cookie3, NULL);
    g_atom_net_wm_name = reply0->atom;
    g_atom_utf8_string = reply1->atom;
    g_atom_wm_protocols = reply2->atom;
    g_atom_wm_delete_window = reply3->atom;
    free(reply0);
    free(reply1);
    free(reply2);
    free(reply3);
  }

  xcb_window_t w = make_window(c, s);
  xcb_gcontext_t g = xcb_generate_id(c);
  xcb_create_gc(c, g, w, 0, NULL);
  init_keymap(c, z);
  xcb_flush(c);
  g_pixmap = xcb_generate_id(c);

  uint32_t window_width = 0;
  uint32_t window_height = 0;

  int arg = 1;
  bool loaded = load(argv[arg]);
  bool rendered = false;

  while (true) {
    bool reload = false;
    bool rerender = false;

    xcb_generic_event_t* event = xcb_wait_for_event(c);
    if (!event) {
      if (xcb_connection_has_error(c) == XCB_CONN_CLOSED_REQ_LEN_EXCEED) {
        printf("main: XCB connection error (request length exceeded)\n");
        return 1;
      }
      printf("main: XCB connection error\n");
      return 1;
    }
    switch (event->response_type & 0x7F) {
      case XCB_EXPOSE: {
        xcb_expose_event_t* e = (xcb_expose_event_t*)event;
        if (rendered && (e->count == 0) && (g_pixmap_width > 0) &&
            (g_pixmap_height > 0)) {
          xcb_copy_area(c, g_pixmap, w, g, 0, 0, 0, 0, g_pixmap_width,
                        g_pixmap_height);
          xcb_flush(c);
        }
        break;
      }

      case XCB_KEY_PRESS: {
        xcb_key_press_event_t* e = (xcb_key_press_event_t*)event;
        uint32_t i = e->detail;
        if ((z->min_keycode <= i) && (i <= z->max_keycode)) {
          i = g_keysyms[(i - z->min_keycode) *
                        g_keyboard_mapping->keysyms_per_keycode];
          switch (i) {
            case XK_Escape:
              return 0;

            case ' ':
            case XK_BackSpace:
            case XK_Return:
              if (argc <= 2) {
                break;
              }
              arg += (i != XK_BackSpace) ? +1 : -1;
              if (arg == 0) {
                arg = argc - 1;
              } else if (arg == argc) {
                arg = 1;
              }
              reload = true;
              rerender = true;
              break;

            case ',':
            case '.':
              g_background_color_index +=
                  (i == ',') ? (NUM_BACKGROUND_COLORS - 1) : 1;
              g_background_color_index %= NUM_BACKGROUND_COLORS;
              rerender = true;
              break;
          }
        }
        break;
      }

      case XCB_CONFIGURE_NOTIFY: {
        xcb_configure_notify_event_t* e = (xcb_configure_notify_event_t*)event;
        if ((window_width != e->width) || (window_height != e->height)) {
          window_width = e->width;
          window_height = e->height;
          rerender = true;
        }
        break;
      }

      case XCB_CLIENT_MESSAGE: {
        xcb_client_message_event_t* e = (xcb_client_message_event_t*)event;
        if (e->data.data32[0] == g_atom_wm_delete_window) {
          return 0;
        }
        break;
      }
    }
    free(event);

    const char* filename = argv[arg];
    if (reload) {
      loaded = load(filename);
    }
    if (loaded && rerender && (window_width > 0) && (window_height > 0)) {
      my_context ctx;
      ctx.c = c;
      ctx.s = s;
      ctx.w = w;
      ctx.g = g;
      rendered = render(&ctx, window_width, window_height, filename,
                        &upload_pixel_buffer);
      xcb_clear_area(c, 1, w, 0, 0, 0xFFFF, 0xFFFF);
      xcb_flush(c);
    }
  }
  return 0;
}

#endif  // defined(__linux__)

// --------

#if !defined(SUPPORTED_OPERATING_SYSTEM)

int  //
main(int argc, char** argv) {
  printf("main: unsupported operating system\n");
  return 1;
}

#endif  // !defined(SUPPORTED_OPERATING_SYSTEM)
