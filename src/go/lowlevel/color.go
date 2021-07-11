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

package lowlevel

import (
	"image/color"
)

// TODO: convert from FFV0's CREG (32-bit) to FFV1's REGS (64-bit).

func validAlphaPremulColor(c color.RGBA) bool {
	return c.R <= c.A && c.G <= c.A && c.B <= c.A
}

// colorType distinguishes types of Colors.
type colorType uint8

const (
	// colorTypeRGBA is a direct RGBA color.
	colorTypeRGBA colorType = iota

	// colorTypePaletteIndex is an indirect color, indexing the custom palette.
	colorTypePaletteIndex

	// colorTypeCReg is an indirect color, indexing the CREG color registers.
	colorTypeCReg

	// colorTypeBlend is an indirect color, blending two other colors.
	colorTypeBlend
)

// Color is an IconVG color, whose RGBA values can depend on context. Some
// Colors are direct RGBA values. Other Colors are indirect, referring to an
// index of the custom palette, a color register of the decoder virtual
// machine, or a blend of two other Colors.
//
// See the "Colors" section in the specification for details.
type Color struct {
	typ  colorType
	data color.RGBA
}

func (c Color) rgba() color.RGBA         { return c.data }
func (c Color) paletteIndex() uint8      { return c.data.R }
func (c Color) cReg() uint8              { return c.data.R }
func (c Color) blend() (t, c0, c1 uint8) { return c.data.R, c.data.G, c.data.B }

// Resolve resolves the Color's RGBA value, given its context: the custom
// palette and the color registers of the decoder virtual machine.
func (c Color) Resolve(pal *Palette, cReg *[64]color.RGBA) color.RGBA {
	switch c.typ {
	case colorTypeRGBA:
		return c.rgba()
	case colorTypePaletteIndex:
		return pal[c.paletteIndex()&0x3f]
	case colorTypeCReg:
		return cReg[c.cReg()&0x3f]
	}
	t, c0, c1 := c.blend()
	p, q := uint32(255-t), uint32(t)
	rgba0 := decodeColor1(c0).Resolve(pal, cReg)
	rgba1 := decodeColor1(c1).Resolve(pal, cReg)
	return color.RGBA{
		uint8(((p * uint32(rgba0.R)) + q*uint32(rgba1.R) + 128) / 255),
		uint8(((p * uint32(rgba0.G)) + q*uint32(rgba1.G) + 128) / 255),
		uint8(((p * uint32(rgba0.B)) + q*uint32(rgba1.B) + 128) / 255),
		uint8(((p * uint32(rgba0.A)) + q*uint32(rgba1.A) + 128) / 255),
	}
}

// RGBAColor returns a direct Color.
func RGBAColor(c color.RGBA) Color { return Color{colorTypeRGBA, c} }

// PaletteIndexColor returns an indirect Color referring to an index of the
// custom palette.
func PaletteIndexColor(i uint8) Color { return Color{colorTypePaletteIndex, color.RGBA{R: i & 0x3f}} }

// CRegColor returns an indirect Color referring to a color register of the
// decoder virtual machine.
func CRegColor(i uint8) Color { return Color{colorTypeCReg, color.RGBA{R: i & 0x3f}} }

// BlendColor returns an indirect Color that blends two other Colors. Those two
// other Colors must both be encodable as a 1 byte color.
//
// To blend a Color that is not encodable as a 1 byte color, first load that
// Color into a CREG color register, then call CRegColor to produce a Color
// that is encodable as a 1 byte color. See testdata/favicon.ivg for an
// example.
//
// See the "Colors" section in the specification for details.
//
// TODO: update this for FFV1.
func BlendColor(t, c0, c1 uint8) Color { return Color{colorTypeBlend, color.RGBA{R: t, G: c0, B: c1}} }

func decodeColor1(x byte) Color {
	if x >= 0x80 {
		if x >= 0xc0 {
			return CRegColor(x)
		} else {
			return PaletteIndexColor(x)
		}
	}
	switch x {
	case 0:
		return RGBAColor(color.RGBA{0x00, 0x00, 0x00, 0x00})
	case 1:
		return RGBAColor(color.RGBA{0x80, 0x80, 0x80, 0x80})
	case 2:
		return RGBAColor(color.RGBA{0xc0, 0xc0, 0xc0, 0xc0})
	}
	x -= 3
	blue := dc1Table[x%5]
	x = x / 5
	green := dc1Table[x%5]
	x = x / 5
	red := dc1Table[x]
	return RGBAColor(color.RGBA{red, green, blue, 0xff})
}

var dc1Table = [5]byte{0x00, 0x40, 0x80, 0xc0, 0xff}
