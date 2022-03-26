# IconVG Specification

IconVG is a compact, binary format for simple vector graphics: icons, logos,
glyphs and emoji.

**WARNING: THIS FORMAT IS EXPERIMENTAL AND SUBJECT TO INCOMPATIBLE CHANGES.**


## Overview

An IconVG graphic consists of a Magic Identifier, one or more Metadata bytes
(8-bit octets) then a sequence of ops (also called operations, instructions or
IconVG bytecode) for a register-based VM (Virtual Machine). Ops encode a
sequence of filled paths. Conceptually, it repeats three steps:

1. Define geometry (combining linear, quadratic and cubic Bézier segments).
2. Define paint (flat colors or gradients).
3. Fill the geometry with the paint.

The second step can often be skipped, as previously defined paint (saved to VM
registers) can be re-used. Repeated geometry (including under affine
transformation) can also be factored out, like function calls in other VMs.

There is no explicit representation of path strokes: no stroke width, caps,
joins or dashes. IconVG is a presentation format, not an authoring format.
Vector graphic authoring tools should flatten strokes and groups into simple
filled paths when exporting to IconVG.


### Color

IconVG graphics work in 32-bit alpha-premultiplied color, with 8 bits for red,
green, blue and alpha. Using `RR:GG:BB:AA` color notation (four colon-separated
two-hexadecimal-digit numbers), alpha-premultiplication means that
`00:C0:00:C0` represents a 75%-opaque, 100% Saturation, 100% Value green (with
120° Hue).

An RGBA 4-tuple is sensible (under alpha-premultiplication) if `(R <= A)` and
`(G <= A)` and `(B <= A)`. Depending on further context, a non-sensible value
may mean an invalid IconVG file or it might still be valid (if there is an
alternative interpretation of the 4-tuple).

Interpolation between explicit gradient stops also uses alpha-premultiplied
color, unlike SVG. The color `80:80:80:80`, almost-halfway between opaque
bright red `FF:00:00:FF` and transparent black `00:00:00:00`, is a 50% opaque
bright red, not a 50% opaque dark red. The halfway point still has 100%
Saturation and 100% Value (in the HSV Hue Saturation Value sense). It just has
smaller alpha.


### Global Alpha and Transformation Matrices

VM state includes a Global Alpha value `Gα`, a fractional value ranging between
0 and 1 inclusive at 1/255 granularity, initialized to 1.

VM state includes a 3-by-2 Global Forward Transformation Matrix `GFTM = [Fa,
Fb, Fc; Fd, Fe, Ff]` of floating point values. Implementations may use
`float32`, `float64` or other mechanisms for floating point computation. IconVG
is not a pixel-exact image format.

For any coordinate pair `(x, y)`, the transformed point `GFTM(x, y)` is:

    GFTMx = (Fa * x) + (Fb * y) + Fc
    GFTMy = (Fd * x) + (Fe * y) + Ff

The GFTM implicitly defines a Global Backwards Transformation Matrix `GBTM =
[Ba, Bb, Bc; Bd, Be, Bf]`, given by:

    FDet = (Fa * Fe) - (Fb * Fd)
    Ba = +Fe / FDet
    Bb = -Fb / FDet
    Bc = ((Fb * Ff) - (Fe * Fc)) / FDet
    Bd = -Fd / FDet
    Be = +Fa / FDet
    Bf = ((Fd * Fc) - (Fa * Ff)) / FDet

Any update to the GFTM implicitly updates the GBTM.

The GFTM and GBTM are both initialized to the identity matrix `[1, 0, 0; 0, 1,
0]`.

If the determinant `FDet` is infinite (in the floating point sense) or if its
absolute value is less than `1e-20` then IconVG implementations may set the
`GBTM` to the identity matrix.


## SegRefs

A SegRef (Segment Reference) locates a slice of the IconVG file, called the
segment contents. The location consists of a `uint64` Segment Offset (the
number of bytes relative to the start of the file) and a `uint64` Segment
Length, both measured in bytes. It is invalid for the Segment Offset plus
Segment Length to overflow a `uint64`.

A SegRef also includes a `uint8` Segment Type describing the contents'
semantics. `0x00` means IconVG bytecode. All other values are reserved.

A SegRef encodes in 8 bytes and there are three forms within that. The first
two have restrictions on the Segment Offset and Segment Length:

- An Inline SegRef's low 8 bits hold the Segment Type. The 24 bits after that
  hold the Segment Length. The high 32 bits are all zero. The Segment Offset is
  implicit: it immediately follows those 8 bytes.
- An Absolute SegRef (Direct)'s highest bit must be zero. The low 32 bits pack
  the same as for an Inline SegRef: 8 bits Segment Type then 24 bits Segment
  Length. The 31 remaining bits hold the Segment Offset and cannot all be zero.
- An Absolute SegRef (Indirect)'s highest bit must be one. The low 8 bits hold
  the Segment Type. The middle 55 bits are a file offset locating 16 additional
  bytes that hold a 64-bit Segment Length and then a 64-bit Segment Offset.

Throughout IconVG, integers are unsigned and little-endian unless otherwise
noted.


## Registers

VM state includes 64 general registers `REGS[0], REGS[1], ..., REGS[63]`. Each
register is 64 bits wide. Register indexing is done modulo 64, so `REGS[70]` is
the same as `REGS[6]`, and `REGS[-1]` is the same as `REGS[63]`.

VM state includes a selector register `SEL`. It is effectively a 6 bit integer,
as it indexes `REGS`.

A register's low 32 bits can hold a gradient stop position. Its high 32 bits
can hold a color. A future IconVG version may assign further semantics to those
bits.


### Registers' Low 32 Bits

Each register's low 32 bits represents an unsigned 16.16 fixed point value for
a gradient stop position. For example, if `REGS[7] = 0xFEDC_BA98_0000_4000`
then its low 32 bits represent the fraction `0.25`.


### Registers' High 32 Bits

Each register's high 32 bits represents an alpha-premultiplied RGBA color,
directly or indirectly. Letting `[I ..= J]` and `[I .. J]` denote inclusive and
half-exclusive ranges:

- Bits `[32 ..= 39]` hold the `R` (red) channel.
- Bits `[40 ..= 47]` hold the `G` (green) channel.
- Bits `[48 ..= 55]` hold the `B` (blue) channel.
- Bits `[56 ..= 63]` hold the `A` (alpha) channel.

If those four values form a sensible alpha-premultiplied color then the
represented color is simply that color. For example, if `REGS[40] =
0xC000_C000_7654_3210` then its high 32 bits represent that same 75%-opaque
green, `00:C0:00:C0`, mentioned above.

If those four values do not form a sensible alpha-premultiplied color then the
high 32 bits represent a linear blend of two other colors:

- Bits `[32 ..= 39]` hold the `Blend`.
- Bits `[40 ..= 47]` hold the `ColRef0` Color Reference.
- Bits `[48 ..= 55]` hold the `ColRef1` Color Reference.
- Bits `[56 ..= 63]` is ignored (other than the 4-tuple being non-sensible).

`Blend` weights the two Color References. A `Blend` value of `64` out of `255`
means that the resultant color is roughly three quarters `ColRef0` and one
quarter `ColRef1`. Specifically, if the Color References `ColRef0` and
`ColRef1` resolve to RGBA colors `Color0` and `Color1` then the resultant `R`
(red) value is given by:

    Weight0 = 255 - Blend
    Weight1 = Blend
    Resultant.R = ((Weight0 * Color0.R) + (Weight1 * Color1.R) + 128) / 255

This is rounded down to an integer value (the mathematical floor function), and
likewise for the green, blue and alpha channels.


### Color References

There are three categories of 1-byte Color References:

- Values `[0x00 ..= 0x7F]` index the 128-entry Built-In Palette.
- Values `[0x80 ..= 0xBF]` index the 64-entry Custom Palette.
- Values `[0xC0 ..= 0xFF]` are an index-offset to another `REGS` register.

For the last category, the Color Reference value is added (modulo 64) to the
index of the original blended color to identify another register (or the same
register if the index-offset is `0xC0`), called the target register.

If the target register's high 32 bits holds a sensible alpha-premultiplied
color then resolving the Color Reference produces that color. Otherwise, it
resolves to transparent black `00:00:00:00`. Either way, resolution always
produces a sensible alpha-premultiplied color with no further recursion to
blend other Color References.

For example, if `REGS[21] = 0x0081_D340_7654_3210` then its high 32 bits
represent a blended color that is roughly three quarters (`(255 - 64) / 255 ≈
0.749`) of an index-offset color (the Color Reference `0xD3`) and roughly one
quarter of the 2nd element of the Custom Palette (the Color Reference `0x81`).
Starting from `REGS[21]`, the index-offset Color Reference `0xD3` points to the
target register `REGS[(21 + 0xD3) % 64]`, which is `REGS[40]`.


### 128-Entry Built-In Palette

The first three entries are `00:00:00:00`, `80:80:80:80` and `C0:C0:C0:C0`. The
remaining `125 = 5 * 5 * 5` entries are opaque (their alpha value is `0xFF`)
whose red, green and blue values are quantized to `0x00`, `0x40`, `0x80`,
`0xC0` or `0xFF`. These 128 distinct colors are arranged so that their
`RR:GG:BB:AA` values (as a little-endian `uint32`) are in increasing order. The
first and last twelve elements of the built-in palette are therefore:

- `built_in[0x00] = 00:00:00:00`
- `built_in[0x01] = 80:80:80:80`
- `built_in[0x02] = C0:C0:C0:C0`
- `built_in[0x03] = 00:00:00:FF`
- `built_in[0x04] = 40:00:00:FF`
- `built_in[0x05] = 80:00:00:FF`
- `built_in[0x06] = C0:00:00:FF`
- `built_in[0x07] = FF:00:00:FF`
- `built_in[0x08] = 00:40:00:FF`
- `built_in[0x09] = 40:40:00:FF`
- `built_in[0x0A] = 80:40:00:FF`
- `built_in[0x0B] = C0:40:00:FF`
- etc
- `built_in[0x74] = C0:80:FF:FF`
- `built_in[0x75] = FF:80:FF:FF`
- `built_in[0x76] = 00:C0:FF:FF`
- `built_in[0x77] = 40:C0:FF:FF`
- `built_in[0x78] = 80:C0:FF:FF`
- `built_in[0x79] = C0:C0:FF:FF`
- `built_in[0x7A] = FF:C0:FF:FF`
- `built_in[0x7B] = 00:FF:FF:FF`
- `built_in[0x7C] = 40:FF:FF:FF`
- `built_in[0x7D] = 80:FF:FF:FF`
- `built_in[0x7E] = C0:FF:FF:FF`
- `built_in[0x7F] = FF:FF:FF:FF`


### Hit Testing

Recall the `REGS` register bit assignments for blended colors:

- Bits `[32 ..= 39]` hold the `Blend`.
- Bits `[40 ..= 47]` hold the `ColRef0` Color Reference.
- Bits `[48 ..= 55]` hold the `ColRef1` Color Reference.
- Bits `[56 ..= 63]` is ignored (other than the 4-tuple being non-sensible).

Since an `0x00` Color Reference is transparent black, any non-zero `Blend` with
all zeroes in the other 24 bits also resolve to transparent black. Painting
with transparent black does nothing, in terms of modifying pixels, but having
multiple transparent black values may be useful for hit testing. For example,
this invisible shape is transparent black `01:00:00:00`, this other shape is
transparent black `02:00:00:00`, etc, and rasterizers can calculate the first
invisible shape (if any) that covers a particular pixel.


### 64-Entry Custom Palette

An IconVG graphic's rasterization can be varied by a 64 color palette. For
example, an emoji graphic may be rasterized with palette color 0 for skin and
palette color 1 for hair. Decorative variations, such as different clothing,
can be implemented by palette colors possibly set to completely transparent to
switch paths off.

IconVG rasterizer software should allow users to pass an optional 64 color
palette. If one isn't provided, the Suggested Palette (see below) should be
used. Whichever palette ends up being used is called the Custom Palette.

It is invalid for any of the user-given colors to be non-sensible.

A future IconVG version may allow Metadata to associate names like "skin",
"hair" or "bow-tie" to the integer indexes of the Custom Palette.


### Register Initialization

The low 32 bits of each `REGS` element is initialized to zero. The high 32 bits
are initialized from the Custom Palette's `RR:GG:BB:AA` values in little-endian
order. `REGS[0]` copies from the Custom Palette's first entry, `REGS[1]` from
the second entry and so on.

`SEL` is initialized to 56 so that, at the start, `REGS[SEL+0 .. SEL+8]` can be
set to a vector graphic's hard-coded colors while `REGS[SEL+8 .. SEL+16]`
accesses the Custom Palette's first eight colors.


## Numbers

An IconVG file uses separate byte encodings for floating point, natural
(non-negative integer) and coordinate numbers.

Floating point numbers are always encoded in 4 bytes: the little-endian
encoding of a 32-bit IEEE 754 `float32` number. `NaN` values are invalid IconVG
but `+Inf`, `-Inf` and `-0.0` are valid. For example, [`0x04 0x00 0x80
0x3F`](https://go.dev/play/p/Wp_KptYcpMP) encodes the value
`1.000000476837158203125`.

Each natural or coordinate number occupies 1, 2 or 4 bytes in the IconVG file,
depending on the low two bits of the first byte:

- `0x00` means a 4-byte encoding.
- `0x01` means a 1-byte encoding.
- `0x02` means a 2-byte encoding.
- `0x03` means a 1-byte encoding.

It is unusual but still valid for a natural or coordinate number to use a
longer encoding when an equivalent shorter encoding exists.


### Natural Numbers

For a 1-byte encoding, the high 7 bits form an integer value in the range `[0
.. 1<<7]`. For example, `0x29` encodes the natural number `0x14` or, in
decimal, `20`.

For a 2-byte encoding, the high 14 bits form an integer in the range `[0 ..
1<<14]`. For example, `0x5A 0x83` encodes the natural number `0x20D6` or, in
decimal, `8406`.

For a 4-byte encoding, the high 30 bits form an integer in the range `[0 ..
1<<30]`. For example, `0x04 0x00 0x80 0x3F` encodes the natural number
`0xFE0_0001` or, in decimal, `266_338305`.


### Coordinate Numbers

For encodings shorter than 4 bytes, let `N` be the natural number decoding
(converted to `float32`) of those bytes.

For a 1-byte encoding, the coordinate number is `(N - 64)`, so that a 1-byte
coordinate ranges in `[-64 .. +64]` at integer granularity. For example, the
coordinate number `7` can be encoded as `0x8F`.

For a 2-byte encoding, the coordinate number is `((N - 8192) / 64)`, so that a
2-byte coordinate ranges in `[-128 .. +128]` at `1/64` granularity. For
example, the coordinate number `7.5` can be encoded as `0x82 0x87`.

For a 4-byte encoding, the coordinate number just equals the floating point
decoding. Again, `NaN` values are invalid IconVG. For example, the coordinate
`7.5` can also be encoded as [`0x00 0x00 0xF0
0x40`](https://go.dev/play/p/Wp_KptYcpMP).


## Magic Identifier

An IconVG graphic starts with the four bytes `0x8A 0x49 0x56 0x47`, which is
`"\x8aIVG"`.


## Metadata

The Metadata encoding starts with a natural number (the number of metadata
chunks in the Metadata) followed by that many chunks. Each chunk starts with a
natural number `ChunkLength`: the number of bytes remaining in the chunk,
excluding the chunk length itself. After that is a MID (Metadata Identifier)
natural number and then the MSD (MID-Specific Data), a variable number of
bytes. It is invalid for the combined MID and MSD to be shorter or longer than
`ChunkLength`.

Chunks must be presented in increasing MID order and as MIDs are natural
numbers, the minimum MID is zero. MIDs cannot be repeated. All MIDs are
optional.


### MID 8 - ViewBox

MID 8 means that the MSD contains four coordinate numbers. These are the
`MinX`, `MinY`, `MaxX` and `MaxY` of the graphic's ViewBox: its clipping
rectangle in (scalable) vector space. Individual path nodes may be outside of
the ViewBox but, when filling paths, only pixels within the clipping rectangle
are affected.

Coordinates are in abstract units and one unit does not necessarily mean one
pixel. The ViewBox's aspect ratio is also a hint (but not a mandate) for the
rasterized pixels' aspect ratio. Implementations may rasterize a "naturally
3:2" IconVG graphic as 300x200 or as 45x30 but may also (non-uniformly) scale
it to create a 100x100 pixel image.

A zero-width or zero-height ViewBox (i.e. a degenerate clipping rectangle) is
valid, resulting in an empty but valid graphic in the same way that an empty
character string is valid.

If this MID is not present, the ViewBox defaults to `(-32, -32, +32, +32)`. A
ViewBox is invalid if `(MinX > MaxX)`, if `(MinY > MaxY)` or if any of those
four coordinate numbers are infinite (or `NaN`).


### MID 16 - Suggested Palette

MID 16 means that the MSD contains a Suggested Palette. This provides default
values for variable colors, e.g. for an emoji's skin and hair.

The MSD starts with a 1-byte 'PalCount', invalid if `(PalCount > 63)`. There
are `(4 * (PalCount + 1))` bytes after that, being `(PalCount + 1)` RGBA colors
in `{R0, G0, B0, A0, R1, G1, etc}` byte order. It is invalid for any of these
colors to be non-sensible.

The remaining `(63 - PalCount)` entries of the suggested palette are implicitly
set to opaque black `00:00:00:FF`.

If this MID is not present, the suggested palette consists entirely of opaque
black, as black is always fashionable.


## Ops

Ops occupy a variable but integer number of bytes and the first byte of each op
is called its opcode. There are four opcode categories, identified by their
high two bits:

- Opcodes `[0x00 ..= 0x3F]` are Geometry, Miscellaneous and Control Flow ops.
- Opcodes `[0x40 ..= 0x7F]` are Register ops.
- Opcodes `[0x80 ..= 0xCF]` are Fill ops.
- Opcodes `[0xD0 ..= 0xFF]` are Reserved ops.

Some op descriptions refer to a `LOW4` value. This is the low four bits of the
opcode, in the range `[0 ..= 15]`.


## Geometry Ops

The VM state includes a Current Path, open at the Pen Position (also called
`(PPx, PPy)`). The Current Path is initially empty but starts at the origin
`(0, 0)` and the Pen Position is likewise initialized to `(0, 0)`. Geometry ops
add linear, quadratic or cubic Bézier segments to the Current Path:

- Opcodes `[0x00 ..= 0x0F]` are LineTo ops.
- Opcodes `[0x10 ..= 0x1F]` are QuadTo ops.
- Opcodes `[0x20 ..= 0x2F]` are CubeTo ops.

If `LOW4` is non-zero, let `RepCount` equal `LOW4`. Otherwise, the opcode byte
is followed by a natural number and let `RepCount` equal that natural number
plus 16. Either way, after that comes `(2 * RepCount)` for LineTo, `(4 *
RepCount)` for QuadTo or `(6 * RepCount)` for CubeTo coordinate numbers.

For example, an `0x19` opcode byte would be followed by 9 groups of 4
coordinate numbers `{x1, y1, x2, y2}`. If the groups were labeled `{a, b, ...,
i}` then the coordinate numbers appear in the sequence `{ax1, ay1, ax2, ay2,
bx1, by1, bx2, by2, ..., ix1, iy1, ix2, iy2}`. Each group adds a quadratic
Bézier segment to the Current Path, from `(PPx, PPy)` to `GFTM(x2, y2)` via
`GFTM(x1, y1)`. Each coordinate number pair is transformed by the GFTM (Global
Forward Transformation Matrix). Processing a group ends with setting the Pen
Position to the final (transformed) coordinate number pair, `GFTM(x2, y2)`.

For example, an `0x21` opcode byte would be followed by 1 group of 6 coordinate
numbers `{x1, y1, x2, y2, x3, y3}`, adding a cubic Bézier segment from `(PPx,
PPy)` to `GFTM(x3, y3)` via `GFTM(x1, y1)` and `GFTM(x2, y2)` and then setting
the Pen Position to `GFTM(x3, y3)`.

For example, an `0x00 0x15` byte sequence would be followed by 26 groups
(`0x15` encodes the natural number 10 and 10 + 16 = 26) of 2 coordinate numbers
`{x1, y1}`, each group adding a linear segment from `(PPx, PPy)` to `GFTM(x1,
y1)` and then setting the Pen Position to `GFTM(x1, y1)`.


### Ellipse and Parallelogram Ops

- Opcodes `[0x30 ..= 0x34]` are the Ellipse and Parallelogram ops.

Each of these ops are followed by exactly 4 coordinate numbers `{x1, y1, x2,
y2}`. Let `A`, `B` and `C` denote the three points `(PPx, PPy)`, `GFTM(x1, y1)`
and `GFTM(x2, y2)`. This implies a fourth point of a parallelogram,
`D = A - B + C`.

Starting from `A`, the `0x34` Parallelogram op is equivalent to a 4-segment
`LineTo` op: to `B`, to `C`, to `D` and finally to `A`. The Pen Position does
not change. It stays at `A`.

For the `[0x30 ..= 0x33]` Ellipse opcodes, define a center point
`X = (A + C) / 2` and two axis vectors `r = B - X` and `s = C - X`. These
aren't necessarily the major (longest) and minor (shortest) axes of the
ellipse. They're just two axes, derived only from `A`, `B` and `C`. We then
derive four cubic Bézier segments:

- The 1st segment's control points are `A`, `A+`, `B-` and `B`.
- The 2nd segment's control points are `B`, `B+`, `C-` and `C`.
- The 3rd segment's control points are `C`, `C+`, `D-` and `D`.
- The 4th segment's control points are `D`, `D+`, `A-` and `A`.

The 8 off-curve control points are defined by a scalar constant `k =
0.551784777779014`:

- `A- = A - k.(B-X) = A - k.r`
- `A+ = A + k.(B-X) = A + k.r`
- `B- = B - k.(C-X) = B - k.s`
- `B+ = B + k.(C-X) = B + k.s`
- `C- = C - k.(D-X) = C + k.r`
- `C+ = C + k.(D-X) = C - k.r`
- `D- = D - k.(A-X) = D + k.s`
- `D+ = D + k.(A-X) = D - k.s`

These implicit control points (and the magic number `k`) are discussed in
["Three Points (Two Opposing) Define an
Ellipse"](https://nigeltao.github.io/blog/2020/three-points-define-ellipse.html).

- The `0x30` Quarter Ellipse op adds only the 1st segment to the Current Path,
  moving the Pen Position to `B`.
- The `0x31` Half Ellipse op adds the 1st and 2nd segments, moving to `C`.
- The `0x32` Three Quarter Ellipse op adds the 1st, 2nd and 3rd segments,
  moving to `D`.
- The `0x33` Full Ellipse op adds all four segments and leaves the Pen Position
  unchanged at `A`.


### ClosePathMoveTo Op

- Opcode `0x35` is the ClosePathMoveTo op.

The ClosePathMoveTo op combines two actions (ClosePath, MoveTo) and is followed
by 2 coordinate numbers `{x1, y1}`. ClosePath closes the Current Path (adding
an implicit linear segment if the Pen Position isn't at the path start),
appending that closed path to a list of Pending Paths. MoveTo then opens a new
Current Path starting at `GFTM(x1, y1)`. The Pen Position also moves to that
point.


## Miscellaneous Ops

- Opcode `0x36` is a 2-byte op that adds (modulo 64) the second byte to `SEL`.
  For example, the byte sequence `0x36 0x02` adds 2 to `SEL`.
- Opcode `0x37` is a 1-byte NOP (no operation; do nothing).


## Control Flow Ops

VM state includes a Program Counter (PC), the file offset of the next op to
execute. It is initialized to the first byte after all of the Metadata.

The PC usually advances sequentially, being incremented by N after executing an
op that occupies N bytes. However, Jump, Call and Return ops can change the PC
in other ways.


### End Of Bytecode

VM state includes an End Of Bytecode (EOB) `uint64` value, initialized to
EOBMax (also called `UINT64_MAX = 0xFFFF_FFFF_FFFF_FFFF`).

The VM executes an implicit Return op (see below) when the PC is at the EOB or
at the end of the IconVG file. It is invalid for an op's bytes to cross the EOB
or to be incomplete because it reached the end of the IconVG file.


### Jump Ops

Each Jump opcode is followed by a natural number, `JumpCount`. These ops all
propel the PC forward (never backward), jumping over the next `JumpCount` ops.
That count excludes the Jump op itself, so that a zero `JumpCount` Jump is
effectively a NOP (and not an infinite loop). Ops are atomic and Jumps cannot
move the PC into the middle of a multi-byte op.

It is valid to jump to the EOB or the end of the IconVG file exactly (the next
op will be an implicit Return), but invalid to jump past either.

- Opcode `0x38` is an Unconditional Jump. The jump is always taken.
- Opcode `0x39` is a Feature Detection Jump (FDJump). The `JumpCount` is
  followed by a natural number `FeaturesNeeded`. The jump is taken unless the
  rasterizer provides all of those features. See "Feature Detection" below.
- Opcode `0x3A` is a Level Of Detail Jump (LODJump). The `JumpCount` is
  followed by two coordinate numbers, `LOD0` and `LOD1`. The jump is taken
  unless the rasterization's height in pixels `H` satisfies both `(LOD0 <= H)`
  and `(H < LOD1)`.

The LODJump op allows an IconVG file to provide a simpler version for small
rasterizations (e.g. below 32 pixels) and a more complex version for large
rasterizations (e.g. 32 and above pixels).


### Feature Detection

Future IconVG versions may add additional features and provide semantics to
previously reserved opcodes. The FDJump mechanism allows newer IconVG files to
instruct older or limited IconVG rasterizers to skip over the newer ops that
they do not implement and possibly jump to a fallback op sequence instead.

Rasterizers provide an ambient, read-only `FeaturesImplemented` `uint32` value,
which may be zero. IconVG reserves bits of that overall value for different
features and an FDJump op takes the jump unless the bitwise-and of the op's
`FeaturesNeeded` and the rasterizer's `FeaturesImplemented` values equals
`FeaturesNeeded`. For example, an FDJump needing `0x0000_0103` meeting a
rasterizer implementing `0x0000_00F7` would take the jump as the `0x0000_0100`
feature bit was unsatisfied.

All feature bits are reserved. The `0x0000_0001` feature bit is expected to
relate to Animation as a broad concept, although this document does not yet
give further details on how IconVG would specifically represent Animation.


### Return Op

VM state includes a Global Return Address (GRA), a file offset, initialized to
zero. IconVG bytecode calls cannot nest. The maximum call stack depth is one.

- Opcode `0x3B` is a Return op.

If the GRA is zero, the Return op ends the graphic (even if we are not at the
EOB or the end of the IconVG file). Otherwise, it resets the PC, EOB, Gα and
GFTM to the GRA, EOBMax, 1 and the identity matrix (and the GBTM to the
identity matrix) and the GRA is then reset to zero.


### Call Ops

- Opcode `0x3C` is a Call Untransformed op.
- Opcode `0x3D` is a Call Transformed op.
- Opcodes `[0x3E ..= 0x3F]` are reserved (see "Reserved Ops" below).

If the GRA is non-zero then the Call op is invalid. Otherwise, it sets the GRA
to the file offset after the last byte of the Call op.

The `0x3D` opcode is followed by an αFTM (Alpha and Forward Transformation
Matrix). An αFTM is a 1-byte alpha value (scaled by 255 so that α ranges
between 0 and 1) and then six coordinate numbers forming a 3-by-2 affine
transformation matrix `[a, b, c; d, e, f]`. The Gα and GFTM are set to these
values (which also updates the GBTM).

For the `0x3C` opcode, the Gα and GFTM (and GBTM) remain unchanged at 1 and the
identity matrix (and the identity matrix).

The αFTM (or lack of it) is followed by a 8-byte SegRef (e.g. "switch to the
Segment Type `0x2A` instruction set" or "re-use some shared geometry and
paint"). If the SegRef is Inline, the segment contents (the Segment Length
bytes following the SegRef) are considered part of this Call op, when setting
the GRA past "the last byte of the Call op" and when Jump ops jump over the op.
An Absolute SegRef's 8 bytes are also considered part of this Call op, for
those same purposes, but its segment contents are not.

Whether Inline or Absolute, the PC and EOB are set to the segment contents'
start and end offsets and, for IconVG bytecode (Segment Type `0x00`), VM
bytecode execution continues. That start and end may overlap with the top level
bytecode and the start may be in the middle of what was previously considered a
multi-byte op. Op decoding starts afresh when executing the callee bytecode.

Segment Types other than `0x00` are reserved and are invalid to Call in this
version (there is no fallback behavior). Future-versioned IconVG files that use
them should guard them with FDJump ops.


## Register Ops

- Opcodes `[0x40 ..= 0x4F]` set the low 32 bits of `REGS[SEL + LOW4]`. The high
  32 bits are set to zero. The opcode is followed by 4 bytes.
- Opcodes `[0x50 ..= 0x5F]` set the high 32 bits of `REGS[SEL + LOW4]`. The low
  32 bits are set to zero. The opcode is followed by 4 bytes.
- Opcodes `[0x60 ..= 0x6F]` set all 64 bits of `REGS[SEL + LOW4]`. The opcode
  is followed by 8 bytes (4 low bytes and then 4 high bytes).

For the three Register Op categories above, if `LOW4 == 0` then the op also
decrements `SEL` by one, after updating `REGS`.

- Opcodes `[0x70 ..= 0x7F]` first decrements `SEL` by `(LOW4 + 2)`. The opcode
  is followed by `(8 * (LOW4 + 2))` bytes, which are then copied 8 bytes at a
  time to `REGS[SEL + 1], REGS[SEL + 2], ..., REGS[SEL + LOW4 + 2]`.

When setting the low 32 bits, high 32 bits or all 64 bits of a `REGS` register,
the 4 or 8 bytes are copied from the IconVG file in little-endian order.


## Fill Ops

If `LOW4 == 0` then these Fill ops first increment `SEL` by one, before doing
anything else. Regardless of `LOW4`, they then ClosePath (the first half of the
ClosePathMoveTo op) without updating the Pen Position and then fill any Pending
Paths. Fills use the Winding fill rule ("inside" means a non-zero sum of signed
edge crossings), not the Even-Odd fill rule. The Pending Paths list is then
cleared. These paths are consumed (not preserved) and take no further part in
any future ops.

Some Fill ops provide a Nominal Gradient Matrix `NGM = [Na, Nb, Nc; Nd, Ne,
Nf]`, discussed below.

- Opcodes `[0x80 ..= 0x8F]` fill with a flat (uniform) color, resolving
  `REGS[SEL + LOW4]`.
- Opcodes `[0x90 ..= 0x9F]` fill with a linear gradient. It is followed by a
  Gradient Configuration byte and then three floating point numbers `Na`, `Nb`
  and `Nc`. The `Nd`, `Ne` and `Nf` numbers are all set to zero.
- Opcodes `[0xA0 ..= 0xAF]` fill with a radial gradient. It is followed by a
  Gradient Configuration byte and then six floating point numbers `Na`, `Nb`,
  `Nc`, `Nd`, `Ne` and `Nf`.

The Gradient Configuration's low 6 bits gives a number in the range `[0 ..=
62]`, with 63 being invalid. Adding 2 gives `NSTOPS`, the number of gradient
stops, whose position and resolved color come from the low and high 32 bits of
`REGS[SEL + LOW4 + 0], REGS[SEL + LOW4 + 1], ..., REGS[SEL + LOW4 + NSTOPS -
1]`. The stop positions (as 16.16 fixed point values) must start at
0, end at 1 and be in non-decreasing order.

The Gradient Configuration's high 2 bits give the Spread (how to extrapolate
gradient colors outside of the `[0 ..= 1]` stop position nominal range):

- `0x00` None means that stop positions below `0` and above `1` map to
  transparent black.
- `0x01` Pad means that stop positions below `0` and above `1` map to the
  colors that `0` and `1` would map to.
- `0x02` Reflect means that the stop position mapping is reflected
  start-to-end, end-to-start, start-to-end, etc.
- `0x03` Repeat means that the stop position mapping is repeated start-to-end,
  start-to-end, start-to-end, etc.


### Gradients

The Nominal Gradient Matrix `NGM = [Na, Nb, Nc; Nd, Ne, Nf]` is multiplied by
the Global Backwards Transformation Matrix `GBTM = [Ba, Bb, Bc; Bd, Be, Bf]` to
produce the Effective Gradient Matrix `EGM = [Ea, Eb, Ec; Ed, Ee, Ef]`:

    Ea = (Na * Ba) + (Nb * Bd)
    Eb = (Na * Bb) + (Nb * Be)
    Ec = (Na * Bc) + (Nb * Bf) + Nc
    Ed = (Nd * Ba) + (Ne * Bd)
    Ee = (Nd * Bb) + (Ne * Be)
    Ef = (Nd * Bc) + (Ne * Bf) + Nf

This matrix maps from graphic coordinate space (the space where the Metadata's
ViewBox lives) to gradient coordinate space. Gradient coordinate space is where
a linear gradient ranges from `x=0` to `x=1`, and a radial gradient has center
`(0, 0)` and radius `1`.

The graphic coordinate `(Px, Py)` maps to the gradient coordinate `(Dx, Dy)`
by:

    Dx = (Ea * Px) + (Eb * Py) + Ec
    Dy = (Ed * Px) + (Ee * Py) + Ef

The [Appendix below](#appendix---gradient-transformation-matrices) gives
explicit formulae for the `[Na, Nb, Nc; Nd, Ne, Nf]` affine transformation
matrix for common gradient geometry, such as a linear gradient defined by two
points.


### Gradient Example

For example, here is a 14 byte sequence for a linear gradient Fill op and its
`[Na, Nb, Nc; 0, 0, 0]` Nominal Gradient Matrix. There are five gradient stops,
from `REGS[SEL+1]` inclusive to `REGS[SEL+6]` exclusive (or equivalently, to
`REGS[SEL+5]` inclusive).

    91 43         #0008 ClosePath; Fill (linear gradient; pad) with REGS[SEL+1 .. SEL+6]
    88 88 08 3d         +0.03333333
    88 88 88 3c         +0.016666666
    24 22 22 3f         +0.63333344

Many vector graphic implementations can take a `[Na, Nb, Nc; Nd, Ne, Nf]`
transformation matrix (or its inverse) directly, but for those that cannot (and
instead take two points `(Px1, Py1)` and `(Px2, Py2)` to define a linear
gradient), the formulae from the ["Inverse Linear
Gradients"](#inverse-linear-gradients) section below recovers two such points
as `(-19, 0)` and `(5, 12)`. The delta between them is `(+24, +12)` in the x:y
ratio of +2:1. The vector `(+7, -14)`, with ratio -1:2, is orthogonal to that
delta, so an equivalent pair of points would be `(-12, -14)` and `(+12, -2)`.

This example is the second gradient ("#0008" identifies the 9th op) in the
[gradient.iconvg.disassembly](test/data/gradient.iconvg.disassembly) example
file. The [test/data](/test/data) directory holds other gradient examples.


## Reserved Ops

Future IconVG versions may redefine these ops' behavior but not how many bytes
they occupy. This version defines fallback behavior, so it is not necessary to
guard them with FDJump ops.

- Opcodes `[0x3E ..= 0x3F]` are followed by Extra Data. The fallback behavior
  is a NOP.
- Opcodes `[0xB0 ..= 0xBF]` are followed by Extra Data. The fallback behavior
  is to fill with a flat color as if the opcode was in `[0x80 ..= 0x8F]`. This
  includes pre-incrementing `SEL` when `LOW4` is zero.
- Opcodes `[0xC0 ..= 0xDF]` are followed by Extra Data and then a coordinate
  pair `(x1, y1)`. The fallback behavior is a single LineTo that point
  (transformed by GFTM). If a future IconVG version redefines these ops, the
  new semantics are expected to also move the Pen Position to this final point.
- Opcodes `[0xE0 ..= 0xFF]` are followed by Extra Data. The fallback behavior
  is a NOP.

Extra Data consists of a natural number `EDLength` and then `EDLength` bytes.
Implementations should skip over them.


## IconVG Example

An ASCII art rasterization of Material Design's "action/info" icon:

    ........................
    ........................
    ........++8888++........
    ......+8888888888+......
    .....+888888888888+.....
    ....+88888888888888+....
    ...+8888888888888888+...
    ...88888888..88888888...
    ..+88888888..88888888+..
    ..+888888888888888888+..
    ..88888888888888888888..
    ..888888888..888888888..
    ..888888888..888888888..
    ..888888888..888888888..
    ..+88888888..88888888+..
    ..+88888888..88888888+..
    ...88888888..88888888...
    ...+8888888888888888+...
    ....+88888888888888+....
    .....+888888888888+.....
    ......+8888888888+......
    ........++8888++........
    ........................
    ........................

The [SVG
form](https://github.com/google/material-design-icons/blob/3.0.0/action/svg/production/ic_info_48px.svg)
is 202 bytes, or 174 bytes after `"gzip --best"`:

    <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 48 48">
    <path d="M24 4C12.95 4 4 12.95 4 24s8.95 20 20 20 20-8.95 20-20S35.05 4 24 4z
    m2 30h-4V22h4v12zm0-16h-4v-4h4v4z"/></svg>

The PNG forms at various sizes:

    18 x 18: 207 bytes
    24 x 24: 222 bytes
    36 x 36: 321 bytes
    48 x 48: 412 bytes

The IconVG form is 36 bytes:

    8a 49 56 47 03 0b 11 51 51 b1 b1 35 81 59 33 59
    81 81 a9 35 85 95 34 7d 95 7d 7d 35 85 75 34 7d
    75 7d 6d 88

The annotated disassembly is below. Note that the IconVG ViewBox ranges from
`-24` to `+24` while the SVG viewBox ranges from `0` to `48`.

    8a 49 56 47   IconVG Magic Identifier
    03            Number of metadata chunks: 1
    0b            Metadata chunk length: 5
    11            Metadata Identifier: 8 (viewBox)
    51                  -24
    51                  -24
    b1                  +24
    b1                  +24
    35            #0000 ClosePath; MoveTo
    81                  +0
    59                  -20
    33            #0001 Ellipse (4 quarters)
    59                  -20
    81                  +0
    81                  +0
    a9                  +20
    35            #0002 ClosePath; MoveTo
    85                  +2
    95                  +10
    34            #0003 Parallelogram
    7d                  -2
    95                  +10
    7d                  -2
    7d                  -2
    35            #0004 ClosePath; MoveTo
    85                  +2
    75                  -6
    34            #0005 Parallelogram
    7d                  -2
    75                  -6
    7d                  -2
    6d                  -10
    88            #0006 ClosePath; Fill (flat color) with REGS[SEL+8]

The [test/data](/test/data) directory holds these files and other examples.


## Appendix - Gradient Transformation Matrices

This appendix derives the Nominal Gradient Matrices `[Na, Nb, Nc; Nd, Ne, Nf]`
for linear, circular and elliptical gradients.


### Linear Gradients

For a linear gradient from `(Px1, Py1)` to `(Px2, Py2)`, let `Δx = (Px2 - Px1)`
and `Δy = (Py2 - Py1)`. In gradient coordinate space, the `y`-coordinate is
ignored and so `Nd`, `Ne` and `Nf` can be arbitrary. The transformation matrix
simplifies to `[Na, Nb, Nc; 0, 0, 0]`. It satisfies the three simultaneous
equations:

    Na*(Px1   ) + Nb*(Py1   ) + Nc = 0   (eq L.0)
    Na*(Px1+Δy) + Nb*(Py1-Δx) + Nc = 0   (eq L.1)
    Na*(Px1+Δx) + Nb*(Py1+Δy) + Nc = 1   (eq L.2)

Subtracting equation `L.0` from equations `L.1` and `L.2` yields:

    Na*Δy - Nb*Δx = 0
    Na*Δx + Nb*Δy = 1

So that

    Na*Δy*Δy - Nb*Δx*Δy = 0
    Na*Δx*Δx + Nb*Δx*Δy = Δx

Overall:

    Na = Δx / (Δx*Δx + Δy*Δy)
    Nb = Δy / (Δx*Δx + Δy*Δy)
    Nc = -a*Px1 - b*Py1
    Nd = 0
    Ne = 0
    Nf = 0


### Inverse Linear Gradients

To invert the previous section's calculation, recovering `(Px1, Py1)` and
`(Px2, Py2)` from `[Na, Nb, Nc; 0, 0, 0]`, note that in the non-degenerate
case, there are infinitely many points `(Px1, Py1)` and `(Px2, Py2)`. This
section derives two of those.

Recast `(Px2, Py2)` as `(Px1+Δx, Py1+Δy)`. These two points map to `Dx=0` and
`Dx=1` in gradient space. Furthermore, rotating that delta by 90 degrees gives
another point `(Px1+Δy, Py1-Δx)` that also maps to `Dx=0`. Solve the
simultaneous equations:

    Dx1 = 0 = (Na * (Px1   )) + (Nb * (Py1   )) + Nc
    Dx2 = 1 = (Na * (Px1+Δx)) + (Nb * (Py1+Δy)) + Nc
    Dx3 = 0 = (Na * (Px1+Δy)) + (Nb * (Py1-Δx)) + Nc

To pick one out of the infinite options, set `Py1=0` to simplify:

    Dx1 = 0 = (Na * (Px1   ))              + Nc
    Dx2 = 1 = (Na * (Px1+Δx)) + (Nb * +Δy) + Nc
    Dx3 = 0 = (Na * (Px1+Δy)) + (Nb * -Δx) + Nc

The first line gives `Px1 = -Nc/Na`. Substituting that into the remaining equations:

    1 = (Na * +Δx)) + (Nb * +Δy)
    0 = (Na * +Δy)) + (Nb * -Δx)

Adding `Nb` times the first to `Na` times the second:

    Nb = (Na*Na + Nb*Nb) * Δy

So `Δy = Nb / (Na*Na + Nb*Nb)`. A similar reduction gives `Δx = Na / (Na*Na +
Nb*Nb)`. We now know `Px1`, `Py1`, `Δx` and `Δy`. To answer the original
question, going from `[Na, Nb, Nc; 0, 0, 0]` to a `(Px1, Py1)` and `(Px2, Py2)`
pair that describes the linear gradient:

    Px1 = -Nc/Na
    Py1 = 0
    Px2 = Px1 + Δx = -Nc/Na + (Na / (Na*Na + Nb*Nb))
    Py2 = Py1 + Δy =      0 + (Nb / (Na*Na + Nb*Nb))

If `Na=0` then replace "set `Py1=0` to simplify" with "set `Px1=0` to simplify"
and the same process yields similar formulae, picking a different one of the
infinitely many solutions. If both `Na=0` and `Nb=0` then it's a degenerate
gradient where every point in graphic coordinate space maps to a gradient
offset of `Nc`.


### Circular Gradients

For a circular gradient with center `(Pcx, Pcy)` and radius vector `(Prx,
Pry)`, such that `(Pcx+Prx, Pcy+Pry)` is on the circle, let

    r = math.Sqrt(Prx*Prx + Pry*Pry)

The transformation matrix maps `(Pcx, Pcy)` to `(0, 0)`, maps `(Pcx+r, Pcy)` to
`(1, 0)` and maps `(Pcx, Pcy+r)` to `(0, 1)`. Solving those six simultaneous
equations give:

    Na = +1   / r
    Nb = +0   / r
    Nc = -Pcx / r
    Nd = +0   / r
    Ne = +1   / r
    Nf = -Pcy / r


### Elliptical Gradients

For an elliptical gradient with center `(Pcx, Pcy)` and axis vectors `(Prx,
Pry)` and `(Psx, Psy)`, such that `(Pcx+Prx, Pcx+Pry)` and `(Pcx+Psx, Pcx+Psy)`
are on the ellipse, the transformation matrix satisfies the six simultaneous
equations:

    Na*(Pcx    ) + Nb*(Pcy    ) + Nc = 0   (eq E.0)
    Na*(Pcx+Prx) + Nb*(Pcy+Pry) + Nc = 1   (eq E.1)
    Na*(Pcx+Psx) + Nb*(Pcy+Psy) + Nc = 0   (eq E.2)
    Nd*(Pcx    ) + Ne*(Pcy    ) + Nf = 0   (eq E.3)
    Nd*(Pcx+Prx) + Ne*(Pcy+Pry) + Nf = 0   (eq E.4)
    Nd*(Pcx+Psx) + Ne*(Pcy+Psy) + Nf = 1   (eq E.5)

Subtracting equation `E.0` from equations `E.1` and `E.2` yields:

    Na*Prx + Nb*Pry = 1
    Na*Psx + Nb*Psy = 0

Solving these two simultaneous equations yields:

    Na = +Psy / (Prx*Psy - Psx*Pry)
    Nb = -Psx / (Prx*Psy - Psx*Pry)

Re-arranging `E.0` yields:

    Nc = -Na*Pcx - Nb*Pcy

Similarly for `Nd`, `Ne` and `Nf` so that, overall:

    Na = +Psy / (Prx*Psy - Psx*Pry)
    Nb = -Psx / (Prx*Psy - Psx*Pry)
    Nc = -Na*Pcx - Nb*Pcy
    Nd = -Pry / (Prx*Psy - Psx*Pry)
    Ne = +Prx / (Prx*Psy - Psx*Pry)
    Nf = -Nd*Pcx - Ne*Pcy

Note that if `Prx = r`, `Pry = 0`, `Psx = 0` and `Psy = r` then this simplifies
to the Circular Gradients formulae [above](#circular-gradients).


---

Updated on December 2021.
