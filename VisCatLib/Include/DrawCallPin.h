#pragma once

#include "Enums.h"

struct DrawCallPin
{
	int TriangleOffsetStart;
	int TriangleOffsetEnd;

	int VertexOffsetStart;
	int VertexOffsetEnd;

	VISCAT_CullMode CullMode;

	inline void Set(int triangleOffsetStart, int triangleCount, int vertexOffsetStart, int vertexCount, VISCAT_CullMode cullMode)
	{
		TriangleOffsetStart=triangleOffsetStart;
		TriangleOffsetEnd=TriangleOffsetStart+triangleCount;

		VertexOffsetStart=vertexOffsetStart;
		VertexOffsetEnd=VertexOffsetStart+vertexCount;
		CullMode = cullMode;
	}
};
