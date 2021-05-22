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
#   sudo apt install libcairo2-dev libpng-dev libxcb1-dev libxcb-image0-dev

if [ ! -e iconvg-root-directory.txt ]; then
  echo "$0 should be run from the IconVG root directory."
  exit 1
fi

mkdir -p gen/bin

${CC:-gcc} -O3 -Wall -std=c99 \
    -DICONVG_CONFIG__ENABLE_CAIRO_BACKEND \
    example/iconvg-to-png/iconvg-to-png.c \
    -lcairo -lpng \
    -o gen/bin/iconvg-to-png-with-cairo

echo "Built gen/bin/iconvg-to-png-with-cairo"

${CC:-gcc} -O3 -Wall -std=c99 \
    -DICONVG_CONFIG__ENABLE_CAIRO_BACKEND \
    example/iconvg-viewer/iconvg-viewer.c \
    -lcairo -lxcb -lxcb-image \
    -o gen/bin/iconvg-viewer-with-cairo

echo "Built gen/bin/iconvg-viewer-with-cairo"
