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

// Package lowlevel provides a low-level decoder for the IconVG file format.
//
// IconVG is specified at
// https://github.com/google/iconvg/blob/main/spec/iconvg-spec.md
package lowlevel

import (
	"errors"
	"image/color"
	"math"

	"golang.org/x/image/math/f32"
)

var (
	errInconsistentMetadataChunkLength = errors.New("iconvg: inconsistent metadata chunk length")
	errInvalidColor                    = errors.New("iconvg: invalid color")
	errInvalidMagicIdentifier          = errors.New("iconvg: invalid magic identifier")
	errInvalidMetadataChunkLength      = errors.New("iconvg: invalid metadata chunk length")
	errInvalidMetadataIdentifier       = errors.New("iconvg: invalid metadata identifier")
	errInvalidNumber                   = errors.New("iconvg: invalid number")
	errInvalidNumberOfMetadataChunks   = errors.New("iconvg: invalid number of metadata chunks")
	errInvalidSuggestedPalette         = errors.New("iconvg: invalid suggested palette")
	errInvalidViewBox                  = errors.New("iconvg: invalid view box")
	errUnsupportedDrawingOpcode        = errors.New("iconvg: unsupported drawing opcode")
	errUnsupportedMetadataIdentifier   = errors.New("iconvg: unsupported metadata identifier")
	errUnsupportedStylingOpcode        = errors.New("iconvg: unsupported styling opcode")
)

var gradientShapeNames = [2]string{
	"linear",
	"radial",
}

var gradientSpreadNames = [4]string{
	"none",
	"pad",
	"reflect",
	"repeat",
}

const magic = "\x89IVG"

var magicBytes = []byte(magic)

func isNaNOrInfinity(f float32) bool {
	return math.Float32bits(f)&0x7f800000 == 0x7f800000
}

// Rectangle is defined by its minimum and maximum coordinates.
type Rectangle struct {
	Min, Max f32.Vec2
}

// AspectRatio returns the Rectangle's aspect ratio. An IconVG graphic is
// scalable; these dimensions do not necessarily map 1:1 to pixels.
func (r *Rectangle) AspectRatio() (dx, dy float32) {
	return r.Max[0] - r.Min[0], r.Max[1] - r.Min[1]
}

// Palette is an IconVG palette.
type Palette [64]color.RGBA

// Metadata is an IconVG's metadata.
type Metadata struct {
	ViewBox Rectangle

	// Palette is a 64 color palette. When encoding, it is the suggested
	// palette to place within the IconVG graphic. When decoding, it is either
	// the optional palette passed to Decode, or if no optional palette was
	// given, the suggested palette within the IconVG graphic.
	Palette Palette
}

const (
	midViewBox          = 0
	midSuggestedPalette = 1
)

// DefaultViewBox is the default ViewBox. Its values should not be modified.
var DefaultViewBox = Rectangle{
	Min: f32.Vec2{-32, -32},
	Max: f32.Vec2{+32, +32},
}

// DefaultPalette is the default Palette. Its values should not be modified.
var DefaultPalette = Palette{
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
	color.RGBA{0x00, 0x00, 0x00, 0xff},
}
