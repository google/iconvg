# iconvg

IconVG renderer

This package contains these potentially useful APIs:

* The `IconVG` widget, which is like `IconImage` but for `IconVG` assets.

* The `IconVGImage` widget, which is like `Image` but for `IconVG` assets.

* The `RawIconVGImage` widget, which is like `IconVGImage` but works
  with raw bytes instead of an asset name.

* The `IconVGFile` class, which parses IconVG files and provides a
  `Picture`.

* The `PicturePainter` widget, which renders `Picture`s.

The widgets are available via `package:iconvg/widgets.dart`, and the
parser via `package:iconvg/decoder.dart`.
