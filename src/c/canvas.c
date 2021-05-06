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

static const char*  //
iconvg_private_canvas_decode(iconvg_canvas* c,
                             const uint8_t* src_ptr,
                             size_t src_len) {
  return NULL;
}

const char*  //
iconvg_canvas__decode(iconvg_canvas* self,
                      const uint8_t* src_ptr,
                      size_t src_len) {
  if (!self) {
    return iconvg_error_null_argument;
  } else if (!self->vtable) {
    return iconvg_error_null_vtable;
  } else if (self->vtable->sizeof__iconvg_canvas_vtable !=
             sizeof(iconvg_canvas_vtable)) {
    // If we want to support multiple library versions (with dynamic linking),
    // we could detect older versions here (with smaller vtable sizes) and
    // substitute in an adapter implementation.
    return iconvg_error_unsupported_vtable;
  }
  const char* err_msg = (*self->vtable->begin_decode)(self);
  if (!err_msg) {
    err_msg = iconvg_private_canvas_decode(self, src_ptr, src_len);
  }
  return (*self->vtable->end_decode)(self, err_msg);
}
