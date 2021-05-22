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

// iconvg-to-png converts from IconVG to PNG (written to stdout).
//
// Usage: iconvg-to-png input.ivg > output.png
//     If input.ivg is omitted, it reads from stdin.

#include <errno.h>
#include <png.h>
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
  }
  cairo_surface_t* cs =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
  cairo_t* cr = cairo_create(cs);

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
  return NULL;
}

const char*  //
flush_pixel_buffer(pixel_buffer* pb, uint32_t width, uint32_t height) {
  // flush_pixel_buffer will always fail. Still, this program might be useful
  // if it prints debug output prior to failure.
  return "main: no IconVG backend configured";
}

const char*  //
finalize_pixel_buffer(pixel_buffer* pb) {
  return NULL;
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
      fprintf(stderr, "main: %s file size (in bytes) is too large\n",
              src_filename);
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
    fprintf(stderr, "main: could not read %s: %s\n", src_filename,
            strerror(err));
    return false;
  }
  return true;
}

// ----

const char*  //
write_png_to_stdout(pixel_buffer* pb) {
  if (!pb || (pb->width > 0x7FFF) || (pb->height > 0x7FFF)) {
    return "main: invalid write_png_to_stdout argument";
  }

  const char* ret = NULL;
  png_structp png = NULL;
  png_infop info = NULL;
  png_byte** rows = NULL;

  {
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
      ret = "main: png_create_write_struct failed";
      goto exit;
    } else if (setjmp(png_jmpbuf(png))) {
      ret = "main: libpng failed";
      goto exit;
    }

    info = png_create_info_struct(png);
    if (!info) {
      ret = "main: png_create_info_struct failed";
      goto exit;
    }

    rows = malloc(pb->height * sizeof(png_byte*));
    for (uint32_t i = 0; i < pb->height; i++) {
      const size_t bytes_per_pixel = 4;
      rows[i] = pb->data + (i * bytes_per_pixel * pb->width);
    }
    png_init_io(png, stdout);
    png_set_IHDR(png, info, pb->width, pb->height, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png, info, rows);
    png_write_png(png, info, PNG_TRANSFORM_BGR, NULL);
  }

exit:
  if (rows) {
    free(rows);
  }
  if (png) {
    png_destroy_write_struct(&png, &info);
  }
  return ret;
}

// ----

int  //
main(int argc, char** argv) {
  // Read the input bytes.
  const char* input_filename = NULL;
  uint8_t* src_ptr = &g_src_buffer_array[0];
  size_t src_len = 0;
  {
    FILE* in = NULL;
    switch (argc) {
      case 1:
        input_filename = "<stdin>";
        in = stdin;
        break;
      case 2:
        input_filename = argv[1];
        in = fopen(input_filename, "r");
        if (!in) {
          fprintf(stderr, "main: could not open %s: %s\n", input_filename,
                  strerror(errno));
          return 1;
        }
        // No need to explicitly close in later. The program exits (and releases
        // all file descriptors) when main returns.
        break;
      default:
        fprintf(stderr,
                "Usage: %s input.ivg > output.png\n"
                "    If input.ivg is omitted, it reads from stdin.\n",
                argv[0]);
        return 1;
    }
    if (!read_file(&src_len, &g_src_buffer_array[0], SRC_BUFFER_ARRAY_SIZE, in,
                   input_filename)) {
      return 1;
    }
  }

  // Decode the IconVG viewbox.
  iconvg_rectangle_f32 viewbox = {0};
  uint32_t pixel_width = 256;   // TODO
  uint32_t pixel_height = 256;  // TODO
  {
    const char* err_msg = iconvg_decode_viewbox(&viewbox, src_ptr, src_len);
    if (err_msg) {
      fprintf(stderr, "main: could not decode %s\n%s\n", input_filename,
              err_msg);
      return 1;
    }
  }

  // Check that the graphic isn't too large. An 0x7FFF = 32767 pixel width or
  // height upper bound is somewhat arbitrary, but it simplifies any uint32_t
  // overflow concerns about (pixel_width * pixel_height * bytes_per_pixel).
  if ((pixel_width > 0x7FFF) || (pixel_height > 0x7FFF)) {
    fprintf(stderr, "main: graphic is too large");
    return 1;
  } else if ((pixel_width == 0) || (pixel_height == 0)) {
    // IconVG can represent empty images (containing no pixels when rasterized,
    // analogous to empty strings containing no characters), but PNG cannot.
    fprintf(stderr, "main: cannot write an empty-sized PNG image");
    return 1;
  }

  // Initialize the pixel buffer.
  pixel_buffer pb = {0};
  {
    const char* err_msg =
        initialize_pixel_buffer(&pb, pixel_width, pixel_height);
    if (err_msg) {
      fprintf(stderr, "main: could not initialize the pixel buffer\n%s\n",
              err_msg);
      return 1;
    }
  }

  // Decode the IconVG.
  {
    iconvg_canvas* c = &pb.canvas;
    iconvg_canvas debug_canvas = iconvg_make_debug_canvas(
        stderr, "debug: ", pb.canvas.vtable ? &pb.canvas : NULL);
    if (true) {  // TODO: parse a -debug command line arg.
      c = &debug_canvas;
    }
    const char* err_msg = iconvg_decode(
        c, iconvg_make_rectangle_f32(0, 0, pixel_width, pixel_height), src_ptr,
        src_len, NULL);
    if (err_msg) {
      fprintf(stderr, "main: could not decode %s\n%s\n", input_filename,
              err_msg);
      return 1;
    }
  }

  // Flush the backend-specific drawing ops to the pixel buffer.
  {
    const char* err_msg = flush_pixel_buffer(&pb, pixel_width, pixel_height);
    if (err_msg) {
      fprintf(stderr, "main: could not flush the pixel buffer\n%s\n", err_msg);
      return 1;
    }
  }

  // Convert from premultiplied alpha to non-premultiplied alpha.
  // CAIRO_FORMAT_ARGB32 uses the former. libpng uses the latter.
  {
    for (uint32_t y = 0; y < pb.height; y++) {
      const size_t bytes_per_pixel = 4;
      uint8_t* row = pb.data + (y * bytes_per_pixel * pb.width);
      for (uint32_t x = 0; x < pb.width; x++) {
        uint8_t* rgba = row + (x * bytes_per_pixel);
        if ((rgba[3] != 0x00) && (rgba[3] != 0xFF)) {
          uint32_t a = rgba[3];
          rgba[0] = (uint8_t)((rgba[0] * ((uint32_t)0xFF)) / a);
          rgba[1] = (uint8_t)((rgba[1] * ((uint32_t)0xFF)) / a);
          rgba[2] = (uint8_t)((rgba[2] * ((uint32_t)0xFF)) / a);
        }
      }
    }
  }

  // Write the PNG to stdout.
  {
    const char* err_msg = write_png_to_stdout(&pb);
    if (err_msg) {
      fprintf(stderr, "main: could not write the PNG to stdout\n%s\n", err_msg);
      return 1;
    }
  }

  // Finalize the pixel buffer.
  {
    const char* err_msg = finalize_pixel_buffer(&pb);
    if (err_msg) {
      fprintf(stderr, "main: could not finalize the pixel buffer\n%s\n",
              err_msg);
      return 1;
    }
  }

  return 0;
}
