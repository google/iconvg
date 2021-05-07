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

static void  //
iconvg_private_decoder__advance_to_ptr(iconvg_private_decoder* self,
                                       const uint8_t* new_ptr) {
  if (new_ptr >= self->ptr) {
    size_t delta = ((size_t)(new_ptr - self->ptr));
    if (delta <= self->len) {
      self->ptr += delta;
      self->len -= delta;
      return;
    }
  }
  self->ptr = NULL;
  self->len = 0;
}

static iconvg_private_decoder  //
iconvg_private_decoder__limit_u32(iconvg_private_decoder* self,
                                  uint32_t limit) {
  iconvg_private_decoder d;
  d.ptr = self->ptr;
  d.len = (limit < self->len) ? limit : self->len;
  return d;
}

static void  //
iconvg_private_decoder__skip_to_the_end(iconvg_private_decoder* self) {
  self->ptr += self->len;
  self->len = 0;
}

// ----

static bool  //
iconvg_private_decoder__decode_coordinate_number(iconvg_private_decoder* self,
                                                 float* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      int32_t i = (int32_t)(v >> 1);
      *dst = ((float)(i - 64));
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        int32_t i = (int32_t)(iconvg_private_peek_u16le(self->ptr) >> 2);
        *dst = ((float)(i - (128 * 64))) / 64.0f;
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        // TODO: reject NaNs?
        *dst = iconvg_private_reinterpret_from_u32_to_f32(
            0xFFFFFFFCu & iconvg_private_peek_u32le(self->ptr));
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

static bool  //
iconvg_private_decoder__decode_natural_number(iconvg_private_decoder* self,
                                              uint32_t* dst) {
  if (self->len >= 1) {
    uint8_t v = self->ptr[0];
    if ((v & 0x01) == 0) {  // 1-byte encoding.
      *dst = v >> 1;
      self->ptr += 1;
      self->len -= 1;
      return true;

    } else if ((v & 0x02) == 0) {  // 2-byte encoding.
      if (self->len >= 2) {
        *dst = iconvg_private_peek_u16le(self->ptr) >> 2;
        self->ptr += 2;
        self->len -= 2;
        return true;
      }

    } else {  // 4-byte encoding.
      if (self->len >= 4) {
        *dst = iconvg_private_peek_u32le(self->ptr) >> 2;
        self->ptr += 4;
        self->len -= 4;
        return true;
      }
    }
  }
  return false;
}

// ----

static bool  //
iconvg_private_decoder__decode_magic_identifier(iconvg_private_decoder* self) {
  if ((self->len < 4) &&         //
      (self->ptr[0] != 0x89) &&  //
      (self->ptr[1] != 0x49) &&  //
      (self->ptr[2] != 0x56) &&  //
      (self->ptr[3] != 0x47)) {
    return false;
  }
  self->ptr += 4;
  self->len -= 4;
  return true;
}

static bool  //
iconvg_private_decoder__decode_metadata_viewbox(iconvg_private_decoder* self,
                                                iconvg_rectangle* dst) {
  return iconvg_private_decoder__decode_coordinate_number(self, &dst->min_x) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->min_y) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->max_x) &&
         iconvg_private_decoder__decode_coordinate_number(self, &dst->max_y);
}

// ----

static const char*  //
iconvg_private_execute_bytecode(iconvg_canvas* c,
                                iconvg_private_decoder* d,
                                iconvg_private_bank* b) {
  // Drawing ops will typically set curr_x and curr_y. They also set x1 and y1
  // in case the subsequent op is smooth and needs an implicit point.
  float curr_x = +0.0f;
  float curr_y = +0.0f;
  float x1 = +0.0f;
  float y1 = +0.0f;
  float x2 = +0.0f;
  float y2 = +0.0f;
  float x3 = +0.0f;
  float y3 = +0.0f;

styling_mode:
  while (true) {
    if (d->len == 0) {
      return NULL;
    }
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    if (opcode < 0x80) {
      // TODO: implement.
      return iconvg_error_bad_styling_opcode;

    } else if (opcode < 0xC0) {
      // TODO: implement.
      return iconvg_error_bad_styling_opcode;

    } else if (opcode < 0xC7) {  // Switch to the drawing mode.
      if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
          !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
        return iconvg_error_bad_coordinate;
      }
      ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
      x1 = curr_x;
      y1 = curr_y;
      // TODO: if H is outside the LOD range then skip the drawing.
      goto drawing_mode;

    } else if (opcode < 0xC8) {  // Set Level of Detail bounds.
      // TODO: implement.
      return iconvg_error_bad_styling_opcode;
    }

    return iconvg_error_bad_styling_opcode;
  }

drawing_mode:
  while (true) {
    if (d->len == 0) {
      return iconvg_error_bad_path_unfinished;
    }
    uint8_t opcode = d->ptr[0];
    d->ptr += 1;
    d->len -= 1;

    switch (opcode >> 4) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x07:
        // TODO: implement.
        return iconvg_error_bad_drawing_opcode;

      case 0x08: {  // 'S' mnemonic: absolute smooth cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * x3) - x2;
          y1 = (2 * y3) - y2;
        }
        continue;
      }

      case 0x09: {  // 's' mnemonic: relative smooth cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          x2 += curr_x;
          y2 += curr_y;
          x3 += curr_x;
          y3 += curr_y;
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * x3) - x2;
          y1 = (2 * y3) - y2;
        }
        continue;
      }

      case 0x0A: {  // 'C' mnemonic: absolute cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * x3) - x2;
          y1 = (2 * y3) - y2;
        }
        continue;
      }

      case 0x0B: {  // 'c' mnemonic: relative cube_to.
        for (int reps = opcode & 0x0F; reps >= 0; reps--) {
          if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y1) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y2) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &x3) ||
              !iconvg_private_decoder__decode_coordinate_number(d, &y3)) {
            return iconvg_error_bad_coordinate;
          }
          x1 += curr_x;
          y1 += curr_y;
          x2 += curr_x;
          y2 += curr_y;
          x3 += curr_x;
          y3 += curr_y;
          ICONVG_PRIVATE_TRY(
              (*c->vtable->path_cube_to)(c, x1, y1, x2, y2, x3, y3));
          curr_x = x3;
          curr_y = y3;
          x1 = (2 * x3) - x2;
          y1 = (2 * y3) - y2;
        }
        continue;
      }

      case 0x0C:
      case 0x0D:
        // TODO: implement.
        return iconvg_error_bad_drawing_opcode;
    }

    switch (opcode) {
      case 0xE1: {  // 'z' mnemonic: close_path.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        // TODO: call c->vtable->paint.
        goto styling_mode;
      }

      case 0xE2: {  // 'z; M' mnemonics: close_path; absolute move_to.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x) ||
            !iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE3: {  // 'z; m' mnemonics: close_path; relative move_to.
        ICONVG_PRIVATE_TRY((*c->vtable->end_path)(c));
        if (!iconvg_private_decoder__decode_coordinate_number(d, &x1) ||
            !iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_x += x1;
        curr_y += y1;
        ICONVG_PRIVATE_TRY((*c->vtable->begin_path)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE6: {  // 'H' mnemonic: absolute horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_x)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE7: {  // 'h' mnemonic: relative horizontal line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &x1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_x += x1;
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE8: {  // 'V' mnemonic: absolute vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &curr_y)) {
          return iconvg_error_bad_coordinate;
        }
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }

      case 0xE9: {  // 'v' mnemonic: relative vertical line_to.
        if (!iconvg_private_decoder__decode_coordinate_number(d, &y1)) {
          return iconvg_error_bad_coordinate;
        }
        curr_y += y1;
        ICONVG_PRIVATE_TRY((*c->vtable->path_line_to)(c, curr_x, curr_y));
        x1 = curr_x;
        y1 = curr_y;
        continue;
      }
    }

    return iconvg_error_bad_drawing_opcode;
  }
  return iconvg_private_internal_error_unreachable;
}

// ----

const char*  //
iconvg_decode_viewbox(iconvg_rectangle* dst_viewbox,
                      const uint8_t* src_ptr,
                      size_t src_len) {
  iconvg_private_decoder d;
  d.ptr = src_ptr;
  d.len = src_len;

  if (!iconvg_private_decoder__decode_magic_identifier(&d)) {
    return iconvg_error_bad_magic_identifier;
  }
  uint32_t num_metadata_chunks;
  if (!iconvg_private_decoder__decode_natural_number(&d,
                                                     &num_metadata_chunks)) {
    return iconvg_error_bad_metadata;
  }

  bool use_default_viewbox = true;
  int32_t previous_metadata_id = -1;
  for (; num_metadata_chunks > 0; num_metadata_chunks--) {
    uint32_t chunk_length;
    if (!iconvg_private_decoder__decode_natural_number(&d, &chunk_length) ||
        (chunk_length > d.len)) {
      return iconvg_error_bad_metadata;
    }
    iconvg_private_decoder chunk =
        iconvg_private_decoder__limit_u32(&d, chunk_length);
    uint32_t metadata_id;
    if (!iconvg_private_decoder__decode_natural_number(&chunk, &metadata_id)) {
      return iconvg_error_bad_metadata;
    } else if (previous_metadata_id >= ((int32_t)metadata_id)) {
      return iconvg_error_bad_metadata_id_order;
    }

    if (metadata_id == 0) {  // MID 0 (ViewBox).
      use_default_viewbox = false;
      iconvg_rectangle r;
      if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk, &r) ||
          (chunk.len != 0)) {
        return iconvg_error_bad_metadata_viewbox;
      } else if (dst_viewbox) {
        *dst_viewbox = r;
      }
    }

    iconvg_private_decoder__skip_to_the_end(&chunk);
    iconvg_private_decoder__advance_to_ptr(&d, chunk.ptr);
    previous_metadata_id = ((int32_t)metadata_id);
  }

  if (use_default_viewbox && dst_viewbox) {
    *dst_viewbox = iconvg_private_default_viewbox();
  }
  return NULL;
}

static const char*  //
iconvg_private_decode(iconvg_canvas* c,
                      const uint8_t* src_ptr,
                      size_t src_len) {
  const char* err_msg = NULL;

  iconvg_private_decoder d;
  d.ptr = src_ptr;
  d.len = src_len;

  // TODO: custom/suggested palette.
  iconvg_private_bank b;
  memset(b.creg, 0x00, sizeof(b.creg));
  memset(b.nreg, 0x00, sizeof(b.nreg));

  if (!iconvg_private_decoder__decode_magic_identifier(&d)) {
    return iconvg_error_bad_magic_identifier;
  }
  uint32_t num_metadata_chunks;
  if (!iconvg_private_decoder__decode_natural_number(&d,
                                                     &num_metadata_chunks)) {
    return iconvg_error_bad_metadata;
  }

  bool use_default_viewbox = true;
  int32_t previous_metadata_id = -1;
  for (; num_metadata_chunks > 0; num_metadata_chunks--) {
    uint32_t chunk_length;
    if (!iconvg_private_decoder__decode_natural_number(&d, &chunk_length) ||
        (chunk_length > d.len)) {
      return iconvg_error_bad_metadata;
    }
    iconvg_private_decoder chunk =
        iconvg_private_decoder__limit_u32(&d, chunk_length);
    uint32_t metadata_id;
    if (!iconvg_private_decoder__decode_natural_number(&chunk, &metadata_id)) {
      return iconvg_error_bad_metadata;
    } else if (previous_metadata_id >= ((int32_t)metadata_id)) {
      return iconvg_error_bad_metadata_id_order;
    }

    if (metadata_id == 0) {  // MID 0 (ViewBox).
      use_default_viewbox = false;
      iconvg_rectangle r;
      if (!iconvg_private_decoder__decode_metadata_viewbox(&chunk, &r) ||
          (chunk.len != 0)) {
        return iconvg_error_bad_metadata_viewbox;
      }
      err_msg = (*c->vtable->on_metadata_viewbox)(c, r);
      if (err_msg) {
        return err_msg;
      }
    } else if ((metadata_id > 0) && use_default_viewbox) {
      use_default_viewbox = false;
      err_msg = (*c->vtable->on_metadata_viewbox)(
          c, iconvg_private_default_viewbox());
      if (err_msg) {
        return err_msg;
      }
    }

    iconvg_private_decoder__skip_to_the_end(&chunk);
    iconvg_private_decoder__advance_to_ptr(&d, chunk.ptr);
    previous_metadata_id = ((int32_t)metadata_id);
  }

  if (use_default_viewbox) {
    use_default_viewbox = false;
    err_msg =
        (*c->vtable->on_metadata_viewbox)(c, iconvg_private_default_viewbox());
    if (err_msg) {
      return err_msg;
    }
  }

  return iconvg_private_execute_bytecode(c, &d, &b);
}

const char*  //
iconvg_decode(iconvg_canvas* dst_canvas,
              const uint8_t* src_ptr,
              size_t src_len) {
  iconvg_canvas fallback_canvas = iconvg_make_debug_canvas(NULL, NULL, NULL);
  if (!dst_canvas) {
    dst_canvas = &fallback_canvas;
  }

  if (!dst_canvas->vtable) {
    return iconvg_error_null_vtable;
  } else if (dst_canvas->vtable->sizeof__iconvg_canvas_vtable !=
             sizeof(iconvg_canvas_vtable)) {
    // If we want to support multiple library versions (with dynamic linking),
    // we could detect older versions here (with smaller vtable sizes) and
    // substitute in an adapter implementation.
    return iconvg_error_unsupported_vtable;
  }
  const char* err_msg = (*dst_canvas->vtable->begin_decode)(dst_canvas);
  if (!err_msg) {
    err_msg = iconvg_private_decode(dst_canvas, src_ptr, src_len);
  }
  return (*dst_canvas->vtable->end_decode)(dst_canvas, err_msg);
}
