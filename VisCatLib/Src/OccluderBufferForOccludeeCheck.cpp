#include "stdafx.h"
#include "OccluderBuffer.h"

#include "MinMax.h"
#include "Float3.h"
#include "VisCatMath.h"
#include "sse_preset.h"
#include "Tile.h"

#include <math.h>

//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <ppl.h>

#define OCCLUDEE_VERTEX_MAX_COUNT 8
#define OCCLUDEE_CLIPPED_VERTEX_MAX_COUNT 24

/// point순서 
/// z         z
/// | 7  6    | 3  2 
/// | 4  5    | 0  1
/// ---x      ---x
///  아랫쪽   윗쪽
/// 
bool COccluderBuffer::BoxIsVisible(const Float3* boxPoints)
{
	Float4MM occludeeVertices[OCCLUDEE_VERTEX_MAX_COUNT];	
	MatrixMM::TransformPointArray(&m_vpMatrix, boxPoints, occludeeVertices, 8);

	Float4MM ndcMin, ndcMax;
	if(false==GetNdcBoxMinMax(&ndcMin, &ndcMax, occludeeVertices)) return false;	
	return IsVisibleCascadeCheck(ndcMin, ndcMax);
}


//return value : is inside of near plane
bool COccluderBuffer::GetNdcBoxMinMax(Float4MM* pNdcMin, Float4MM* pNdcMax, Float4MM* pOccludeeVertices)
{	
	__m128 z0123, z4567;
	Float4MM::ExtractZ4(pOccludeeVertices+0, &z0123);
	Float4MM::ExtractZ4(pOccludeeVertices+4, &z4567);

	int vert0123Reject = _mm_movemask_ps(_mm_cmplt_ps(z0123, g_zero4));
	int vert4567Reject = _mm_movemask_ps(_mm_cmplt_ps(z4567, g_zero4));
	int allOutsideCode = vert0123Reject & vert4567Reject;
	if(allOutsideCode==0x0f) return false;
	
	Float4MM occldeeClippedVertices[OCCLUDEE_CLIPPED_VERTEX_MAX_COUNT];
	int occldeeClippedVertexCount;

	int allInsideCode = vert0123Reject | vert4567Reject;
	if(allInsideCode==0)
	{
		occldeeClippedVertexCount=8;
		Float4MM::ClipToNdcArray(pOccludeeVertices, occldeeClippedVertices, 8);
	}
	else
	{
		Float4MM occludeeFraction4[2];
		Side4 occludeeSide4[2];

		Float4MM::GetPlaneSidesWithoutAllSide(	pOccludeeVertices,
												occludeeFraction4,
												occludeeSide4,
												8,
												Float4MM::GetNearPlaneSide4);
		
		occldeeClippedVertexCount = ClipOccludeeVerticesByNearPlane(pOccludeeVertices,
																	occludeeFraction4,
																	occludeeSide4,
																	occldeeClippedVertices);
	}

	assert(occldeeClippedVertexCount>=3);
	OccludeeNdcVerticesToNdcMinMax(	pNdcMin, 
									pNdcMax,
									occldeeClippedVertices,
									occldeeClippedVertexCount);



	__m128 size = _mm_sub_ps(pNdcMax->xyzw, pNdcMin->xyzw);
	float area = size.m128_f32[0] * size.m128_f32[1];
	assert(0 <= area);
	return 0 < area;
}

int COccluderBuffer::ClipOccludeeVerticesByNearPlane(Float4MM* pOccludeeVertices, 
													 Float4MM* pOccludeeFraction4,
				   									 Side4* pOccludeeSide4,
													 Float4MM* pOccldeeClippedVertices)
{
    int occldeeClippedVertexCount=0;

    //0 -> 1, 3, 4 와 연결
    //1 -> 0, 2, 5
    //2 -> 1, 3, 6
    //3 -> 0, 2, 7
    //4 -> 5, 7, 0
    //5 -> 4, 6, 1
    //6 -> 5, 7, 2
    //7 -> 4, 6, 3

	int neighborIndices[3];

    for (int i = 0; i < 8; ++i)
    {
        if (OccludeeVertIsInsideOfNearPlane(pOccludeeSide4, i))
        {
			pOccldeeClippedVertices[occldeeClippedVertexCount++] = pOccludeeVertices[i];
        }
        else
        {
            int bottomFlag = _ftoi(floorf(i/4.0f)); //0이면 top, 1이면 bottom
            int bottomStartIndex = bottomFlag*4; //top은 0, bottom은 4
            int roundIndex = i%4;

            neighborIndices[0] = (roundIndex + 1)%4 + bottomStartIndex;
			neighborIndices[1] = (roundIndex + 3)%4 + bottomStartIndex;
			neighborIndices[2] = roundIndex + ((bottomFlag + 1)%2)*4;

            Float4MM currentVert = pOccludeeVertices[i];
			float currentFrac=GetOccludeeVertFraction(pOccludeeFraction4, i);

            for (int neighborCount = 0; neighborCount < 3; ++neighborCount)
            {
                int neighborIndex = neighborIndices[neighborCount];
				if (OccludeeVertIsInsideOfNearPlane(pOccludeeSide4, neighborIndex))
                {
                    Float4MM neighborVert = pOccludeeVertices[neighborIndex];
					float neighborFrac=GetOccludeeVertFraction(pOccludeeFraction4, neighborIndex);
					float lerpFactor = GetLerpFactor(currentFrac,neighborFrac);

					Float4MM intersectPoint = Float4MM::Lerp(&currentVert, &neighborVert, lerpFactor);

                    pOccldeeClippedVertices[occldeeClippedVertexCount++] = intersectPoint;
                }
            }
        }
    }

	Float4MM::ClipToNdcArray(pOccldeeClippedVertices, pOccldeeClippedVertices, occldeeClippedVertexCount);
	return occldeeClippedVertexCount;
}

bool COccluderBuffer::OccludeeVertIsInsideOfNearPlane(Side4* pOccludeeSide4, int index)
{
	assert(index<8);

	int arrayIndex = index/4;
	int componentIndex = index%4;
	return pOccludeeSide4[arrayIndex].IsInside(componentIndex);
}

float COccluderBuffer::GetOccludeeVertFraction(Float4MM* pOccludeeFraction4, int index)
{
	assert(index<8);

	int arrayIndex = index/4;
	int componentIndex = index%4;
	return pOccludeeFraction4[arrayIndex].Get(componentIndex);	
}

void COccluderBuffer::OccludeeNdcVerticesToNdcMinMax(Float4MM* ndcMin, 
													 Float4MM* ndcMax,
													 Float4MM* pOccldeeClippedVertices,
													 int occldeeClippedVertexCount)
{
	__m128 minMM=_mm_set1_ps(100000);
	__m128 maxMM=_mm_set1_ps(-100000);

	for (int i = 0; i < occldeeClippedVertexCount; ++i)
    {
		minMM = _mm_min_ps(minMM, pOccldeeClippedVertices[i].xyzw);
		maxMM = _mm_max_ps(maxMM, pOccldeeClippedVertices[i].xyzw);
    }

	//안해도 별차이가 안난다
	//NdcMinMaxHalftShift(&minMM, &maxMM);

	minMM = _mm_max_ps(_mm_min_ps(minMM, g_one4), g_negOne4);
	maxMM = _mm_max_ps(_mm_min_ps(maxMM, g_one4), g_negOne4);

	*ndcMin=Float4MM(minMM);
	*ndcMax=Float4MM(maxMM);
}

void COccluderBuffer::NdcMinMaxHalftShift(__m128* pNdcMin, __m128* pNdcMax)
{
	*pNdcMin = _mm_add_ps(*pNdcMin, m_ndcSpaceHalfShift.xyzw);
	*pNdcMax = _mm_sub_ps(*pNdcMax, m_ndcSpaceHalfShift.xyzw);
}

void COccluderBuffer::GetBoxScreenStartEnd(	const Float4MM& min, 
											const Float4MM& max,
											Point& outStart,
											Point& outEnd,
											int level)
{
	//Xmin, yMax, Xmax, Ymin
	Float4MM ndcMinMaxToScreen = Float4MM::NdcMinMaxToScreenXY(	m_screenScale2[level],
																m_screenShift2[level],
																&min,
																&max);

	__m128 floorVal = _mm_floor_ps(ndcMinMaxToScreen.xyzw);
	__m128 ceilVal =  _mm_ceil_ps(ndcMinMaxToScreen.xyzw);
	__m128 xStartEnd = _mm_shuffle_ps(floorVal, ceilVal, _MM_SHUFFLE(2,2,0,0));
	__m128 yStartEnd = _mm_shuffle_ps(floorVal, ceilVal, _MM_SHUFFLE(1,1,3,3));

	__m128i xStartEndi = _mm_cvttps_epi32(xStartEnd);
	__m128i yStartEndi = _mm_cvttps_epi32(yStartEnd);

	assert(xStartEndi.m128i_i32[0] >=0);			
	assert(xStartEndi.m128i_i32[2] >=0);
	assert(xStartEndi.m128i_i32[0] <=GetWidth(0));
	assert(xStartEndi.m128i_i32[2] <=GetWidth(0));

	assert(yStartEndi.m128i_i32[0] >=0);			
	assert(yStartEndi.m128i_i32[2] >=0);
	assert(yStartEndi.m128i_i32[0] <=GetHeight(0));
	assert(yStartEndi.m128i_i32[2] <=GetHeight(0));

	outStart = Point((short)xStartEndi.m128i_i32[0], (short)yStartEndi.m128i_i32[0]);
	outEnd = Point((short)xStartEndi.m128i_i32[2], (short)yStartEndi.m128i_i32[2]);
}

bool COccluderBuffer::IsVisibleCascadeCheck(Float4MM& ndcMin, Float4MM& ndcMax)
{
	__m128 occludeeDepth4 = _mm_set1_ps(ndcMin.Z());
	for(int invLevel=m_depthLevelCount-1; invLevel>=0; --invLevel)
	{
		Point start, end;
		GetBoxScreenStartEnd(ndcMin, ndcMax, start, end, invLevel);

		int packXStart, startXRemain;
		int packXEnd, endXRemain;

		ConvertPackStartAndRemain(start.x, &packXStart, &startXRemain);
		ConvertPackEndAndRemain(end.x, &packXEnd, &endXRemain);	
		
		bool isOccludeedByMaxDepth = true;

		__m128 startMask4	= StartCompareMask4(startXRemain);
		__m128 endMask4		= EndCompareMask4(endXRemain);
		if(PackXIsEnd(packXStart, packXEnd)) startMask4 = _mm_and_ps(startMask4, endMask4);
		
		//캐쉬 효율을 높이기 위해 line부터 루프돈다
		for(int y=start.y;y<end.y; ++y)
		{
			for(int packX=packXStart; packX<packXEnd; packX+=4)
			{
				Point point((short)packX, (short)y);

				if(invLevel==0)
				{
					float* depthPointer = GetDepthPointer(point);
					__m128 depth4 = GetDepth4(depthPointer);
					__m128 compare4 = _mm_cmplt_ps(occludeeDepth4, depth4);

					if(PackXIsStart(packX, packXStart))
					{
						compare4 = _mm_and_ps(compare4, startMask4);
					}
					else if(PackXIsEnd(packX, packXEnd))
					{
						compare4 = _mm_and_ps(compare4, endMask4);
					}

					//한픽셀이라도 depth보다 작으면 보이는거.
					if(_mm_movemask_ps(compare4) > 0) return true;
				}
				else
				{
					float* pMinPointer;
					float* pMaxPointer;
					GetMinMaxDepthPointer(	point, 
											invLevel, 
											&pMinPointer,
											&pMaxPointer);

					__m128 minDepth4 = GetDepth4(pMinPointer);
					__m128 maxDepth4 = GetDepth4(pMaxPointer);
				
					__m128 minCompare4 = _mm_cmplt_ps(occludeeDepth4, minDepth4);
					__m128 maxCompare4 = _mm_cmplt_ps(occludeeDepth4, maxDepth4);

					if(PackXIsStart(packX, packXStart))
					{
						minCompare4 = _mm_and_ps(minCompare4, startMask4);
						maxCompare4 = _mm_and_ps(maxCompare4, startMask4);
					}
					else if(PackXIsEnd(packX, packXEnd))
					{
						minCompare4 = _mm_and_ps(minCompare4, endMask4);
						maxCompare4 = _mm_and_ps(maxCompare4, endMask4);
					}

					//한픽셀이라도 min보다 작으면 보이는거.
					if(_mm_movemask_ps(minCompare4) > 0) return true;

					//(level 0를 제외하고)모든 픽셀이 max보다 크면 아래level을 볼필요없이 안보이는 물체이다.
					//if (allpixel >= max) 은 if(onepixel < max) 이랑 같다.
					if(_mm_movemask_ps(maxCompare4) > 0) isOccludeedByMaxDepth=false;
				}
			}
		}

		if(isOccludeedByMaxDepth) return false;
	}
	return false;
}
