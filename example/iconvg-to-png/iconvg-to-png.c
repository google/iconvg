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
// usage: %s input.ivg > output.png
//     If input.ivg is omitted, it reads from stdin.

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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
                "usage: %s input.ivg > output.png\n"
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
  {
    iconvg_rectangle viewbox = {};
    const char* err_msg = iconvg_decode_viewbox(&viewbox, src_ptr, src_len);
    if (err_msg) {
      fprintf(stderr, "main: could not decode %s\n%s\n", input_filename,
              err_msg);
      return 1;
    }
    printf("\t%f\n", viewbox.min_x);
    printf("\t%f\n", viewbox.min_y);
    printf("\t%f\n", viewbox.max_x);
    printf("\t%f\n", viewbox.max_y);
    printf("\tw: %f\n", iconvg_rectangle__width(&viewbox));
    printf("\th: %f\n", iconvg_rectangle__height(&viewbox));
  }

  // Decode the IconVG.
  {
    iconvg_canvas canvas = iconvg_make_debug_canvas(stdout, "debug: ", NULL);
    const char* err_msg = iconvg_canvas__decode(&canvas, src_ptr, src_len);
    if (err_msg) {
      fprintf(stderr, "main: could not decode %s\n%s\n", input_filename,
              err_msg);
      return 1;
    }
  }

  return 0;
}
