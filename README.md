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

- `cowbell.png` is 18555 bytes (256 × 256 pixels)
- `cowbell.svg` is  4506 bytes
- `cowbell.ivg` is  1017 bytes (see also its
  [disassembly](./test/data/cowbell.ivg.disassembly))

The [test/data](./test/data) directory holds these files and other examples.


## Implementations

The [original Go IconVG
package](https://pkg.go.dev/golang.org/x/exp/shiny/iconvg) implements a decoder
and encoder.


# Disclaimer

This is not an official Google product, it is just code that happens to be
owned by Google.


---

Updated on March 2021.
