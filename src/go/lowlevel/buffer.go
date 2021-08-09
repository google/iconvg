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
	"math"
)

// buffer holds an encoded IconVG graphic.
//
// The decodeXxx methods return the decoded value and an integer n, the number
// of bytes that value was encoded in. They return n == 0 if an error occured.
//
// The encodeXxx methods append to the buffer, modifying the slice in place.
type buffer []byte

func (b buffer) decodeNatural() (u uint32, n int) {
	if len(b) < 1 {
		return 0, 0
	}
	x := b[0]
	if x&0x01 != 0 {
		return uint32(x) >> 1, 1
	}
	if x&0x02 != 0 {
		if len(b) >= 2 {
			y := uint16(b[0]) | uint16(b[1])<<8
			return uint32(y) >> 2, 2
		}
		return 0, 0
	}
	if len(b) >= 4 {
		y := uint32(b[0]) | uint32(b[1])<<8 | uint32(b[2])<<16 | uint32(b[3])<<24
		return y >> 2, 4
	}
	return 0, 0
}

func (b buffer) decodeCoordinate() (f float32, n int) {
	switch u, n := b.decodeNatural(); n {
	case 0:
		return 0, n
	case 1:
		return float32(int32(u) - 64), n
	case 2:
		return float32(int32(u)-64*128) / 64, n
	default:
		f = math.Float32frombits(u << 2)
		if f != f { // Reject NaN.
			return 0, 0
		}
		return f, n
	}
}

func (b buffer) decodeFloat32() (f float32, n int) {
	if len(b) < 4 {
		return 0, 0
	}
	u := uint32(b[0]) | uint32(b[1])<<8 | uint32(b[2])<<16 | uint32(b[3])<<24
	f = math.Float32frombits(u)
	if f != f { // Reject NaN.
		return 0, 0
	}
	return f, 4
}

func (b *buffer) encodeNatural(u uint32) {
	if u < 1<<7 {
		u = (u << 1) | 0x01
		*b = append(*b, uint8(u))
	} else if u < 1<<14 {
		u = (u << 2) | 0x02
		*b = append(*b, uint8(u), uint8(u>>8))
	} else {
		u = (u << 2)
		*b = append(*b, uint8(u), uint8(u>>8), uint8(u>>16), uint8(u>>24))
	}
}

func (b *buffer) encodeCoordinate(f float32) {
	if i := int32(f); -64 <= i && i < +64 && float32(i) == f {
		u := uint32(i + 64)
		u = (u << 1) | 0x01
		*b = append(*b, uint8(u))
	} else if i := int32(f * 64); -128*64 <= i && i < +128*64 && float32(i) == f*64 {
		u := uint32(i + 128*64)
		u = (u << 2) | 0x02
		*b = append(*b, uint8(u), uint8(u>>8))
	} else {
		u := math.Float32bits(f)

		// Round the fractional bits (the low 23 bits) to the nearest multiple
		// of 4, being careful not to overflow into the upper bits.
		v := u & 0x007fffff
		if v < 0x007ffffe {
			v += 2
		}
		u = (u & 0xff800000) | v

		*b = append(*b, uint8(u), uint8(u>>8), uint8(u>>16), uint8(u>>24))
	}
}

func (b *buffer) encodeFloat32(f float32) {
	u := math.Float32bits(f)
	*b = append(*b, uint8(u), uint8(u>>8), uint8(u>>16), uint8(u>>24))
}
