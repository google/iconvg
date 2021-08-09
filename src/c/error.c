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

const char iconvg_error_bad_coordinate[] =  //
    "iconvg: bad coordinate";
const char iconvg_error_bad_jump[] =  //
    "iconvg: bad jump";
const char iconvg_error_bad_magic_identifier[] =  //
    "iconvg: bad magic identifier";
const char iconvg_error_bad_metadata[] =  //
    "iconvg: bad metadata";
const char iconvg_error_bad_metadata_id_order[] =  //
    "iconvg: bad metadata ID order";
const char iconvg_error_bad_metadata_suggested_palette[] =  //
    "iconvg: bad metadata (suggested palette)";
const char iconvg_error_bad_metadata_viewbox[] =  //
    "iconvg: bad metadata (viewbox)";
const char iconvg_error_bad_number[] =  //
    "iconvg: bad number";
const char iconvg_error_bad_opcode_length[] =  //
    "iconvg: bad opcode length";

const char iconvg_error_system_failure_out_of_memory[] =  //
    "iconvg: system failure: out of memory";

const char iconvg_error_invalid_backend_not_enabled[] =  //
    "iconvg: invalid backend (not enabled)";
const char iconvg_error_invalid_constructor_argument[] =  //
    "iconvg: invalid constructor argument";
const char iconvg_error_invalid_paint_type[] =  //
    "iconvg: invalid paint type";
const char iconvg_error_invalid_vtable[] =  //
    "iconvg: invalid vtable";

// ----

bool  //
iconvg_error_is_file_format_error(const char* err_msg) {
  return (err_msg == iconvg_error_bad_coordinate) ||
         (err_msg == iconvg_error_bad_jump) ||
         (err_msg == iconvg_error_bad_magic_identifier) ||
         (err_msg == iconvg_error_bad_metadata) ||
         (err_msg == iconvg_error_bad_metadata_id_order) ||
         (err_msg == iconvg_error_bad_metadata_suggested_palette) ||
         (err_msg == iconvg_error_bad_metadata_viewbox) ||
         (err_msg == iconvg_error_bad_number) ||
         (err_msg == iconvg_error_bad_opcode_length);
}
