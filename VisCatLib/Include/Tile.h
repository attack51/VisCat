#pragma once

#include "dllexport.h"
#include "Enums.h"
#include "Point.h"

class VISCAT_LIB_API Tile
{
private:
	//DO NOT ACCESS!!
	char m_frontDummyForCacheLine[32];

public:
	float m_buffer[TILE_BUFFER_SIZE];

	inline static Point ClipToTile(Point point) 
	{
		return Point(	max(0, min(point.x,TILE_BUFFER_WIDTH)),
						max(0, min(point.y,TILE_BUFFER_WIDTH)));
	}

private:
	//DO NOT ACCESS!!
	char m_backDummyForCacheLine[32];
};