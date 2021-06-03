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

import 'dart:typed_data';
import 'dart:ui';

import 'package:flutter/foundation.dart';
import 'package:vector_math/vector_math_64.dart';

export 'dart:typed_data' show Uint8List;
export 'dart:ui' show Color, Picture, Rect;

@immutable
/// IconVG parser.
///
/// The constructor takes a [Uint8List] of raw bytes to parse.
///
/// The output is a [picture] and the graphic coordinate space [rect].
///
/// A custom palette can be provided to the decoder using the `palette` argument
/// to the constructor. It must not be longer than 64 colors.
///
/// If the size that the image will be rendered at is known ahead of time, the
/// height that will be used can be provided using the `height` argument. The
/// default is the graphic coordinate space height.
///
/// By default the rendered image is clipped without anti-aliasing. This can be
/// controled using the constructor's `clipBehavior` argument.
class IconVGFile {
  const IconVGFile._(this.picture, this.rect);

  factory IconVGFile(Uint8List bytes, {
    List<Color>? palette,
    double height = double.infinity,
    Clip clipBehavior = Clip.hardEdge,
  }) {
    assert(palette == null || palette.length <= _IconVGMachine.kCREGLength);
    final _IconVGMachine machine = _IconVGMachine(bytes, palette: palette, height: height, clipBehavior: clipBehavior);
    return IconVGFile._(machine.picture, machine.viewBox);
  }

  /// The parsed image, rendered as a vector graphic image.
  ///
  /// This includes a clip as specified by the `clipBehavior` argument to the constructor.
  final Picture picture;

  /// The view box (graphic coordinate space).
  ///
  /// It is common for images to use negative coordinates, so the top left of
  /// this rectangle may be to the left of and above the origin.
  final Rect rect;

  /// Verifies that the IconVG decoding logic fulfills various invariants described
  /// in the IconVG specification.
  static void tests() {
    final _IconVGMachine testMachine = _IconVGMachine._testDecoders(
      // We create a test machine with a bogus program that just consists of data
      // in a carefully-crafted order so that we can then check the decoders below.
      Uint8List.fromList(<int>[
        // decodeColor1
        0x00,
        0x18,
        0x30,
        0x56,
        0x7c,
        125,
        126,
        127,
        128,
        129,
        130,
        191,
        192,
        193,
        194,
        255,
        // decodeColor2
        0x38, 0x0F,
        // decodeDirectColor3
        0x30, 0x66, 0x07,
        // decodeColor4
        0x30, 0x66, 0x07, 0x80,
        // decodeIndirectColor3
        0x40, 0x7F, 0x82,
        // numbers
        0x28,
        0x59, 0x83,
        0x07, 0x00, 0x80, 0x3F,
        0x28,
        0x59, 0x83,
        0x07, 0x00, 0x80, 0x3F,
        0x8E,
        0x81, 0x87,
        0x03, 0x00, 0xF0, 0x40,
        0x0A,
        0x41, 0x1A,
        0x63, 0x0B, 0x36, 0x3B,
      ]),
      List<Color>.generate(_IconVGMachine.kCREGLength, (int index) => Color(0xFF000000 + (2 * index + 1))),
    );
    assert(testMachine.decodeColor1() == 0x000000FF);
    assert(testMachine.decodeColor1() == 0x00FFFFFF);
    assert(testMachine.decodeColor1() == 0x40FFC0FF);
    assert(testMachine.decodeColor1() == 0xC08040FF);
    assert(testMachine.decodeColor1() == 0xFFFFFFFF);
    assert(testMachine.decodeColor1() == 0xC0C0C0C0);
    assert(testMachine.decodeColor1() == 0x80808080);
    assert(testMachine.decodeColor1() == 0x00000000);
    assert(testMachine.decodeColor1() == 0x000001FF);
    assert(testMachine.decodeColor1() == 0x000003FF);
    assert(testMachine.decodeColor1() == 0x000005FF);
    assert(testMachine.decodeColor1() == 0x00007FFF);
    assert(testMachine.decodeColor1() == 0x000001FF);
    assert(testMachine.decodeColor1() == 0x000003FF);
    assert(testMachine.decodeColor1() == 0x000005FF);
    assert(testMachine.decodeColor1() == 0x00007FFF);
    assert(testMachine.decodeColor2() == 0x338800FF);
    assert(testMachine.decodeDirectColor3() == 0x306607FF);
    assert(testMachine.decodeColor4() == 0x30660780);
    testMachine.customPalette[2] = 0xFF9000FF; // "opaque orange"
    assert(testMachine.decodeIndirectColor3() == 0x40240040); // "25% opaque orange" (premultiplied)
    assert(testMachine.decodeNaturalNumber() == 0x14);
    assert(testMachine.decodeNaturalNumber() == 0x20D6);
    assert(testMachine.decodeNaturalNumber() == 0xFE00001);
    assert(testMachine.decodeRealNumber() == 20.0);
    assert(testMachine.decodeRealNumber() == 8406.0);
    assert(testMachine.decodeRealNumber() == 1.000000476837158203125);
    assert(testMachine.decodeCoordinateNumber() == 7.0);
    assert(testMachine.decodeCoordinateNumber() == 7.5);
    assert(testMachine.decodeCoordinateNumber() == 7.5);
    assert(testMachine.decodeZeroToOneNumber() == 15.0 / 360.0);
    assert(testMachine.decodeZeroToOneNumber() == 40.0 / 360.0);
    assert(testMachine.decodeZeroToOneNumber() == 0.00277777761220932); // approx 1.0/360.0
  }
}

enum _RenderingMode { styling, drawing }
enum _DrawingCommand { other, quadratic, cubic }

class _IconVGMachine {
  _IconVGMachine(this.bytes, {
    List<Color>? palette,
    required double height,
    required this.clipBehavior,
  }) : assert(palette == null || palette.length <= kCREGLength) {
    checkSignature();
    if (palette != null) {
      applyCustomPalette(palette);
    }
    readMetadata();
    if (foundPalette) {
      assert(CREG.length == customPalette.length);
      CREG.setRange(0, customPalette.length, customPalette);
    }
    if (!foundViewBox) {
      minX = -32.0;
      maxX = 32.0;
      minY = -32.0;
      maxY = 32.0;
    }
    viewBox = Rect.fromLTRB(minX, minY, maxX, maxY);
    H = height.isFinite ? height : maxY - minY; // TODO(ianh): consider the conclusions in https://github.com/google/iconvg/issues/20
    execute();
  }

  _IconVGMachine._testDecoders(this.bytes, List<Color> palette) : H = 0.0, clipBehavior = Clip.none {
    applyCustomPalette(palette);
    assert(foundPalette);
    assert(CREG.length == customPalette.length);
    CREG.setRange(0, customPalette.length, customPalette);
  }

  final Clip clipBehavior;

  late final Picture picture;
  late final Rect viewBox;

  late final double minX, maxX, minY, maxY;
  bool foundViewBox = false;

  final Uint8List bytes;
  int cursor = 0;

  int get nextByte {
    if (cursor >= bytes.length) {
      throw FormatException('Unexpected end of file at offset $cursor.');
    }
    return bytes[cursor++];
  }

  static int ColorToRGBA(Color color) {
    final int alpha = color.value >> 24;
    final int red = (((color.value & 0x00FF0000) >> 16) * alpha) ~/ 255;
    final int green = (((color.value & 0x0000FF00) >> 8) * alpha) ~/ 255;
    final int blue = ((color.value & 0x000000FF) * alpha) ~/ 255;
    return (red << 24) + (green << 16) + (blue << 8) + alpha;
  }

  static Color RGBAToColor(int color) {
    final int alpha = color & 0x000000FF;
    if (alpha == 0x00) {
      // We sometimes (for gradients) add in real color to the zero-alpha colors.
      // These don't need to be unmultiplied further, they're already unmultiplied.
      return Color((color & 0xFFFFFF00) >> 8);
    }
    assert(!isNonColor(color));
    assert(((color & 0xFF000000) >> 24) <= alpha);
    final int red = (((color & 0xFF000000) >> 24) * 255) ~/ alpha;
    assert(((color & 0x00FF0000) >> 16) <= alpha);
    final int green = (((color & 0x00FF0000) >> 16) * 255) ~/ alpha;
    assert(((color & 0x0000FF00) >> 8) <= alpha);
    final int blue = (((color & 0x0000FF00) >> 8) * 255) ~/ alpha;
    return Color((alpha << 24) + (red << 16) + (green << 8) + blue);
  }

  static const int kCREGLength = 64;
  static const int kNREGLength = 64;

  final Uint32List customPalette = Uint32List.fromList(List<int>.filled(kCREGLength, 0x000000FF));
  bool foundPalette = false;

  final Uint32List CREG = Uint32List.fromList(List<int>.filled(kCREGLength, 0x000000FF));
  final Float32List NREG = Float32List(kNREGLength);
  late final double H;

  int CSEL = 0;
  int NSEL = 0;

  double LOD0 = 0.0;
  double LOD1 = double.infinity;

  static bool isNonColor(int color) {
    return (((color & 0xFF000000) >> 24) > (color & 0x000000FF))
        || (((color & 0x00FF0000) >> 16) > (color & 0x000000FF))
        || (((color & 0x0000FF00) >> 8) > (color & 0x000000FF));
  }
  static bool isGradient(int color) => (color & 0x000080FF) == 0x00008000;

  Gradient RGBAToGradient(int color) {
    assert(isGradient(color));
    int NSTOPS = (color >> 24) & 0x3F;
    final int CBASE = (color >> 16) & 0x3F;
    final TileMode tileMode;
    switch (color & 0x00C00000) {
      case 0x00000000: // None
        tileMode = TileMode.decal;
        break;
      case 0x00400000: // Pad
        tileMode = TileMode.clamp;
        break;
      case 0x00800000: // Reflect
        tileMode = TileMode.mirror;
        break;
      case 0x00C00000: // Repeat
        tileMode = TileMode.repeated;
        break;
      default:
        throw StateError('unreachable');
    }
    final int NBASE = (color >> 8) & 0x3F;
    final bool isRadial = (color & 0x00004000) == 0x00004000;
    final List<int> rawColors = List<int>.generate(NSTOPS, (int index) => CREG[(CBASE + index) % kCREGLength]).toList(); // "Color stops" in IconVG
    final List<double> colorStops = List<double>.generate(NSTOPS, (int index) => NREG[(NBASE + index) % kNREGLength]); // "Offset stops" in IconVG

    // IconVG uses premultiplied alpha, but Gradient uses straight alpha.
    //
    // When the alpha channel is non-zero, RGBAToColor unmultiplies the color
    // correctly, but when alpha is zero, there's no way for it to know what the
    // color actually should be. So here we replace fully-transparent colors
    // with straight-alpha alternatives. RGBAToColor (used below to generate the
    // list of Colors) does not remultiply colors that have been affected by
    // this code.
    //
    // As part of this we sometimes have to double-up any fully-transparent
    // stops since the colors on either side may be different.
    for (int index = 0; index < NSTOPS; index += 1) {
      if (rawColors[index] == 0x00000000) {
        if (index > 0) {
          rawColors[index] = rawColors[index - 1] & 0xFFFFFF00;
        }
        if (index < NSTOPS - 1) {
          if ((rawColors[index + 1] & 0xFFFFFF00) != rawColors[index]) {
            rawColors.insert(index + 1, rawColors[index + 1] & 0xFFFFFF00);
            colorStops.insert(index + 1, colorStops[index]);
            index += 1;
            NSTOPS += 1;
          } else {
            rawColors[index] = rawColors[index + 1] & 0xFFFFFF00;
          }
        }
      }
    }
    final List<Color> colors = rawColors.map<Color>((int value) => RGBAToColor(value)).toList();

    final double a = NREG[(NBASE - 6) % kNREGLength];
    final double b = NREG[(NBASE - 5) % kNREGLength];
    final double c = NREG[(NBASE - 4) % kNREGLength];
    final double d = NREG[(NBASE - 3) % kNREGLength];
    final double e = NREG[(NBASE - 2) % kNREGLength];
    final double f = NREG[(NBASE - 1) % kNREGLength];
    if (isRadial) {
      final Matrix4 matrix = Matrix4.identity();
      matrix[0] = a;
      matrix[1] = d;
      matrix[4] = b;
      matrix[5] = e;
      matrix[12] = c;
      matrix[13] = f;
      if (matrix.invert() == 0.0) {
        // TODO(ianh): apply decisions from https://github.com/google/iconvg/issues/22
        matrix[0] = 0.0;
        matrix[1] = 0.0;
        matrix[4] = 0.0;
        matrix[5] = 0.0;
      }
      return Gradient.radial(
        Offset.zero,
        1.0,
        colors,
        colorStops,
        tileMode,
        matrix.storage,
      );
    }
    double x1, y1, dx, dy;
    if (a != 0) {
      x1 = -c / a;
      y1 = 0.0; // (arbitrarily)
      dx = a / (a * a + b * b);
      dy = b / (b * b + a * a);
    } else if (b != 0) {
      x1 = 0.0; // (arbitrarily)
      y1 = -c / b;
      dx = a / (a * a + b * b);
      dy = b / (b * b + a * a);
    } else {
      // TODO(ianh): apply decisions from https://github.com/google/iconvg/issues/22
      x1 = -1e9;
      y1 = -1e9;
      dx = 2e9;
      dy = 2e9;
    }
    return Gradient.linear(
      Offset(x1, y1),
      Offset(x1 + dx, y1 + dy),
      colors,
      colorStops,
      tileMode,
    );
  }

  static Uint32List byte1DecoderRing = Uint32List.fromList(<int>[ 0x00, 0x40, 0x80, 0xC0, 0xFF ]);

  int decodeColor1() {
    final int byte1 = nextByte;
    if (byte1 >= 192) {
      return CREG[(byte1 - 192) % kCREGLength];
    }
    if (byte1 >= 128) {
      return customPalette[(byte1 - 128) % kCREGLength];
    }
    switch (byte1) {
      case 127: return 0x00000000;
      case 126: return 0x80808080;
      case 125: return 0xC0C0C0C0;
    }
    final int blue = byte1 % 5;
    final int remainder = ((byte1 - blue) ~/ 5);
    final int green = remainder % 5;
    final int red = ((remainder - green) ~/ 5) % 5;
    return (byte1DecoderRing[red] << 24)
         + (byte1DecoderRing[green] << 16)
         + (byte1DecoderRing[blue] << 8)
         + 0xFF;
  }

  int decodeColor2() {
    final int byte1 = nextByte;
    final int byte2 = nextByte;
    return (byte1 & 0xF0) * 0x1100000
         + (byte1 & 0x0F) * 0x110000
         + (byte2 & 0xF0) * 0x110
         + (byte2 & 0x0F) * 0x11;
  }

  int decodeDirectColor3() {
    return (nextByte << 24) + (nextByte << 16) + (nextByte << 8) + 0xFF;
  }

  int decodeIndirectColor3() {
    final int byte1 = nextByte;
    final int C0 = decodeColor1();
    final int C1 = decodeColor1();
    final int red =   (((255 - byte1) * ((C0 & 0xFF000000) >> 24)) + (byte1 * ((C1 & 0xFF000000) >> 24)) + 128) ~/ 255;
    final int green = (((255 - byte1) * ((C0 & 0x00FF0000) >> 16)) + (byte1 * ((C1 & 0x00FF0000) >> 16)) + 128) ~/ 255;
    final int blue =  (((255 - byte1) * ((C0 & 0x0000FF00) >> 8))  + (byte1 * ((C1 & 0x0000FF00) >> 8))  + 128) ~/ 255;
    final int alpha = (((255 - byte1) *  (C0 & 0x000000FF))        + (byte1 *  (C1 & 0x000000FF))        + 128) ~/ 255;
    return (red << 24) + (green << 16) + (blue << 8) + alpha;
  }

  int decodeColor4() {
    return (nextByte << 24) + (nextByte << 16) + (nextByte << 8) + nextByte;
  }

  int decodeNaturalNumber() {
    final int byte1 = nextByte;
    if (byte1 & 0x01 == 0x00) {
      // 1 byte encoding
      return byte1 >> 1;
    }
    final int byte2 = nextByte;
    if (byte1 & 0x02 == 0x00) {
      // 2 byte encoding
      return (byte1 >> 2) + (byte2 << 6);
    }
    // 4 byte encoding
    final int byte3 = nextByte;
    final int byte4 = nextByte;
    return (byte1 >> 2) + (byte2 << 6) + (byte3 << 14) + (byte4 << 22);
  }

  static final ByteData _buffer = ByteData(4);

  double decodeRealNumber() {
    final int byte1 = nextByte;
    if (byte1 & 0x01 == 0x00) {
      // 1 byte encoding (same as decodeNaturalNumber)
      return (byte1 >> 1).toDouble();
    }
    final int byte2 = nextByte;
    if (byte1 & 0x02 == 0x00) {
      // 2 byte encoding (same as decodeNaturalNumber)
      return ((byte1 >> 2) + (byte2 << 6)).toDouble();
    }
    // 4 byte encoding (decodeNaturalNumber << 2, cast to float)
    final int byte3 = nextByte;
    final int byte4 = nextByte;
    _buffer.setUint32(0, (byte1 & 0xFC) + (byte2 << 8) + (byte3 << 16) + (byte4 << 24));
    return _buffer.getFloat32(0);
  }

  double decodeCoordinateNumber() {
    final int byte1 = nextByte;
    if (byte1 & 0x01 == 0x00) {
      // 1 byte encoding (decodeRealNumber with a bias)
      return (byte1 >> 1).toDouble() - 64.0;
    }
    final int byte2 = nextByte;
    if (byte1 & 0x02 == 0x00) {
      // 2 byte encoding (decodeRealNumber with a scale and a bias)
      return ((byte1 >> 2) + (byte2 << 6)).toDouble() / 64.0 - 128.0;
    }
    // 4 byte encoding (same as decodeRealNumber)
    final int byte3 = nextByte;
    final int byte4 = nextByte;
    _buffer.setUint32(0, (byte1 & 0xFC) + (byte2 << 8) + (byte3 << 16) + (byte4 << 24));
    return _buffer.getFloat32(0);
  }

  double decodeZeroToOneNumber() {
    final int byte1 = nextByte;
    if (byte1 & 0x01 == 0x00) {
      // 1 byte encoding (decodeRealNumber with a scale)
    return (byte1 >> 1).toDouble() / 120.0;
    }
    final int byte2 = nextByte;
    if (byte1 & 0x02 == 0x00) {
      // 2 byte encoding (decodeRealNumber with a scale)
      return ((byte1 >> 2) + (byte2 << 6)).toDouble() / 15120.0;
    }
    // 4 byte encoding (same as decodeRealNumber)
    final int byte3 = nextByte;
    final int byte4 = nextByte;
    _buffer.setUint32(0, (byte1 & 0xFC) + (byte2 << 8) + (byte3 << 16) + (byte4 << 24));
    return _buffer.getFloat32(0);
  }

  void checkSignature() {
    if (nextByte != 0x89 || nextByte != 0x49 || nextByte != 0x56 || nextByte != 0x47) {
      throw const FormatException('Signature did not match IconVG signature.');
    }
  }

  void applyCustomPalette(List<Color> palette) {
    assert(!foundPalette);
    int index = 0;
    for (final Color color in palette) {
      int value = _IconVGMachine.ColorToRGBA(color);
      if (isNonColor(value)) {
        value = 0x000000FF;
      }
      customPalette[index] = value;
      index += 1;
    }
    foundPalette = true;
  }

  void readMetadata() {
    final int count = decodeNaturalNumber();
    int lastMID = -1;
    for (int index = 0; index < count; index += 1) {
      final int blockLength = decodeNaturalNumber();
      final int blockEnd = cursor + blockLength;
      final int MID = decodeNaturalNumber();
      assert(MID >= 0);
      if (MID < lastMID) {
        throw FormatException('Metadata blocks out of order ($MID followed $lastMID).');
      }
      if (MID == lastMID) {
        throw FormatException('Duplicate metadata block with ID $MID.');
      }
      lastMID = MID;
      switch (MID) {
        case 0: // ViewBox
          assert(!foundViewBox);
          minX = decodeCoordinateNumber();
          minY = decodeCoordinateNumber();
          maxX = decodeCoordinateNumber();
          maxY = decodeCoordinateNumber();
          if (!minX.isFinite) {
            throw FormatException('ViewBox minX must be finite, not $minX.');
          }
          if (!minY.isFinite) {
            throw FormatException('ViewBox minY must be finite, not $minY.');
          }
          if (!maxX.isFinite) {
            throw FormatException('ViewBox maxX must be finite, not $maxX.');
          }
          if (!maxY.isFinite) {
            throw FormatException('ViewBox maxY must be finite, not $maxY.');
          }
          if (minX > maxX) {
            throw FormatException('ViewBox minX ($minX) must not be bigger than maxX ($maxX).');
          }
          if (minY > maxY) {
            throw FormatException('ViewBox minY ($minY) must not be bigger than maxY ($maxY).');
          }
          foundViewBox = true;
          break;
        case 1: // Suggested Palette
          // TODO(ianh): apply https://github.com/google/iconvg/issues/11 if we can skip this section (if we have a palette already)
          // TODO(ianh): apply https://github.com/google/iconvg/issues/14 ("at least one byte" wording)
          final int atLeastOneByte = nextByte;
          final int N = atLeastOneByte & 0x3F; // low six bits
          Iterable<int> newPalette; // we can't manipulate customPalette inplace because decoding the colors could read customPalette
          switch (N & 0xC0) {
            case 0x00: // 1 byte colors
              newPalette = List<int>.generate(N + 1, (int index) => decodeColor1());
              break;
            case 0x40: // 2 byte colors
              newPalette = List<int>.generate(N + 1, (int index) => decodeColor2());
              break;
            case 0x80: // 3 byte (direct) colors
              newPalette = List<int>.generate(N + 1, (int index) => decodeDirectColor3());
              break;
            case 0xC0: // 4 byte colors
              newPalette = List<int>.generate(N + 1, (int index) => decodeColor4());
              break;
            default:
              throw StateError('unreachable');
          }
          if (!foundPalette) {
            customPalette.setRange(0, newPalette.length, newPalette);
            foundPalette = true;
          }
          break;
      }
      // TODO(ianh): apply the decisions from https://github.com/google/iconvg/issues/11 (whether cursor can go past blockEnd)
      cursor = blockEnd;
    }
  }

  void execute() {
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder, viewBox); // TODO(ianh): apply https://github.com/google/iconvg/issues/15 (viewBox culling)
    switch (clipBehavior) {
      case Clip.none:
        break;
      case Clip.hardEdge:
        canvas.clipRect(viewBox, doAntiAlias: false);
        break;
      case Clip.antiAlias:
        canvas.clipRect(viewBox);
        break;
      case Clip.antiAliasWithSaveLayer:
        canvas.clipRect(viewBox);
        canvas.saveLayer(viewBox, Paint());
        break;
    }
    _RenderingMode mode = _RenderingMode.styling;
    Path? path;
    Paint? paint;
    late double x, y, cx, cy; // x,y is the current point; cx,cy is the old control point.
    _DrawingCommand lastOpcode = _DrawingCommand.other;
    while (cursor < bytes.length) {
      final int opcode = nextByte;
      switch (mode) {
        case _RenderingMode.styling:
          if (opcode <= 0x3F) {
            CSEL = opcode & 0x3F;

          } else if (opcode <= 0x7F) {
            NSEL = opcode & 0x3F;

          } else if (opcode <= 0x86) {
            CREG[(CSEL - (opcode & 0x07)) % kCREGLength] = decodeColor1();

          } else if (opcode <= 0x87) {
            CREG[CSEL % kCREGLength] = decodeColor1();
            CSEL += 1;

          } else if (opcode <= 0x8E) {
            CREG[(CSEL - (opcode & 0x07)) % kCREGLength] = decodeColor2();

          } else if (opcode <= 0x8F) {
            CREG[CSEL % kCREGLength] = decodeColor2();
            CSEL += 1;

          } else if (opcode <= 0x96) {
            CREG[(CSEL - (opcode & 0x07)) % kCREGLength] = decodeDirectColor3();

          } else if (opcode <= 0x97) {
            CREG[CSEL % kCREGLength] = decodeDirectColor3();
            CSEL += 1;

          } else if (opcode <= 0x9E) {
            CREG[(CSEL - (opcode & 0x07)) % kCREGLength] = decodeColor4();

          } else if (opcode <= 0x9F) {
            CREG[CSEL % kCREGLength] = decodeColor4();
            CSEL += 1;

          } else if (opcode <= 0xA6) {
            CREG[(CSEL - (opcode & 0x07)) % kCREGLength] = decodeIndirectColor3();

          } else if (opcode <= 0xA7) {
            CREG[CSEL % kCREGLength] = decodeIndirectColor3();
            CSEL += 1;

          } else if (opcode <= 0xAE) {
            NREG[(NSEL - (opcode & 0x07)) % kCREGLength] = decodeRealNumber();

          } else if (opcode <= 0xAF) {
            NREG[NSEL % kNREGLength] = decodeRealNumber();
            NSEL += 1;

          } else if (opcode <= 0xB6) {
            NREG[(NSEL - (opcode & 0x07)) % kCREGLength] = decodeCoordinateNumber();

          } else if (opcode <= 0xB7) {
            NREG[NSEL % kNREGLength] = decodeCoordinateNumber();
            NSEL += 1;

          } else if (opcode <= 0xBE) {
            NREG[(NSEL - (opcode & 0x07)) % kCREGLength] = decodeZeroToOneNumber();

          } else if (opcode <= 0xBF) {
            NREG[NSEL % kNREGLength] = decodeZeroToOneNumber();
            NSEL += 1;

          } else if (opcode <= 0xC6) {
            mode = _RenderingMode.drawing;
            x = decodeCoordinateNumber();
            y = decodeCoordinateNumber();
            path = Path() // TODO(ianh): apply https://github.com/google/iconvg/issues/21 (fill type)
              ..moveTo(x, y);
            lastOpcode = _DrawingCommand.other;
            paint = Paint()
              ..style = PaintingStyle.fill;
            final int color = CREG[(CSEL - (opcode & 0x07)) % kCREGLength];
            if (isGradient(color)) {
              paint.shader = RGBAToGradient(color);
            } else {
              paint.color = RGBAToColor(color);
            }

          } else if (opcode <= 0xC7) {
            LOD0 = decodeRealNumber();
            LOD1 = decodeRealNumber();

          } else {
            throw FormatException('Unexpected reserved opcode $opcode.');
          }
          break;

        case _RenderingMode.drawing:
          path!; paint!;
          final int RC = (opcode <= 0x3F ? opcode & 0x1F : opcode & 0x0F) + 1;
          if (opcode <= 0x1F) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              x = decodeCoordinateNumber();
              y = decodeCoordinateNumber();
              path.lineTo(x, y);
            }
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0x3F) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              x += decodeCoordinateNumber();
              y += decodeCoordinateNumber();
              path.lineTo(x, y);
            }
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0x4F) {
            if (lastOpcode != _DrawingCommand.quadratic) {
              cx = x;
              cy = y;
            }
            for (int repeat = 0; repeat < RC; repeat += 1) {
              cx = 2.0 * x - cx;
              cy = 2.0 * y - cy;
              x = decodeCoordinateNumber();
              y = decodeCoordinateNumber();
              path.quadraticBezierTo(cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.quadratic;

          } else if (opcode <= 0x5F) {
            if (lastOpcode != _DrawingCommand.quadratic) {
              cx = x;
              cy = y;
            }
            for (int repeat = 0; repeat < RC; repeat += 1) {
              cx = 2.0 * x - cx;
              cy = 2.0 * y - cy;
              x += decodeCoordinateNumber();
              y += decodeCoordinateNumber();
              path.quadraticBezierTo(cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.quadratic;

          } else if (opcode <= 0x6F) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              cx = decodeCoordinateNumber();
              cy = decodeCoordinateNumber();
              x = decodeCoordinateNumber();
              y = decodeCoordinateNumber();
              path.quadraticBezierTo(cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.quadratic;

          } else if (opcode <= 0x7F) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              cx = x + decodeCoordinateNumber();
              cy = y + decodeCoordinateNumber();
              x += decodeCoordinateNumber();
              y += decodeCoordinateNumber();
              path.quadraticBezierTo(cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.quadratic;

          } else if (opcode <= 0x8F) {
            if (lastOpcode != _DrawingCommand.cubic) {
              cx = x;
              cy = y;
            }
            for (int repeat = 0; repeat < RC; repeat += 1) {
              final double cx1 = 2.0 * x - cx;
              final double cy1 = 2.0 * y - cy;
              cx = decodeCoordinateNumber();
              cy = decodeCoordinateNumber();
              x = decodeCoordinateNumber();
              y = decodeCoordinateNumber();
              path.cubicTo(cx1, cy1, cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.cubic;

          } else if (opcode <= 0x9F) {
            if (lastOpcode != _DrawingCommand.cubic) {
              cx = x;
              cy = y;
            }
            for (int repeat = 0; repeat < RC; repeat += 1) {
              final double cx1 = 2.0 * x - cx;
              final double cy1 = 2.0 * y - cy;
              cx = x + decodeCoordinateNumber();
              cy = y + decodeCoordinateNumber();
              x += decodeCoordinateNumber();
              y += decodeCoordinateNumber();
              path.cubicTo(cx1, cy1, cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.cubic;

          } else if (opcode <= 0xAF) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              final double cx1 = decodeCoordinateNumber();
              final double cy1 = decodeCoordinateNumber();
              cx = decodeCoordinateNumber();
              cy = decodeCoordinateNumber();
              x = decodeCoordinateNumber();
              y = decodeCoordinateNumber();
              path.cubicTo(cx1, cy1, cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.cubic;

          } else if (opcode <= 0xBF) {
            for (int repeat = 0; repeat < RC; repeat += 1) {
              final double cx1 = x + decodeCoordinateNumber();
              final double cy1 = y + decodeCoordinateNumber();
              cx = x + decodeCoordinateNumber();
              cy = y + decodeCoordinateNumber();
              x += decodeCoordinateNumber();
              y += decodeCoordinateNumber();
              path.cubicTo(cx1, cy1, cx, cy, x, y);
            }
            lastOpcode = _DrawingCommand.cubic;

          } else if (opcode <= 0xDF) {
            final bool absolute = opcode <= 0xCF;
            for (int repeat = 0; repeat < RC; repeat += 1) {
              final double rx = decodeCoordinateNumber();
              final double ry = decodeCoordinateNumber();
              final Radius radius = Radius.elliptical(rx, ry);
              final double angle = decodeZeroToOneNumber();
              final double rotation = angle * 360;
              final int flags = decodeNaturalNumber();
              final bool largeArc = flags & 0x01 > 0;
              final bool clockwise = flags & 0x02 > 0;
              if (absolute) {
                x = decodeCoordinateNumber();
                y = decodeCoordinateNumber();
              } else {
                x += decodeCoordinateNumber();
                y += decodeCoordinateNumber();
              }
              final Offset arcEnd = Offset(x, y);
              path.arcToPoint(arcEnd, radius: radius, rotation: rotation, largeArc: largeArc, clockwise: clockwise);
            }
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE0) {
            throw FormatException('Unexpected reserved opcode $opcode.');

          } else if (opcode <= 0xE1) {
            path.close();
            lastOpcode = _DrawingCommand.other;
            if (LOD0 <= H && H < LOD1) {
              canvas.drawPath(path, paint);
            }
            mode = _RenderingMode.styling;
            path = null;
            paint = null;

          } else if (opcode <= 0xE2) {
            path.close();
            x = decodeCoordinateNumber();
            y = decodeCoordinateNumber();
            path.moveTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE3) {
            path.close();
            x += decodeCoordinateNumber();
            y += decodeCoordinateNumber();
            path.moveTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE5) {
            throw FormatException('Unexpected reserved opcode $opcode.');

          } else if (opcode <= 0xE6) {
            x = decodeCoordinateNumber();
            path.lineTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE7) {
            x += decodeCoordinateNumber();
            path.lineTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE8) {
            y = decodeCoordinateNumber();
            path.lineTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else if (opcode <= 0xE9) {
            y += decodeCoordinateNumber();
            path.lineTo(x, y);
            lastOpcode = _DrawingCommand.other;

          } else {
            throw FormatException('Unexpected reserved opcode $opcode.');
          }
          break;
      }
    }
    picture = recorder.endRecording();
  }
}
