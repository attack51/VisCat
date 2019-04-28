#pragma once

#include "dllexport.h"

class VISCAT_LIB_API Float3
{
public:
	float x, y, z;

	inline Float3() { }
	inline Float3(float _x, float _y, float _z) { x=_x; y=_y; z=_z; }

	static float Dot(const Float3* lhs, const Float3* rhs);
	static Float3 Cross(const Float3* lhs, const Float3* rhs);
	static float Length(const Float3* val);
	static Float3 Normalize(const Float3* val);
};