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
	"bytes"
	"fmt"
	"image/color"
)

var midDescriptions = [...]string{
	midViewBox:          "ViewBox",
	midSuggestedPalette: "Suggested Palette",
}

// Destination handles the actions decoded from an IconVG graphic's byte code.
//
// When passed to Decode, the first method called (if any) will be Reset. No
// methods will be called at all if an error is encountered in the encoded form
// before the metadata is fully decoded.
type Destination interface {
	Reset(m Metadata)

	// QueryLevelOfDetail returns whether the height-in-pixels H satisfies
	// ((lod0 <= H) && (H < lod1)).
	QueryLevelOfDetail(lod0, lod1 float32) bool

	ClosePathMoveTo(x, y float32)
	LineTo(x, y float32)
	QuadTo(x1, y1, x, y float32)
	CubeTo(x1, y1, x2, y2, x, y float32)
	Ellipse(nQuarters uint32, x1, y1, x2, y2, x, y float32)
	Parallelogram(x1, y1, x2, y2, x, y float32)

	ClosePathFill() // TODO.
}

// printer prints debug information (the disassembly) during the decode. b
// holds the byte code of a single IconVG operation. format and args contain
// human-readable commentary in fmt.Printf style.
type printer func(b []byte, format string, args ...interface{})

// DecodeOptions are the optional parameters to the Decode function.
type DecodeOptions struct {
	// Palette is an optional 64 color palette. If one isn't provided, the
	// IconVG graphic's suggested palette will be used.
	Palette *Palette
}

// Decode decodes an IconVG graphic.
//
// opts may be nil, which means to use the default options.
func Decode(dst Destination, src []byte, opts *DecodeOptions) error {
	return decode(dst, nil, nil, false, src, opts)
}

// DecodeMetadata decodes only the metadata in an IconVG graphic.
func DecodeMetadata(src []byte) (m Metadata, retErr error) {
	m.ViewBox = DefaultViewBox
	m.Palette = DefaultPalette
	if err := decode(nil, nil, &m, true, src, nil); err != nil {
		return Metadata{}, err
	}
	return m, nil
}

func decode(dst Destination, p printer, m *Metadata, metadataOnly bool, src buffer, opts *DecodeOptions) (retErr error) {
	if !bytes.HasPrefix(src, magicBytes) {
		return errInvalidMagicIdentifier
	}
	if p != nil {
		p(src[:len(magic)], "IconVG Magic Identifier\n")
	}
	src = src[len(magic):]

	nMetadataChunks, n := src.decodeNatural()
	if n == 0 {
		return errInvalidNumberOfMetadataChunks
	}
	if p != nil {
		p(src[:n], "Number of metadata chunks: %d\n", nMetadataChunks)
	}
	src = src[n:]

	if m == nil {
		m = &Metadata{}
	}
	for ; nMetadataChunks > 0; nMetadataChunks-- {
		if src, retErr = decodeMetadataChunk(p, m, src, opts); retErr != nil {
			return retErr
		}
	}
	if metadataOnly {
		return nil
	}
	if dst != nil {
		dst.Reset(*m)
	}
	return decodeBytecode(dst, p, src)
}

func decodeMetadataChunk(p printer, m *Metadata, src buffer, opts *DecodeOptions) (src1 buffer, retErr error) {
	length, n := src.decodeNatural()
	if n == 0 {
		return nil, errInvalidMetadataChunkLength
	}
	if p != nil {
		p(src[:n], "Metadata chunk length: %d\n", length)
	}
	src = src[n:]
	lenSrcWant := int64(len(src)) - int64(length)

	mid, n := src.decodeNatural()
	if n == 0 {
		return nil, errInvalidMetadataIdentifier
	}
	if mid >= uint32(len(midDescriptions)) {
		return nil, errUnsupportedMetadataIdentifier
	}
	if p != nil {
		p(src[:n], "Metadata Identifier: %d (%s)\n", mid, midDescriptions[mid])
	}
	src = src[n:]

	switch mid {
	case midViewBox:
		args := [4]float32{}
		if src, retErr = decodeCoordinates(args[:4], p, src); retErr != nil {
			return nil, retErr
		}
		m.ViewBox.Min[0] = args[0]
		m.ViewBox.Min[1] = args[1]
		m.ViewBox.Max[0] = args[2]
		m.ViewBox.Max[1] = args[3]
		if m.ViewBox.Min[0] > m.ViewBox.Max[0] || m.ViewBox.Min[1] > m.ViewBox.Max[1] ||
			isNaNOrInfinity(m.ViewBox.Min[0]) || isNaNOrInfinity(m.ViewBox.Min[1]) ||
			isNaNOrInfinity(m.ViewBox.Max[0]) || isNaNOrInfinity(m.ViewBox.Max[1]) {
			return nil, errInvalidViewBox
		}

	case midSuggestedPalette:
		if (len(src) == 0) || ((src[0] >> 6) != 0) {
			return nil, errInvalidSuggestedPalette
		}
		length := 1 + int(src[0]&0x3f)
		if p != nil {
			p(src[:1], "      %d palette colors\n", length)
		}
		src = src[1:]

		for i := 0; i < length; i++ {
			if len(src) < 4 {
				return nil, errInvalidSuggestedPalette
			}
			c := color.RGBA{
				R: src[0],
				G: src[1],
				B: src[2],
				A: src[3],
			}
			if !validAlphaPremulColor(c) {
				c = color.RGBA{0x00, 0x00, 0x00, 0xff}
			}
			if p != nil {
				p(src[:4], "      rgba(%02X:%02X:%02X:%02X)\n", c.R, c.G, c.B, c.A)
			}
			src = src[4:]
			if opts == nil || opts.Palette == nil {
				m.Palette[i] = c
			}
		}

	default:
		return nil, errUnsupportedMetadataIdentifier
	}

	if int64(len(src)) != lenSrcWant {
		return nil, errInconsistentMetadataChunkLength
	}
	return src, nil
}

func decodeBytecode(dst Destination, p printer, src buffer) (retErr error) {
	originalDst := dst
	pc := uint32(0) // 'Program counter', counting instructions.
	jumpDistRemaining := uint32(0)
	curr := [2]float32{}
	args := [6]float32{}

	for len(src) > 0 {
		if jumpDistRemaining > 0 {
			jumpDistRemaining--
			if jumpDistRemaining == 0 {
				dst = originalDst
			}
		}

		switch opcode := src[0]; opcode >> 6 {
		case 0: // Path-drawing, miscellaneous, jump and call opcodes.
			if opcode < 0x30 {
				if src, retErr = decodeLineQuadCubeTo(dst, p, src, pc, opcode, &curr); retErr != nil {
					return retErr
				}
				pc++

			} else if opcode < 0x34 {
				nQuarters := 1 + uint32(opcode&3)
				if p != nil {
					p(src[:1], "#%04d Ellipse (%d quarters)\n", pc, nQuarters)
					pc++
				}
				src = src[1:]
				if src, retErr = decodeCoordinates(args[:4], p, src); retErr != nil {
					return retErr
				}
				if dst != nil {
					dst.Ellipse(nQuarters, args[0], args[1], args[2], args[3], curr[0], curr[1])
				}

			} else {
				switch opcode & 0x0F {
				case 0x04:
					if p != nil {
						p(src[:1], "#%04d Parallelogram\n", pc)
						pc++
					}
					src = src[1:]
					if src, retErr = decodeCoordinates(args[:4], p, src); retErr != nil {
						return retErr
					}
					if dst != nil {
						dst.Parallelogram(args[0], args[1], args[2], args[3], curr[0], curr[1])
					}

				case 0x05:
					if p != nil {
						p(src[:1], "#%04d ClosePath; MoveTo\n", pc)
						pc++
					}
					src = src[1:]
					if src, retErr = decodeCoordinates(curr[:2], p, src); retErr != nil {
						return retErr
					}
					if dst != nil {
						dst.ClosePathMoveTo(curr[0], curr[1])
					}

				case 0x06:
					if len(src) < 2 {
						return errInvalidNumber
					}
					delta := src[1] & 63
					if p != nil {
						p(src[:2], "#%04d SEL += %d\n", pc, delta)
						pc++
					}
					src = src[2:]
					if dst != nil {
						// TODO: set register state.
					}

				case 0x07:
					if p != nil {
						p(src[:1], "#%04d NOP\n", pc)
						pc++
					}
					src = src[1:]

				case 0x08:
					if p != nil {
						p(src[:1], "#%04d Jump Unconditional\n", pc)
						pc++
					}
					src = src[1:]
					jumpDist, n := src.decodeNatural()
					if n == 0 {
						return errInvalidNumber
					}
					if p != nil {
						p(src[:n], "      Target: #%04d (PC+%d)\n", pc+jumpDist, jumpDist)
					}
					src = src[n:]
					if dst != nil {
						dst = nil
						jumpDistRemaining = jumpDist + 1
					}

				case 0x09:
					if p != nil {
						p(src[:1], "#%04d Jump Feature-Bits\n", pc)
						pc++
					}
					src = src[1:]
					jumpDist, n := src.decodeNatural()
					if n == 0 {
						return errInvalidNumber
					}
					if p != nil {
						p(src[:n], "      Target: #%04d (PC+%d)\n", pc+jumpDist, jumpDist)
					}
					src = src[n:]
					fBits, n := src.decodeNatural()
					if n == 0 {
						return errInvalidNumber
					}
					if p != nil {
						p(src[:n], "      FeatureBits: 0x%08X\n", fBits)
					}
					src = src[n:]
					// This decoder doesn't support any feature bits.
					if dst != nil {
						dst = nil
						jumpDistRemaining = jumpDist + 1
					}

				case 0x0A:
					if p != nil {
						p(src[:1], "#%04d Jump Level-of-Detail\n", pc)
						pc++
					}
					src = src[1:]
					jumpDist, n := src.decodeNatural()
					if n == 0 {
						return errInvalidNumber
					}
					if p != nil {
						p(src[:n], "      Target: #%04d (PC+%d)\n", pc+jumpDist, jumpDist)
					}
					src = src[n:]
					lod := [2]float32{}
					if src, retErr = decodeCoordinates(lod[:2], p, src); retErr != nil {
						return retErr
					}
					if (dst != nil) && !dst.QueryLevelOfDetail(lod[0], lod[1]) {
						dst = nil
						jumpDistRemaining = jumpDist + 1
					}

				case 0x0B:
					if p != nil {
						p(src[:1], "#%04d RET\n", pc)
						pc++
					}
					src = src[1:]
					if dst != nil {
						return nil
					}

				default:
					return fmt.Errorf("iconvg: TODO: call opcodes")
				}
			}

		case 1: // Set-register opcodes.
			decr, adj := "", opcode&0x0F
			if adj == 0 {
				decr = "; SEL--"
			}
			switch (opcode >> 4) & 3 {
			case 0:
				if p != nil {
					p(src[:1], "#%04d Set REGS[SEL+%d].lo32%s\n", pc, adj, decr)
					pc++
				}
				src = src[1:]
				if src, retErr = decodeSetRegLo32(dst, p, src); retErr != nil {
					return retErr
				}
			case 1:
				if p != nil {
					p(src[:1], "#%04d Set REGS[SEL+%d].hi32%s\n", pc, adj, decr)
					pc++
				}
				src = src[1:]
				if src, retErr = decodeSetRegHi32(dst, p, src); retErr != nil {
					return retErr
				}
			case 2:
				if p != nil {
					p(src[:1], "#%04d Set REGS[SEL+%d]%s\n", pc, adj, decr)
					pc++
				}
				src = src[1:]
				if src, retErr = decodeSetRegLo32(dst, p, src); retErr != nil {
					return retErr
				}
				if src, retErr = decodeSetRegHi32(dst, p, src); retErr != nil {
					return retErr
				}
			case 3:
				if p != nil {
					p(src[:1], "#%04d SEL -= %d; Set REGS[SEL+1 .. SEL+%d]\n", pc, adj+2, adj+3)
					pc++
				}
				src = src[1:]
				for i := 0; i < int(adj+2); i++ {
					if src, retErr = decodeSetRegLo32(dst, p, src); retErr != nil {
						return retErr
					}
					if src, retErr = decodeSetRegHi32(dst, p, src); retErr != nil {
						return retErr
					}
				}
			}

		case 2: // Fill opcodes.
			incr, adj := "", opcode&0x0F
			if adj == 0 {
				incr = "SEL++; "
			}

			switch o := (opcode >> 4) & 3; o {
			case 0:
				if p != nil {
					p(src[:1], "#%04d ClosePath; %sFill (flat color) with REGS[SEL+%d]\n", pc, incr, adj)
					pc++
				}
				src = src[1:]
				if dst != nil {
					dst.ClosePathFill()
				}

			case 1, 2:
				if len(src) < 2 {
					return errInvalidColor
				}
				grad := "linear"
				if o == 2 {
					grad = "radial"
				}
				if p != nil {
					p(src[:2], "#%04d ClosePath; %sFill (%s gradient; %s) with REGS[SEL+%d .. SEL+%d]\n",
						pc, incr, grad, gradientSpreadNames[src[1]>>6], adj, adj+2+(src[1]&63))
					pc++
				}
				src = src[2:]
				for i := 0; i < int(3*o); i++ {
					f, n := src.decodeFloat32()
					if n == 0 {
						return errInvalidNumber
					}
					if p != nil {
						p(src[:n], "      %+g\n", f)
					}
					args[i] = f
					src = src[n:]
				}
				if dst != nil {
					dst.ClosePathFill()
				}

			case 3:
				if p != nil {
					p(src[:1], "#%04d ClosePath; %sFill (reserved) with REGS[SEL+%d]\n", pc, incr, adj)
					pc++
				}
				src = src[1:]
				if src, retErr = decodeExtraData(dst, p, src); retErr != nil {
					return retErr
				}
				if dst != nil {
					dst.ClosePathFill()
				}
			}

		default:
			if src, retErr = decodeReservedOpcodes(dst, p, src); retErr != nil {
				return retErr
			}
			pc++
		}
	}
	return nil
}

func oneByteColorString(x byte) string {
	c := decodeColor1(x)
	switch c.typ {
	case colorTypePaletteIndex:
		return fmt.Sprintf("CPAL[%d]", c.paletteIndex())
	case colorTypeCReg:
		return fmt.Sprintf("REGS[INDEX+%d]", c.cReg())
	}
	rgba := c.rgba()
	return fmt.Sprintf("rgba(%02X:%02X:%02X:%02X)", rgba.R, rgba.G, rgba.B, rgba.A)
}

func decodeLineQuadCubeTo(dst Destination, p printer, src buffer, pc uint32, opcode byte, curr *[2]float32) (buffer, error) {
	coords := [6]float32{}

	op, nCoords := "", 0
	switch opcode >> 4 {
	case 0:
		op = "LineTo"
		nCoords = 2
	case 1:
		op = "QuadTo"
		nCoords = 4
	case 2:
		op = "CubeTo"
		nCoords = 6
	}

	nReps := uint32(opcode & 0x0f)
	if nReps > 0 {
		if p != nil {
			p(src[:1], "#%04d %s (%d reps)\n", pc, op, nReps)
		}
		src = src[1:]
	} else {
		if p != nil {
			p(src[:1], "#%04d %s...\n", pc, op)
		}
		src = src[1:]
		n := 0
		nReps, n = src.decodeNatural()
		if n == 0 {
			return nil, errInvalidNumber
		}
		nReps += 16
		if p != nil {
			p(src[:n], "      ...(%d reps)\n", nReps)
		}
		src = src[n:]
	}

	for i := uint32(0); i < nReps; i++ {
		if p != nil && i != 0 {
			p(nil, "      (rep)\n")
		}
		err := error(nil)
		src, err = decodeCoordinates(coords[6-nCoords:6], p, src)
		if err != nil {
			return nil, err
		}
		if dst != nil {
			switch op[0] {
			case 'L':
				dst.LineTo(coords[4], coords[5])
			case 'Q':
				dst.QuadTo(coords[2], coords[3], coords[4], coords[5])
			case 'C':
				dst.CubeTo(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5])
			}
		}
		curr[0], curr[1] = coords[4], coords[5]
	}
	return src, nil
}

func decodeCoordinates(coords []float32, p printer, src buffer) (src1 buffer, retErr error) {
	for i := range coords {
		x, n := src.decodeCoordinate()
		if n == 0 {
			return nil, errInvalidNumber
		}
		if p != nil {
			p(src[:n], "      %+g\n", x)
		}
		src = src[n:]
		coords[i] = x
	}
	return src, nil
}

func decodeSetRegLo32(dst Destination, p printer, src buffer) (buffer, error) {
	if len(src) < 4 {
		return nil, errInvalidNumber
	}
	if p != nil {
		p(src[:4], "      lo32 = 0x%02X%02X_%02X%02X\n",
			src[3], src[2], src[1], src[0])
	}
	if dst != nil {
		// TODO: set register state.
	}
	return src[4:], nil
}

func decodeSetRegHi32(dst Destination, p printer, src buffer) (buffer, error) {
	if len(src) < 4 {
		return nil, errInvalidColor
	}
	cr := src[0]
	cg := src[1]
	cb := src[2]
	ca := src[3]
	if p != nil {
		if (cr <= ca) && (cg <= ca) && (cb <= ca) {
			p(src[:4], "      hi32 = rgba(%02X:%02X:%02X:%02X)\n", cr, cg, cb, ca)
		} else {
			if (src[0] == 0x00) || (src[1] == src[2]) {
				p(src[:4], "      hi32 = %s\n", oneByteColorString(src[1]))
			} else if src[0] == 0xFF {
				p(src[:4], "      hi32 = %s\n", oneByteColorString(src[2]))
			} else {
				p(src[:4], "      hi32 = blend(0x%02X * %s, 0x%02X * %s)\n",
					0xFF-src[0], oneByteColorString(src[1]),
					0x00+src[0], oneByteColorString(src[2]))
			}
		}
	}
	if dst != nil {
		// TODO: set register state.
	}
	return src[4:], nil
}

func decodeReservedOpcodes(dst Destination, p printer, src buffer) (src1 buffer, retErr error) {
	lineTo, fallback := src[0] < 0xE0, "NOP"
	if lineTo {
		fallback = "LineTo"
	}
	if p != nil {
		p(src[:1], "Reserved (%s)\n", fallback)
	}
	src = src[1:]

	if src, retErr = decodeExtraData(dst, p, src); retErr != nil {
		return nil, retErr
	}

	if lineTo {
		coords := [2]float32{}
		if src, retErr = decodeCoordinates(coords[:2], p, src); retErr != nil {
			return nil, retErr
		}
		if dst != nil {
			dst.LineTo(coords[0], coords[1])
		}
	}

	return src, nil
}

func decodeExtraData(dst Destination, p printer, src buffer) (src1 buffer, retErr error) {
	length, n := src.decodeNatural()
	if n == 0 {
		return nil, errInvalidNumber
	}
	if p != nil {
		p(src[:n], "      Extra data length: %d\n", length)
	}
	src = src[n:]

	if len(src) < int(length) {
		return nil, errInvalidExtraDataLength
	}
	extra, src := src[:length], src[length:]
	if p != nil {
		for ; len(extra) > 4; extra = extra[4:] {
			p(extra[:4], "      ???\n")
		}
		if len(extra) > 0 {
			p(extra, "      ???\n")
		}
	}
	return src, nil
}
