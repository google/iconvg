#!/bin/bash -eu
# Copyright 2021 The IconVG Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# ----------------

# On a Debian or Ubuntu system, you might first need to run:
#   sudo apt install libpng-dev libxcb1-dev libxcb-image0-dev
#
# You will also need to build Skia from source (as a shared library) and set
# SKIA_LIB_DIR before running this script. See the https link below.

if [[ -z ${SKIA_LIB_DIR:-} ]]; then
  echo "Cannot run build-example-with-skia.sh unless SKIA_LIB_DIR is set. See"
  echo "https://github.com/google/skia/blob/9b2baac1d6500a66f661b8efe453cd13ff34e40e/experimental/c-api-example/c.md#example"
  exit 1
fi

if [ ! -e iconvg-root-directory.txt ]; then
  echo "$0 should be run from the IconVG root directory."
  exit 1
fi

mkdir -p gen/bin

# ----

echo "Building gen/bin/iconvg-to-png-with-skia"

${CC:-gcc} -O3 -Wall -std=c99 \
    -DICONVG_CONFIG__ENABLE_SKIA_BACKEND \
    -I $SKIA_LIB_DIR/../.. \
    example/iconvg-to-png/iconvg-to-png.c \
    $SKIA_LIB_DIR/libskia.* \
    -lpng \
    -o gen/bin/iconvg-to-png-with-skia \
    -Wl,-rpath \
    -Wl,$SKIA_LIB_DIR

# ----

echo "Building gen/bin/iconvg-viewer-with-skia"

${CC:-gcc} -O3 -Wall -std=c99 \
    -DICONVG_CONFIG__ENABLE_SKIA_BACKEND \
    -I $SKIA_LIB_DIR/../.. \
    example/iconvg-viewer/iconvg-viewer.c \
    $SKIA_LIB_DIR/libskia.* \
    -lxcb -lxcb-image \
    -o gen/bin/iconvg-viewer-with-skia \
    -Wl,-rpath \
    -Wl,$SKIA_LIB_DIR
