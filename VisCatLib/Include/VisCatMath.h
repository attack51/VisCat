#pragma once

#include "stdafx.h"
//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)

#define GetLerpFactor(start,end) start/(start-end)

#define _ftoi4(f4) _mm_cvtps_epi32(f4)
#define _ftoi(f) static_cast<int>(f)
//#define _ftoi(f) _mm_cvtt_ss2si(_mm_load_ss(&f));

//zero base
inline void GetSeperatedLoopCursors(int totalCount, int* loopCursors, int parallelCount)
{
	loopCursors[0] = 0;

	int dividedLoopCount = totalCount / parallelCount;
	for(int t=0; t<parallelCount; ++t)
	{
		loopCursors[t+1] = loopCursors[t] + dividedLoopCount;
		if(t < totalCount%parallelCount) loopCursors[t+1] += 1;
	}
}

inline void GetSeperatedLoopCursors(int start, int end, int* loopCursors, int parallelCount)
{
	int totalCount=end-start;
	loopCursors[0] = start;

	int dividedLoopCount = totalCount / parallelCount;
	for(int t=0; t<parallelCount; ++t)
	{
		loopCursors[t+1] = loopCursors[t] + dividedLoopCount;
		if(t < totalCount%parallelCount) loopCursors[t+1] += 1;
	}
}