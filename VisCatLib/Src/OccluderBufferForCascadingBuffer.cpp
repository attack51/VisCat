#include "stdafx.h"
#include "OccluderBuffer.h"

#include "MinMax.h"
#include "Float3.h"
#include "VisCatMath.h"
#include "Tile.h"

#include <math.h>

//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <ppl.h>

void COccluderBuffer::CreateCascadingBuffer()
{
	for(int currentLevel=1; currentLevel<m_depthLevelCount; ++currentLevel)
	{
		__m128 min4, max4;

		int parentLevel=currentLevel-1;
		for(int y=0; y<GetHeight(currentLevel); ++y)
		{
			for(int x=0; x<GetWidth(currentLevel); x+=4)
			{
				Point screenPoint((short)x, (short)y);
				Point parentScreenPoint(screenPoint.x*2, screenPoint.y*2);

				if(0==parentLevel)
				{
					float* pParentDepth = GetDepthPointer(parentScreenPoint);
					__m128 depthLt, depthRt, depthLb, depthRb;
					GetNeighborDepth8x2(pParentDepth, 
										parentLevel, 
										&depthLt, 
										&depthRt, 
										&depthLb, 
										&depthRb);
					min4 = CombinatedMin4(depthLt, depthRt, depthLb, depthRb);
					max4 = CombinatedMax4(depthLt, depthRt, depthLb, depthRb);
				}
				else
				{
					float* pParentMinPointer;
					float* pParentMaxPointer;
					GetMinMaxDepthPointer(	parentScreenPoint, 
											parentLevel, 
											&pParentMinPointer, 
											&pParentMaxPointer);

					__m128 depthLt, depthRt, depthLb, depthRb;

					GetNeighborDepth8x2(pParentMinPointer, 
										parentLevel,
										&depthLt, 
										&depthRt, 
										&depthLb, 
										&depthRb);

					min4 = CombinatedMin4(depthLt, depthRt, depthLb, depthRb);

					GetNeighborDepth8x2(pParentMaxPointer, 
										parentLevel,
										&depthLt, 
										&depthRt, 
										&depthLb, 
										&depthRb);

					max4 = CombinatedMax4(depthLt, depthRt, depthLb, depthRb);
				}

				float* pCurrentMinPointer;
				float* pCurrentMaxPointer;
				GetMinMaxDepthPointer(	screenPoint, 
										currentLevel, 
										&pCurrentMinPointer,
										&pCurrentMaxPointer);

				_mm_store_ps(pCurrentMinPointer, min4);
				_mm_store_ps(pCurrentMaxPointer, max4);
			}
		}
	}
}

void COccluderBuffer::GetNeighborDepth8x2(float* pCurrent, 
										  int level,
										  __m128* depthLt, 
										  __m128* depthRt, 
										  __m128* depthLb, 
										  __m128* depthRb)
{
	*depthLt = GetDepth4(pCurrent + m_8x2NeighborOffset4[level].m128i_i32[0]);
	*depthRt = GetDepth4(pCurrent + m_8x2NeighborOffset4[level].m128i_i32[1]);
	*depthLb = GetDepth4(pCurrent + m_8x2NeighborOffset4[level].m128i_i32[2]);
	*depthRb = GetDepth4(pCurrent + m_8x2NeighborOffset4[level].m128i_i32[3]);
}

__m128 COccluderBuffer::CombinatedMin4(const __m128& parentLt, const __m128& parentRt, 
									   const __m128& parentLb, const __m128& parentRb)
{
	__m128 topMin = _mm_min_ps(_mm_shuffle_ps(parentLt, parentRt, _MM_SHUFFLE(2,0,2,0)),
							   _mm_shuffle_ps(parentLt, parentRt, _MM_SHUFFLE(3,1,3,1)));

	__m128 bottomMin = _mm_min_ps(_mm_shuffle_ps(parentLb, parentRb, _MM_SHUFFLE(2,0,2,0)),
							      _mm_shuffle_ps(parentLb, parentRb, _MM_SHUFFLE(3,1,3,1)));

	return _mm_min_ps(topMin, bottomMin);
}
	
__m128 COccluderBuffer::CombinatedMax4(const __m128& parentLt, const __m128& parentRt, 
									   const __m128& parentLb, const __m128& parentRb)
{
	__m128 topMax = _mm_max_ps(_mm_shuffle_ps(parentLt, parentRt, _MM_SHUFFLE(2,0,2,0)),
							   _mm_shuffle_ps(parentLt, parentRt, _MM_SHUFFLE(3,1,3,1)));

	__m128 bottomMax = _mm_max_ps(_mm_shuffle_ps(parentLb, parentRb, _MM_SHUFFLE(2,0,2,0)),
							      _mm_shuffle_ps(parentLb, parentRb, _MM_SHUFFLE(3,1,3,1)));

	return _mm_max_ps(topMax, bottomMax);
}
