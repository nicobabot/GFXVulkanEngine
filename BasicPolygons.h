#pragma once

class GfxGeometry;

// All primitive types use the same vertex layout and GPU representation.
void GenerateCubeGeometry(GfxGeometry& geometry);
void GeneratePlaneGeometry(GfxGeometry& geometry);
void GenerateSphereGeometry(GfxGeometry& geometry);
