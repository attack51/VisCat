#pragma once

#include "dllexport.h"
#include <pmmintrin.h>

enum AllSide
{
	Mixed,
	AllInside,
	AllOutside,
};

class _MM_ALIGN16 VISCAT_LIB_API Side4
{
private:
	int allInside;
	int allOutside;
	int side;
	int dummy;	//for 16byte align
	//__m128 side;

public:
	inline Side4() {}
	Side4(const __m128& frac);
	//inline bool IsInside(int index) { return side.m128_i32[index]==0xffffffff; }
	inline bool IsInside(int index) { return ((side >> index) & 0x01)==0x01;}

	AllSide GetAllSide(int count);
	static AllSide CombinationAllSide(Side4* side4Array, int count);
};