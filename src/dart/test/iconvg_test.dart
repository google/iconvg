import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:flutter/material.dart';

import 'package:iconvg/decoder.dart';
import 'package:iconvg/widgets.dart';

void main() {
  testWidgets('built-in tests', (WidgetTester tester) async {
    IconVGFile.tests();
  });

  testWidgets('simple invalid files', (WidgetTester tester) async {
    expect(() => IconVGFile(Uint8List.fromList(<int>[])), throwsA(isA<FormatException>()));
    expect(() => IconVGFile(Uint8List.fromList(<int>[0x00])), throwsA(isA<FormatException>()));
    expect(() => IconVGFile(Uint8List.fromList(<int>[0x89, 0x49, 0x56])), throwsA(isA<FormatException>()));
    expect(() => IconVGFile(Uint8List.fromList(<int>[0x89, 0x49, 0x56, 0x46, 0x00])), throwsA(isA<FormatException>()));
  });

  testWidgets('blank.ivg', (WidgetTester tester) async {
    IconVGFile(Uint8List.fromList(<int>[0x89, 0x49, 0x56, 0x47, 0x00]));
  });

  final Uint8List actionInfoHiResBytes = File('../../test/data/action-info.hires.ivg').readAsBytesSync();
  final Uint8List actionInfoLoResBytes = File('../../test/data/action-info.lores.ivg').readAsBytesSync();
  final Uint8List arcsBytes = File('../../test/data/arcs.ivg').readAsBytesSync();
  final Uint8List cowbellBytes = File('../../test/data/cowbell.ivg').readAsBytesSync();
  final Uint8List ellipticalBytes = File('../../test/data/elliptical.ivg').readAsBytesSync();
  final Uint8List faviconBytes = File('../../test/data/favicon.ivg').readAsBytesSync();
  final Uint8List gradientBytes = File('../../test/data/gradient.ivg').readAsBytesSync();
  final Uint8List lodPolygonBytes = File('../../test/data/lod-polygon.ivg').readAsBytesSync();
  final Uint8List video005PrimitiveBytes = File('../../test/data/video-005.primitive.ivg').readAsBytesSync();

  testWidgets('action-info.hires.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoHiResBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.hires.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoHiResBytes, palette: const <Color>[Colors.teal]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.hires.teal.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoHiResBytes, palette: const <Color>[Colors.orange]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.hires.orange.png'));
  });

  testWidgets('action-info.lores.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoLoResBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.lores.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoLoResBytes, palette: const <Color>[Colors.teal]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.lores.teal.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: actionInfoLoResBytes, palette: const <Color>[Colors.orange]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/action-info.lores.orange.png'));
  });

  testWidgets('arcs.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: arcsBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/arcs.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: arcsBytes, size: const Size(512.0, 512.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/arcs.512x512.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: arcsBytes, size: const Size(1024.0, 128.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/arcs.1024x128.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: arcsBytes, size: const Size(128.0, 1024.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/arcs.128x1024.png'));
  });

  testWidgets('cowbell.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: cowbellBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/cowbell.default.png'));
  });

  testWidgets('elliptical.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: ellipticalBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/elliptical.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: ellipticalBytes))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/elliptical.rotated.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: ellipticalBytes, clipBehavior: Clip.none))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/elliptical.rotated.noclip.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: ellipticalBytes, clipBehavior: Clip.antiAlias))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/elliptical.rotated.aa.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: ellipticalBytes, clipBehavior: Clip.antiAliasWithSaveLayer))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/elliptical.rotated.aaWithSaveLayer.png'));
  });

  testWidgets('favicon.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: faviconBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/favicon.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: faviconBytes, palette: const <Color>[Colors.teal]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/favicon.teal.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: faviconBytes, palette: const <Color>[Colors.pink]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/favicon.pink.png'));
  });

  testWidgets('gradient.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: gradientBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/gradient.default.png'));
  });

  testWidgets('lod-polygon.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(10.0, 10.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.10.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(75.0, 75.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.75.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(85.0, 85.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.85.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(200.0, 200.0)))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.200.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(75.0, 75.0), palette: <Color>[ Colors.green.shade500, Colors.green.shade100, Colors.green.shade900 ]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.75.green.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: lodPolygonBytes, size: const Size(85.0, 85.0), palette: <Color>[ Colors.green.shade500, Colors.green.shade100, Colors.green.shade900 ]))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/lod-polygon.85.green.png'));
  });

  testWidgets('video005Primitive.ivg', (WidgetTester tester) async {
    await tester.pumpWidget(Center(child: RepaintBoundary(child: RawIconVGImage(bytes: video005PrimitiveBytes))));
    await expectLater(find.byType(RawIconVGImage), matchesGoldenFile('goldens/video-005-primitive.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: video005PrimitiveBytes))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/video-005-primitive.rotated.default.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: video005PrimitiveBytes, clipBehavior: Clip.none))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/video-005-primitive.rotated.noclip.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: video005PrimitiveBytes, clipBehavior: Clip.antiAlias))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/video-005-primitive.rotated.aa.png'));
    await tester.pumpWidget(Center(child: RepaintBoundary(child: Transform.rotate(angle: 1.0, child: Padding(padding: const EdgeInsets.all(96.0), child: RawIconVGImage(bytes: video005PrimitiveBytes, clipBehavior: Clip.antiAliasWithSaveLayer))))));
    await expectLater(find.byType(RepaintBoundary), matchesGoldenFile('goldens/video-005-primitive.rotated.aaWithSaveLayer.png'));
  });
}
