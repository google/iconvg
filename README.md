# IconVG

IconVG is a compact, binary format for simple vector graphics: icons, logos,
glyphs and emoji.

**WARNING: THIS FORMAT IS EXPERIMENTAL AND SUBJECT TO INCOMPATIBLE CHANGES.**

It is similar in concept to SVG (Scalable Vector Graphics) but much simpler.
Compared to [SVG Tiny](https://www.w3.org/TR/SVGTiny12/), which isn't actually
tiny, it does not have features for text, multimedia, interactivity, linking,
scripting, animation, XSLT, DOM, combination with raster graphics such as JPEG
formatted textures, etc.

It is a format for efficient presentation, not an authoring format. For
example, it does not provide grouping individual paths into higher level
objects. Instead, the anticipated workflow is that artists use other tools and
authoring formats like Inkscape and SVG, or commercial equivalents, and export
IconVG versions of their assets, the same way that they would produce PNG
versions of their vector art. It is not a goal to be able to recover the
original SVG from a derived IconVG.

It is not a pixel-exact format. Different implementations may produce slightly
different renderings, due to implementation-specific rounding errors in the
mathematical computations when rasterizing vector paths to pixels. Artifacts
may appear when scaling up to extreme sizes, say 1 million by 1 million pixels.
Nonetheless, at typical scales, e.g. up to 4096 × 4096, such differences are
not expected to be perceptible to the naked eye.


## Example

![Cowbell image](./test/data/cowbell.png)

- `cowbell.png`    is 18555 bytes (256 × 256 pixels)
- `cowbell.svg`    is  4506 bytes
- `cowbell.iconvg` is  1012 bytes (see also its
  [disassembly](./test/data/cowbell.iconvg.disassembly))

The [test/data](./test/data) directory holds these files and other examples.


## File Format

- [IconVG Specification](spec/iconvg-spec.md)
- Magic number: `0x8A 0x49 0x56 0x47`, which is `"\x8aIVG"`.
- Suggested file extension: `.iconvg`
- Suggested MIME type: `image/x-iconvg`


## Implementations

This repository contains:

- a decoder [written in C](./release/c)
- a decoder [written in Dart](./src/dart), albeit for an [older (obsolete)
  version of the file format](https://github.com/google/iconvg/issues/4)
- a low-level decoder [written in Go](./src/go). Low-level means that it
  outputs numbers (vector coordinates), not pixels.

The [original Go IconVG
package](https://pkg.go.dev/golang.org/x/exp/shiny/iconvg) also implements a
decoder and encoder, albeit for an [older (obsolete) version of the file
format](https://github.com/google/iconvg/issues/4).


## Disclaimer

This is not an official Google product, it is just code that happens to be
owned by Google.


---

Updated on January 2022.
