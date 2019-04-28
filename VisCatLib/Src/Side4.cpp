#include "stdafx.h"
#include "Side4.h"
#include "sse_preset.h"

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

#include <assert.h>

static int g_allSideSet[5] = {0, 1, 3, 7, 15};

//전체체크는 느슨하게, 자르기용 체크는 칼같이
Side4::Side4(const __m128& frac)
{
	allOutside = _mm_movemask_ps(_mm_cmpgt_ps(frac, g_negEpsilon4));
	allInside = _mm_movemask_ps(_mm_cmplt_ps(frac, g_epsilon4));
	side = _mm_movemask_ps(_mm_cmple_ps(frac, g_zero4));
	//side = _mm_cmple_ps(frac, g_zero4);
}

AllSide Side4::GetAllSide(int count)
{
	if((allOutside & g_allSideSet[count])==g_allSideSet[count]) 
	{
		return AllSide::AllOutside;
	}
	if((allInside & g_allSideSet[count])==g_allSideSet[count]) 
	{
		return AllSide::AllInside;
	}
	return AllSide::Mixed;
}

AllSide Side4::CombinationAllSide(Side4* side4Array, int count)
{
	if(count<=4) return side4Array[0].GetAllSide(count);
	
	AllSide firstAllSide = side4Array[0].GetAllSide(4);
	if(AllSide::Mixed==firstAllSide) return AllSide::Mixed;

	AllSide currentAllSide;
	int currentCount;

	for(int remainCount=count-4, pack4Index=1; remainCount>0; remainCount-=4, ++pack4Index)
	{
		if(remainCount>=4) currentCount=4;
		else currentCount=remainCount;

		currentAllSide=side4Array[pack4Index].GetAllSide(currentCount);
		if(firstAllSide != currentAllSide) return AllSide::Mixed;
	}
	
	return firstAllSide;
}