#pragma once
#include "Float4MM.h"

#define BORDER_VERTEX_MAX_COUNT 12

struct ScanLineBorderVertices
{
	int EndY[BORDER_VERTEX_MAX_COUNT];
	Float4MM Verts[BORDER_VERTEX_MAX_COUNT];
	Float4MM Slopes[BORDER_VERTEX_MAX_COUNT];
	int BorderVertCount;
};

struct WindingSilhouette
{
	int YStart, YEnd;
	ScanLineBorderVertices LeftBorder;
	ScanLineBorderVertices RightBorder;
};