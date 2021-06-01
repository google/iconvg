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

import 'dart:ui' show Picture, Rect;

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import 'decoder.dart';

/// Render the specified IconVG [asset] as an icon.
///
/// This honors the ambient [IconTheme].
///
/// The [IconThemeData.color] is used as a one-color custom palette.
class IconVG extends StatelessWidget {
  const IconVG(this.asset, {
    Key? key,
    this.semanticLabel,
  }) : super(key: key);

  /// The name of the asset to render.
  ///
  /// The ambient [DefaultAssetBundle] is used to resolve this name.
  final String asset;

  /// Semantic label for the icon.
  ///
  /// Announced in accessibility modes (e.g TalkBack/VoiceOver).
  /// This label does not show in the UI.
  ///
  ///  * [SemanticsProperties.label], which is set to [semanticLabel] in the
  ///    underlying	 [Semantics] widget.
  final String? semanticLabel;

  @override
  Widget build(BuildContext context) {
    final IconThemeData theme = IconTheme.of(context);
    return Opacity(
      opacity: theme.opacity!,
      child: IconVGImage(
        asset,
        palette: <Color>[ theme.color! ],
        size: Size.square(theme.size!),
        semanticLabel: semanticLabel,
      ),
    );
  }
}

/// Render the specified IconVG [asset].
///
/// If the image is not available in the application's assets, consider fetching
/// the bytes manually and using a [RawIconVGImage].
class IconVGImage extends StatefulWidget {
  const IconVGImage(this.asset, {
    Key? key,
    this.palette,
    this.size,
    this.clipBehavior = Clip.hardEdge,
    this.semanticLabel,
  }) : super(key: key);

  /// The name of the asset to render.
  ///
  /// The ambient [DefaultAssetBundle] is used to resolve this name.
  final String asset;

  /// The custom color palette used to decode the image.
  ///
  /// Must not have more than 64 colors.
  final List<Color>? palette;

  /// The target render size. Defaults to the [BoxConstraints.maxHeight].
  final Size? size;

  /// How to clip the image.
  ///
  /// This can be used to enable anti-aliased clips if the image is rotated, or
  /// disable clips entirely (for improved performance) if the image does not
  /// need them.
  final Clip clipBehavior;

  /// A Semantic description of the image.
  ///
  /// Used to provide a description of the image to TalkBack on Android, and
  /// VoiceOver on iOS.
  final String? semanticLabel;

  @override
  State<IconVGImage> createState() => _IconVGImageState();
}

class _IconVGImageState extends State<IconVGImage> {
  AssetBundle? _assetBundle;
  Uint8List? _bytes;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    final AssetBundle assetBundle = DefaultAssetBundle.of(context);
    if (assetBundle != _assetBundle) {
      _assetBundle = assetBundle;
      _bytes = null;
      _fetchBytes();
    }
  }

  @override
  void didUpdateWidget(IconVGImage oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.asset != widget.asset) {
      _bytes = null;
      _fetchBytes();
    }
  }

  Future<void> _fetchBytes() async {
    assert(_assetBundle != null);
    final ByteData data = await _assetBundle!.load(widget.asset);
    if (mounted) {
      setState(() {
        _bytes = data.buffer.asUint8List();
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return RawIconVGImage(
      bytes: _bytes,
      palette: widget.palette,
      size: widget.size,
      clipBehavior: widget.clipBehavior,
      semanticLabel: widget.semanticLabel,
    );
  }
}

/// Render the specified [bytes] as an IconVG image.
///
/// If the image is an asset, consider using [IconVGImage] instead.
class RawIconVGImage extends StatefulWidget {
  const RawIconVGImage({
    Key? key,
    this.bytes,
    this.palette,
    this.size,
    this.clipBehavior = Clip.hardEdge,
    this.semanticLabel,
  }) : super(key: key);

  /// The bytes. Must be in the IconVG format.
  final Uint8List? bytes;

  /// The custom color palette used to decode the image.
  ///
  /// Must not have more than 64 colors.
  final List<Color>? palette;

  /// The target render size. Defaults to the [BoxConstraints.maxHeight].
  final Size? size;

  /// How to clip the image.
  ///
  /// This can be used to enable anti-aliased clips if the image is rotated, or
  /// disable clips entirely (for improved performance) if the image does not
  /// need them.
  final Clip clipBehavior;

  /// A Semantic description of the image.
  ///
  /// Used to provide a description of the image to TalkBack on Android, and
  /// VoiceOver on iOS.
  final String? semanticLabel;

  @override
  State<RawIconVGImage> createState() => _RawIconVGImageState();
}

class _RawIconVGImageState extends State<RawIconVGImage> {
  IconVGFile? _image;
  double? _decodedHeight;

  @override
  void didUpdateWidget(RawIconVGImage oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.bytes != widget.bytes ||
        oldWidget.palette != widget.palette ||
        oldWidget.clipBehavior != widget.clipBehavior) {
        // widget.size is handled in build
      _image = null;
    }
  }

  @override
  Widget build(BuildContext context) {
    if (widget.bytes == null) {
      return SizedBox.fromSize(size: widget.size ?? Size.zero);
    }
    return Semantics(
      container: widget.semanticLabel != null,
      image: true,
      label: widget.semanticLabel ?? '',
      child: LayoutBuilder(
        builder: (BuildContext context, BoxConstraints constraints) {
          final double height = widget.size?.height ?? constraints.maxHeight;
          if (_image == null || _decodedHeight != height) {
            try {
              _image = IconVGFile(
                widget.bytes!,
                palette: widget.palette,
                height: height,
                clipBehavior: widget.clipBehavior,
              );
              _decodedHeight = height;
            } catch (error, stack) {
              FlutterError.reportError(FlutterErrorDetails(
                exception: error,
                stack: stack,
                library: 'iconvg',
                silent: true,
              ));
              Widget? message;
              assert(() {
                message = Text('$error', style: const TextStyle(fontSize: 14.0, color: Color(0xFFFF0000), inherit: false));
                return true;
              }());
              return SizedBox.fromSize(
                size: widget.size ?? Size.zero,
                child: message, // only shows in debug builds
              );
            }
          }
          final Widget result = PicturePainter(
            picture: _image!.picture,
            rect: _image!.rect,
          );
          if (widget.size != null) {
            return SizedBox.fromSize(
              size: widget.size,
              child: result,
            );
          }
          return AspectRatio(
            aspectRatio: _image!.rect.size.aspectRatio,
            child: result,
          );
        },
      ),
    );
  }
}

class _PicturePainter extends CustomPainter {
  const _PicturePainter({
    required this.picture,
    required this.rect,
  });

  final Picture picture;
  final Rect rect;

  @override
  void paint(Canvas canvas, Size size) {
    canvas.save();
    canvas.scale(size.width / rect.width, size.height / rect.height);
    canvas.translate(-rect.left, -rect.top);
    canvas.drawPicture(picture);
    canvas.restore();
  }

  @override
  bool shouldRepaint(_PicturePainter oldPainter) {
    return oldPainter.picture != picture
        || oldPainter.rect != rect;
  }
}

/// Render a [Picture] in the widget tree.
class PicturePainter extends StatelessWidget {
  const PicturePainter({
    Key? key,
    required this.picture,
    required this.rect,
  }) : super(key: key);

  /// The [Picture] to paint. This is added to the current layer.
  final Picture picture;

  /// The segment of the [Picture] to paint. No clip is applied, so parts
  /// outside of this rect will still be visible outside the bounds of the
  /// widget.
  final Rect rect;

  @override
  Widget build(BuildContext context) {
    return SizedBox.fromSize(
      size: rect.size,
      child: CustomPaint(
        painter: _PicturePainter(
          picture: picture,
          rect: rect,
        ),
      ),
    );
  }
}
