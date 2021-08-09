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

//go:build ignore

package main

// print-one-byte-colors.go prints IconVG's first 128 1 byte colors.

import (
	"fmt"
	"os"
)

func main() {
	if err := main1(); err != nil {
		os.Stderr.WriteString(err.Error() + "\n")
		os.Exit(1)
	}
}

func main1() error {
	fmt.Printf("// iconvg_private_one_byte_colors holds the first 128 one-byte colors, in\n")
	fmt.Printf("// 0xAABBGGRR alpha-premultiplied format.\n")
	fmt.Printf("const uint32_t iconvg_private_one_byte_colors[128] = {\n")
	fmt.Printf("    0x00000000,  //\n")
	fmt.Printf("    0x80808080,  //\n")
	fmt.Printf("    0xC0C0C0C0,  //\n")
	for r := 0; r < 5; r++ {
		for g := 0; g < 5; g++ {
			for b := 0; b < 5; b++ {
				fmt.Printf("    0xFF%02X%02X%02X,  //\n", table[b], table[g], table[r])
			}
		}
	}
	fmt.Printf("};\n\n")

	fmt.Printf("const iconvg_palette iconvg_private_default_palette = {{\n")
	for i := 0; i < 64; i++ {
		fmt.Printf("    {{0x00, 0x00, 0x00, 0xFF}},  //\n")
	}
	fmt.Printf("}};\n")

	return nil
}

var table = [5]uint8{0x00, 0x40, 0x80, 0xC0, 0xFF}
