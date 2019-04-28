#pragma once

struct Point
{
	short x, y;

	inline Point(){}
	inline Point(short _x, short _y) {x=_x; y=_y;}
	inline int ToIndex(int width) {return x+y*width;}

	static inline Point FromIndex(int index, short width)
	{
		return Point((short)index%width, (short)index/width);
	}

	inline static Point Clip(Point point, Point start, Point end)
	{
		return Point(max(start.x, min(point.x, end.x)),
					 max(start.y, min(point.y, end.y)));
	}
};