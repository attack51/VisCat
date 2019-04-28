#include "stdafx.h"
#include "OccluderBuffer.h"

#include "MinMax.h"
#include "Float3.h"
#include "VisCatMath.h"
#include "Tile.h"
#include "Winding.h"
#include "sse_preset.h"
#include "CohenSutherlandCode.h"

#include <math.h>

//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <ppl.h>
#include <ppltasks.h>

#include <Windows.h>

#define GetNextIndex(startIndex, increase, vertexCount) (startIndex+vertexCount+increase)%vertexCount

/////////////////////////////////////////////////////////////////////////////////////////////////////
//GetWindingTopBottomIndex 
void COccluderBuffer::GetWindingTopBottomIndex(Winding* pWinding, 
											   int& topIndex, 
											   int& bottomIndex)
{
	topIndex=0;
	bottomIndex=0;

	float minY=pWinding->ReadVertex(0)->Y();
	float maxY=minY;

	float currentY;
	int vertexCount=pWinding->VertexCount();
	for(int i=1; i<vertexCount; ++i)
	{
		currentY=pWinding->ReadVertex(i)->Y();
		if(currentY<minY)
		{
			topIndex=i;
			minY=currentY;
		}
		if(currentY>maxY)
		{
			bottomIndex=i;
			maxY=currentY;
		}
	}
}

#define DefGetWindingTopBottomIndex(pWinding, topIndex, bottomIndex)\
{\
	topIndex=0;\
	bottomIndex=0;\
	float minY=pWinding->ReadVertex(0)->Y();\
	float maxY=minY;\
	float currentY;\
	int vertexCount=pWinding->VertexCount();\
	for(int i=1; i<vertexCount; ++i)\
	{\
		currentY=pWinding->ReadVertex(i)->Y();\
		if(currentY<minY)\
		{\
			topIndex=i;\
			minY=currentY;\
		}\
		if(currentY>maxY)\
		{\
			bottomIndex=i;\
			maxY=currentY;\
		}\
	}\
}\

/////////////////////////////////////////////////////////////////////////////////////////////////////
//GetStartEndY 
bool COccluderBuffer::GetStartEndY(	Float4MM* topVertex, 
									Float4MM* bottomVertex, 
									float* pOutCeilTopY, 
									int* pOutStartY, 
									int* pOutEndY)
{
	*pOutCeilTopY = _float4CeilY(*topVertex);
	*pOutStartY = _ftoi(*pOutCeilTopY);
	*pOutEndY = _ftoi(_float4CeilY(*bottomVertex));

	return *pOutStartY != *pOutEndY;
}

bool COccluderBuffer::GetStartEndY_SSE(	Float4MM* topVertex, 
										Float4MM* bottomVertex, 
										float* pOutCeilTopY, 
										int* pOutStartY, 
										int* pOutEndY)
{
	__m128 topYbottomY = _mm_shuffle_ps(topVertex->xyzw, bottomVertex->xyzw, _MM_SHUFFLE(1,1,1,1));
	__m128 ceilY = _mm_ceil_ps(topYbottomY);
	__m128i startEndY = _ftoi4(ceilY);
	*pOutCeilTopY = ceilY.m128_f32[0];
	*pOutStartY = startEndY.m128i_i32[0];
	*pOutEndY = startEndY.m128i_i32[2];

	return *pOutStartY != *pOutEndY;
}

#define DefGetStartEndY_SSE(topVertex, bottomVertex, ceilTopY, startY, endY)\
{\
	__m128 topYbottomY = _mm_shuffle_ps(topVertex->xyzw, bottomVertex->xyzw, _MM_SHUFFLE(1,1,1,1));\
	__m128 ceilY = _mm_ceil_ps(topYbottomY);\
	__m128i startEndY = _ftoi4(ceilY);\
	ceilTopY = ceilY.m128_f32[0];\
	startY = startEndY.m128i_i32[0];\
	endY = startEndY.m128i_i32[2];\
	if(startY==endY) return;\
}\

/////////////////////////////////////////////////////////////////////////////////////////////////////
//GetLeftRightXAndSubPixelV 
bool COccluderBuffer::GetLeftRightXAndSubPixelV(Float4MM* pLeft, 
												Float4MM* pRight, 
												int* pOutLeftX, 
												int* pOutRightX,
												float* pSubPixelV)
{
	float ceilLeftX = _float4CeilX(*pLeft);

	*pOutLeftX = _ftoi(ceilLeftX);
	*pOutRightX = _ftoi(ceil(pRight->X()));

	if((*pOutLeftX)>=(*pOutRightX)) return false;

	*pSubPixelV = ceilLeftX-pLeft->X();
	return true;
}

bool COccluderBuffer::GetLeftRightXAndSubPixelV_SSE(Float4MM* pLeft, 
													Float4MM* pRight, 
													int* pOutLeftX, 
													int* pOutRightX,
													float* pSubPixelV)
{
	//left.xx, right.xx
	__m128 leftRightX = _mm_shuffle_ps(pLeft->xyzw, pRight->xyzw, _MM_SHUFFLE(0,0,0,0));
	__m128 ceilLeftRightX = _mm_ceil_ps(leftRightX);

	__m128i startEndX = _ftoi4(ceilLeftRightX);
	*pOutLeftX = startEndX.m128i_i32[0];
	*pOutRightX = startEndX.m128i_i32[2];

	if((*pOutLeftX)>=(*pOutRightX)) return false;

	*pSubPixelV = _mm_sub_ps(ceilLeftRightX, leftRightX).m128_f32[0];
	return true;
}

#define DefGetLeftRightXAndSubPixelV_SSE(left, right, leftX, rightX, subPixelV)\
{\
	/*left.xx, right.xx*/\
	__m128 leftRightX = _mm_shuffle_ps(left.xyzw, right.xyzw, _MM_SHUFFLE(0,0,0,0));\
	__m128 ceilLeftRightX = _mm_ceil_ps(leftRightX);\
	__m128i startEndX = _ftoi4(ceilLeftRightX);\
	leftX = startEndX.m128i_i32[0];\
	rightX = startEndX.m128i_i32[2];\
	if(leftX>=rightX) return;\
	subPixelV = _mm_sub_ps(ceilLeftRightX, leftRightX).m128_f32[0];\
}\

/////////////////////////////////////////////////////////////////////////////////////////////////////
//GetWindingLeftRightIncreaseAndSlope 
void COccluderBuffer::GetWindingLeftRightIncreaseAndSlope(	Winding* pWinding,
														  int topIndex,
														  int& leftIncIndex, 
														  int& rightIncIndex,
														  Float4MM& leftSlope, 
														  Float4MM& rightSlope)
{
	int cwIndex = GetNextIndex(topIndex, 1, pWinding->VertexCount());
	int ccwIndex = GetNextIndex(topIndex, -1, pWinding->VertexCount());

	Float4MM cwSlope = Float4MM::YbaseSlope(pWinding->ReadVertex(topIndex), 
		pWinding->ReadVertex(cwIndex));

	Float4MM ccwSlope = Float4MM::YbaseSlope(pWinding->ReadVertex(topIndex), 
		pWinding->ReadVertex(ccwIndex));

	if(cwSlope.X() < ccwSlope.X())
	{
		leftIncIndex = 1;
		rightIncIndex = -1;

		leftSlope = cwSlope;
		rightSlope = ccwSlope;
	}
	else
	{
		leftIncIndex = -1;
		rightIncIndex = 1;

		leftSlope = ccwSlope;
		rightSlope = cwSlope;
	}
}

#define DefGetWindingLeftRightIncreaseAndSlope(pWinding, topIndex, leftIncIndex, rightIncIndex,leftSlope, rightSlope)\
{\
	int cwIndex = GetNextIndex(topIndex, 1, pWinding->VertexCount());\
	int ccwIndex = GetNextIndex(topIndex, -1, pWinding->VertexCount());\
	Float4MM cwSlope = Float4MM::YbaseSlope(pWinding->ReadVertex(topIndex), pWinding->ReadVertex(cwIndex));\
	Float4MM ccwSlope = Float4MM::YbaseSlope(pWinding->ReadVertex(topIndex), pWinding->ReadVertex(ccwIndex));\
	if(cwSlope.X() < ccwSlope.X())\
	{\
		leftIncIndex = 1;\
		rightIncIndex = -1;\
		leftSlope = cwSlope;\
		rightSlope = ccwSlope;\
	}\
	else\
	{\
		leftIncIndex = -1;\
		rightIncIndex = 1;\
		leftSlope = ccwSlope;\
		rightSlope = cwSlope;\
	}\
}\

/////////////////////////////////////////////////////////////////////////////////////////////////////
//RevisePointWithSubPixel 
Float4MM COccluderBuffer::RevisePointWithSubPixel(const Float4MM* p, const float subPixel, const Float4MM* slope)
{
	Float4MM subPixelOffset = Float4MM::Mul(slope, subPixel);
	return Float4MM::Add(p, &subPixelOffset);
}

void COccluderBuffer::DrawWinding(Winding* pWinding, int parallelIndex)
{
	int topIndex, bottomIndex;
	DefGetWindingTopBottomIndex(pWinding, topIndex, bottomIndex);
	//GetWindingTopBottomIndex(pWinding, topIndex, bottomIndex);
	int leftIncIndex, rightIncIndex;

	Float4MM* topVertex		= pWinding->ReadVertex(topIndex);
	Float4MM* bottomVertex	= pWinding->ReadVertex(bottomIndex);
	assert(topVertex->Y()>-1 && bottomVertex->Y()<GetHeight(0)+1);

	float ceilTopY;
	int startY, endY;
	//if(false==GetStartEndY_SSE(topVertex, bottomVertex, &ceilTopY, &startY, &endY)) return;
	DefGetStartEndY_SSE(topVertex, bottomVertex, ceilTopY, startY, endY);
	assert(startY>=0 && startY<endY);

	Float4MM leftSlope, rightSlope;

	DefGetWindingLeftRightIncreaseAndSlope(	pWinding,
											topIndex, 
											leftIncIndex, 
											rightIncIndex,
											leftSlope, 
											rightSlope);

	//GetWindingLeftRightIncreaseAndSlope(pWinding,
	//									topIndex, 
	//									leftIncIndex, rightIncIndex,
	//									leftSlope, rightSlope);

	Float4MM oLeftSlope=leftSlope;
	Float4MM oRightSlope=rightSlope;
	if(leftSlope.X()==0 && rightSlope.X()==0) return;

	int leftIndex = topIndex;
	int rightIndex = topIndex;

	int nextLeftIndex = GetNextIndex(leftIndex, leftIncIndex, pWinding->VertexCount());
	int nextRightIndex = GetNextIndex(rightIndex, rightIncIndex, pWinding->VertexCount());

	//sub pixel 보정
	float subPixelH = ceilTopY - topVertex->Y();

	Float4MM leftP = *topVertex;
	Float4MM rightP = *topVertex;

	leftP = RevisePointWithSubPixel(&leftP, subPixelH, &leftSlope);
	rightP = RevisePointWithSubPixel(&rightP, subPixelH, &rightSlope);
	assert(leftP.X() <= rightP.X());

	for(int y=startY; y<endY; ++y)
	{
		if(UpdateNextIndex(pWinding, y, leftIncIndex, leftIndex, nextLeftIndex))
		{
			UpdateScanLineBorder(	pWinding,
									leftIndex, 
									nextLeftIndex, 
									leftSlope, 
									leftP);
		}

		if(UpdateNextIndex(pWinding, y, rightIncIndex, rightIndex, nextRightIndex))
		{
			UpdateScanLineBorder(	pWinding,
									rightIndex, 
									nextRightIndex, 
									rightSlope, 
									rightP);
		}

		ScanLineSseAlign(y, parallelIndex, leftP, rightP);

		leftP = Float4MM::Add(&leftP, &leftSlope);
		rightP = Float4MM::Add(&rightP, &rightSlope);
	}
}

bool COccluderBuffer::UpdateNextIndex(Winding* pWinding,
									  const int& y, 
									  const int& inc, 
									  int& curIndex, 
									  int& nextIndex)
{
	bool updated=false;
	float nextY = pWinding->ReadVertex(nextIndex)->Y();
	while(y>=nextY)
	{
		curIndex = nextIndex;
		nextIndex = GetNextIndex(nextIndex, inc, pWinding->VertexCount());
		nextY = pWinding->ReadVertex(nextIndex)->Y();

		updated=true;
	}

	return updated;
}

void COccluderBuffer::UpdateScanLineBorder(	Winding* pWinding,
										   const int& curIndex, 
										   const int& nextIndex, 
										   Float4MM& slope,
										   Float4MM& p)
{
	slope=Float4MM::YbaseSlope(pWinding->ReadVertex(curIndex), pWinding->ReadVertex(nextIndex));

	p = *pWinding->ReadVertex(curIndex);
	float subPixelH = _float4CeilY(p) - p.Y();

	p = RevisePointWithSubPixel(&p, subPixelH, &slope);
}


//한픽셀씩 찍는 방법
void COccluderBuffer::ScanLineScalar(int y, int parallelIndex, Float4MM& left, Float4MM& right)
{
	int startX, endX;
	float subPixelV;
	//if(false==GetLeftRightXAndSubPixelV_SSE(&left, &right, &startX, &endX, &subPixelV)) return;
	DefGetLeftRightXAndSubPixelV_SSE(left, right, startX, endX, subPixelV);

	assert(startX>=0 && startX<endX);

	Float4MM slope = Float4MM::XbaseSlope(&left, &right);
	Float4MM startP = RevisePointWithSubPixel(&left, subPixelV, &slope);

	int pixelCount = endX-startX;	
	for(int index=0; index<pixelCount; ++index)
	{
		Point pixelPoint(Point((short)(startX + index), (short)y));
		float* depthPointer = GetParallelDepthPointer(pixelPoint, parallelIndex);

		float currentZ = startP.Z() + slope.Z()*index;
		if(currentZ < *depthPointer) *depthPointer = currentZ;
	}
}

//16byte unalign되지 않은 start pixel부터 4pixel(16byte)씩 찍고,
//나머지 pixel하나씩 찍는 버젼
void COccluderBuffer::ScanLineSseUnAlign(int y, int parallelIndex, Float4MM& left, Float4MM& right)
{
	int startX, endX;
	float subPixelV;
	//if(false==GetLeftRightXAndSubPixelV_SSE(&left, &right, &startX, &endX, &subPixelV)) return;
	DefGetLeftRightXAndSubPixelV_SSE(left, right, startX, endX, subPixelV);

	assert(startX>=0 && startX<endX);

	Float4MM slope = Float4MM::XbaseSlope(&left, &right);
	Float4MM startP = RevisePointWithSubPixel(&left, subPixelV, &slope);

	int pixelCount=endX-startX;
	int pack4Count = pixelCount/4;
	if(pack4Count>0)
	{
		__m128 slopeZ4 = _mm_set1_ps(slope.Z());
		__m128 startZ4 = _mm_set1_ps(startP.Z());
		//z, z+slope, z+slope*2, z+slope*3
		startZ4 = _mm_add_ps(startZ4, _mm_mul_ps(slopeZ4, g_0123));

		//slope*4, slope*4, slope*4, slope*4
		__m128 slopeZ4Inc = _mm_mul_ps(slopeZ4, g_constNumbers[4]);

		__m128 currentZ4;
		__m128 pixelZ4;
		__m128 packIndex4;
		__m128 compare4;
		for(int packIndex=0; packIndex<pack4Count; ++packIndex)
		{
			Point pixelPoint((short)(startX + packIndex*4), (short)y);
			float* depthPointer = GetParallelDepthPointer(pixelPoint, parallelIndex);

			packIndex4 = _mm_set1_ps((float)packIndex);
			currentZ4 = _mm_add_ps(startZ4, _mm_mul_ps(slopeZ4Inc, packIndex4));

			pixelZ4 = _mm_loadu_ps(depthPointer);
			compare4 = _mm_cmplt_ps(currentZ4, pixelZ4);
			pixelZ4 = _mm_or_ps(_mm_and_ps(compare4, currentZ4), 
				_mm_andnot_ps(compare4, pixelZ4));

			_mm_storeu_ps(depthPointer, pixelZ4);
		}
	}

	float currentZ;
	for(int index=pack4Count*4; index<pixelCount; ++index)
	{
		Point pixelPoint(Point((short)(startX + index), (short)y));
		float* depthPointer = GetParallelDepthPointer(pixelPoint, parallelIndex);

		currentZ = startP.Z() + slope.Z()*index;
		if(currentZ < *depthPointer) *depthPointer = currentZ;
	}
}

//scanline을 병렬로 그릴순 있어도, scanline끼리는 서로 침범할수 없기때문에 가능한 방식임
void COccluderBuffer::ScanLineSseAlign(int y, int parallelIndex, Float4MM& left, Float4MM& right)
{
	assert(left.X()>-1 && right.X()<GetWidth(0)+1);

	int startX, endX;
	float subPixelV;
	//if(false==GetLeftRightXAndSubPixelV_SSE(&left, &right, &startX, &endX, &subPixelV)) return;
	DefGetLeftRightXAndSubPixelV_SSE(left, right, startX, endX, subPixelV);

	assert(startX>=0 && startX<endX);

	Float4MM slope = Float4MM::XbaseSlope(&left, &right);
	Float4MM startP = RevisePointWithSubPixel(&left, subPixelV, &slope);

	int packXStart, startXRemain;
	int packXEnd, endXRemain;

	ConvertPackStartAndRemain(startX, &packXStart, &startXRemain);
	ConvertPackEndAndRemain(endX, &packXEnd, &endXRemain);	
	assert(packXStart%4==0 && packXEnd%4==0);

	__m128 startCompareMask = StartCompareMask4(startXRemain);
	__m128 endCompareMask = EndCompareMask4(endXRemain);

	if(PackXIsEnd(packXStart, packXEnd)) startCompareMask = _mm_and_ps(startCompareMask, endCompareMask);

	////////////////////////////////////////////////////
	//slope관련 값들
	__m128 slopeZ4 = _mm_set1_ps(slope.Z());
	__m128 startZ4 = _mm_set1_ps(startP.Z());	

	//shift안됐으면(startShift==0) 0,1,2,3
	//1번 shift됐으면 (0,1,2,3)-(1,1,1,1) = (-1,0,1,2)
	__m128 startSlopeShift = _mm_sub_ps(g_0123, g_constNumbers[startXRemain]);
	startZ4 = _mm_add_ps(startZ4, _mm_mul_ps(slopeZ4, startSlopeShift));

	__m128 currentZ4 = startZ4;

	//slope*4, slope*4, slope*4, slope*4
	__m128 slopeZ4Inc = _mm_mul_ps(slopeZ4, g_constNumbers[4]);
	for(int packX=packXStart; packX<packXEnd; packX+=4)
	{
		Point pixelPoint((short)packX, (short)y);
		float*depthPointer = GetParallelDepthPointer(pixelPoint, parallelIndex);

		__m128 pixelZ4 = GetDepth4(depthPointer);

		//if(currenZ < pixelZ) compare=1; else compare=0;
		__m128 compare4 = _mm_cmplt_ps(currentZ4, pixelZ4);

		if(PackXIsStart(packX, packXStart)) compare4 = _mm_and_ps(compare4, startCompareMask);
		else if(PackXIsEnd(packX, packXEnd)) compare4 = _mm_and_ps(compare4, endCompareMask);

		if(_mm_movemask_ps(compare4)>0)
		{
			pixelZ4 = _mm_or_ps(_mm_and_ps(compare4, currentZ4), 
								_mm_andnot_ps(compare4, pixelZ4));

			_mm_store_ps(depthPointer, pixelZ4);
		}

		currentZ4 = _mm_add_ps(currentZ4, slopeZ4Inc);
	}
}

__m128 COccluderBuffer::StartCompareMask4(int remain)
{
	//-startShift, 1-startShift, 2-startShift, 3-startShift
	__m128 start4Mask = _mm_sub_ps(g_0123, g_constNumbers[remain]);
	return _mm_cmpge_ps(start4Mask, g_zero4);
}

__m128 COccluderBuffer::EndCompareMask4(int remain)
{
	//3-endShift, 2-endShift, 1-endShift, -endShift
	__m128 end4Mask = _mm_sub_ps(g_3210, g_constNumbers[remain]);
	return _mm_cmpge_ps(end4Mask, g_zero4);
}
