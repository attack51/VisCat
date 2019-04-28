#pragma once

#include "stdafx.h"
#include "Float3.h"
#include "Float4MM.h"
#include "MatrixMM.h"
#include "Side4.h"
#include "Point.h"
#include "Tile.h"
#include "ScanLineBorderVertices.h"
#include "Enums.h"
#include "DrawCallPin.h"

#include "dllexport.h"

#include <assert.h>

typedef unsigned short ushort;
typedef Side4(*PlaneSide4Func)(Float4MM*,Float4MM*);

#define MAX_PARALLEL_COUNT 8

#define VERTEX_BUFFER_SIZE 262144
#define INDEX_BUFFER_SIZE VERTEX_BUFFER_SIZE*3
#define MAX_DRAWCALL_COUNT 1000

class MinMax;
class MatrixU;
class Side4;
class Winding;

class VISCAT_LIB_API COccluderBuffer
{
//members
private:
	int m_depthLevelCount;
	
	int* m_width;
	int* m_height;
	int* m_levelPixelCounts;

	__m128i* m_iWidth4;
	__m128i* m_iHeight4;

	//zw are 1
	Float4MM m_screenScale;
	Float4MM m_screenShift;

	Float4MM m_ndcSpaceHalfShift;

	//xy and zw are same
	Float4MM* m_screenScale2;
	Float4MM* m_screenShift2;
	
	int m_parallelCount;
	float* m_buffer;
	int* m_parallelBufferOffset;
	
	float* m_cascadingMinBuffer;
	float* m_cascadingMaxBuffer;	
	int* m_cascadingBufferOffset;

	__m128i* m_8x2NeighborOffset4;
	
	MatrixMM m_vpMatrix;

	//clip coord
	Float4MM m_transformedVertices[VERTEX_BUFFER_SIZE];
	ushort m_indices[INDEX_BUFFER_SIZE];

	DrawCallPin m_drawCallPin[MAX_DRAWCALL_COUNT];
	int m_currentPin;
	
//methods
private:
	void InitDepthLevelCountAndWidthHeight(VISCAT_Resolution resolution);
	void InitBuffers();
	bool IsCullFaceInScreenCoord(VISCAT_CullMode cullMode, Winding* pWinding);
	inline int GetCascadingBufferOffset(int level) 
	{
		assert(level>0);
		return m_cascadingBufferOffset[level-1];
	}

	inline int GetParallelBufferOffset(int parallelIndex)
	{
		return m_parallelBufferOffset[parallelIndex];
	}

	////////////////////////////////////////////////////////////////////////////
	//Start Clip Winding Methods
	bool ClipWindingVertices(Winding* pWinding, PlaneSide4Func func);
	//End Clip Winding Methods

	////////////////////////////////////////////////////////////////////////////
	//Draw Triangle Methods 

	void DrawTriangle(	VISCAT_CullMode cullMode, 
						int parallelIndex,
						int ind0, 
						int ind1, 
						int ind2);

	////////////////////////////////////////////////////////////////////////////
	//Start Draw Winding Methdos 
	void DrawWinding(Winding* pWinding, int parallelIndex);
		
	void GetWindingTopBottomIndex(	Winding* pWinding, 
									int& topIndex, 
									int& bottomIndex);

	bool GetStartEndY(Float4MM* topVertex, Float4MM* bottomVertex, float* pOutCeilTopY, int* pOutStartY, int* pOutEndY);
	bool GetStartEndY_SSE(Float4MM* topVertex, Float4MM* bottomVertex, float* pOutCeilTopY, int* pOutStartY, int* pOutEndY);
	
	void GetWindingLeftRightIncreaseAndSlope(	Winding* pWinding,
												int topIndex,												
						    					int& leftIncIndex, 
												int& rightIncIndex,
												Float4MM& leftSlope, 
												Float4MM& rightSlope);

	bool UpdateNextIndex(Winding* pWinding, 
						 const int& y, 
						 const int& inc, 
						 int& curIndex, 
						 int& nextIndex);

	void UpdateScanLineBorder(	Winding* pWinding,
								const int& curIndex, 
								const int& nextIndex, 
								Float4MM& slope,
								Float4MM& p);
	
	Float4MM RevisePointWithSubPixel(const Float4MM* p, const float subPixel, const Float4MM* slope);

	void ScanLineScalar(int intY, int parallelIndex, Float4MM& left, Float4MM& right);
	void ScanLineSseUnAlign(int y, int parallelIndex, Float4MM& left, Float4MM& right);
	void ScanLineSseAlign(int intY, int parallelIndex, Float4MM& left, Float4MM& right);
	bool GetLeftRightXAndSubPixelV(	Float4MM* pLeft, 
									Float4MM* pRight, 
									int* pOutLeftX, 
									int* pOutRightX,
									float* pSubPixelV);

	bool GetLeftRightXAndSubPixelV_SSE(	Float4MM* pLeft, 
										Float4MM* pRight, 
										int* pOutLeftX, 
										int* pOutRightX,
										float* pSubPixelV);

	static inline void ConvertPackStartAndRemain(int start, int* packStart, int* startRemain)
	{
		*startRemain = start%4;
		*packStart = start - (*startRemain);
	}
	static inline void ConvertPackEndAndRemain(int end, int* packEnd, int* endRemain)
	{
		int endOver=end+3;
		*packEnd = endOver - endOver%4;
		*endRemain = (*packEnd) - end;
	}

	inline __m128 StartCompareMask4(int remain);
	inline __m128 EndCompareMask4(int remain);

	inline bool PackXIsStart(int packX, int packXStart) {return packX==packXStart;}
	inline bool PackXIsEnd(int packX, int packXEnd) {return (packX+4)==packXEnd;}

	//End Draw Winding Methdos 

	void BatchDraw();
	int SearchDrawCallPin(int triangleIndex);
	void MergeParallelBuffers();

	////////////////////////////////////////////////////////////////////////////
	//Start Create CascadingBuffer Methods	
	void CreateCascadingBuffer();
	void GetNeighborDepth8x2(	float* pCurrent, 
								int level,
								__m128* depthLt, 
								__m128* depthRt, 
								__m128* depthLb, 
								__m128* depthRb);
	__m128 CombinatedMin4(const __m128& parentLt, const __m128& parentRt, 
						  const __m128& parentLb, const __m128& parentRb);
	__m128 CombinatedMax4(const __m128& parentLt, const __m128& parentRt, 
						  const __m128& parentLb, const __m128& parentRb);

	////////////////////////////////////////////////////////////////////////////
	//Start BoxIsVisible Methods
	bool GetNdcBoxMinMax(Float4MM* pNdcMin, Float4MM* pNdcMax, Float4MM* pOccludeeVertices);	

	void OccludeeNdcVerticesToNdcMinMax(Float4MM* ndcMin, 
										Float4MM* ndcMax,
										Float4MM* pOccldeeClippedVertices,
										int occldeeClippedVertexCount);
	
	void NdcMinMaxHalftShift(__m128* pNdcMin, __m128* pNdcMax);

	int ClipOccludeeVerticesByNearPlane(Float4MM* pOccludeeVertices, 
										Float4MM* pOccludeeFraction4,
				   						Side4* pOccludeeSide4,
										Float4MM* pOccldeeClippedVertices);

	bool OccludeeVertIsInsideOfNearPlane(Side4* pOccludeeSide4, int index);
	float GetOccludeeVertFraction(Float4MM* pOccludeeFraction4, int index);

	void GetBoxScreenStartEnd(	const Float4MM& min, 
								const Float4MM& max,
								Point& start,
								Point& end,
								int level);

	bool IsVisibleCascadeCheck(Float4MM& ndcMin, Float4MM& ndcMax);
	//End BoxIsVisible Methods 
public:
	COccluderBuffer(VISCAT_Resolution resolution);
	~COccluderBuffer();

	void SetViewProjMatrix(const MatrixU* vpMatrix);
	void FrameStart();
	void FrameEnd();
	void Clear();
	void DrawIndex(	const Float3* vertices, 
					const ushort* indices, 
					const MatrixU* worldMatrix, 
					VISCAT_CullMode cullMode,
					int vertexOffset, 
					int vertexCount, 
					int primitiveCount);

	//void DrawStrip(	const Float3* vertices, 
	//				const ushort* indices, 
	//				const MatrixU* worldMatrix,
	//				VISCAT_CullMode cullMode,
	//				int vertexOffset, 
	//				int vertexCount, 
	//				int primitiveCount);

	/// point¼ø¼­ 
	/// z         z
    /// | 7  6    | 3  2 
    /// | 4  5    | 0  1
    /// ---x      ---x
    ///  ¾Æ·§ÂÊ   À­ÂÊ
    /// 
	bool BoxIsVisible(const Float3* boxPoints);

	//getter functions
	inline int GetWidth(int depthLevel)	{return m_width[depthLevel];}
	inline int GetHeight(int depthLevel) {return m_height[depthLevel];}
	inline int GetPixelCounts(int depthLevel) {return m_levelPixelCounts[depthLevel]; }
	inline int GetDepthLevelCount() {return m_depthLevelCount;}
	
	float GetPixel(Point screenPoint);
	MinMax GetPixelMinMax(Point screenPoint, int depthLevel);

	//Tile getter		
	//Pixel Pointer Getter	
	inline float* GetDepthPointer(int screenIndex)
	{
		return GetParallelDepthStartPointer(0) + screenIndex;
	}

	inline float* GetDepthPointer(Point screen)
	{
		int screenIndex = ScreenPointToIndex(screen, 0);
		return GetParallelDepthStartPointer(0) + screenIndex;
	}

	inline float* GetParallelDepthStartPointer(int parallelIndex)
	{
		return m_buffer + GetParallelBufferOffset(parallelIndex);
	}
	
	inline float* GetParallelDepthPointer(int screenIndex, int parallelIndex)
	{
		return GetParallelDepthStartPointer(parallelIndex) + screenIndex;
	}

	inline float* GetParallelDepthPointer(Point screen, int parallelIndex)
	{
		int screenIndex = ScreenPointToIndex(screen, 0);
		return GetParallelDepthStartPointer(parallelIndex) + screenIndex;
	}

	inline void GetMinMaxDepthPointer(Point screen, int level, float** ppMin, float** ppMax)
	{
		assert(level>0);

		int levelOffset = GetCascadingBufferOffset(level);
		int screenIndex = ScreenPointToIndex(screen, level);
		*ppMin = m_cascadingMinBuffer + levelOffset + screenIndex;
		*ppMax = m_cascadingMaxBuffer + levelOffset + screenIndex;
	}
	
	//convert functions
	inline int ScreenPointToIndex(Point screenPoint, int level)
	{ return screenPoint.ToIndex(GetWidth(level)); }

	inline Point ScreenIndexToPoint(int screenIndex, int level)
	{ return Point::FromIndex(screenIndex, (short)GetWidth(level)); }

	inline int LocalPixelPointToIndex(Point localPixelPoint)
	{ return localPixelPoint.ToIndex(TILE_BUFFER_WIDTH); }
	
	inline Point LocalPixelIndexToPoint(int localPixelIndex) 
	{ return Point::FromIndex(localPixelIndex, TILE_BUFFER_WIDTH); }
};

#define GetDepth4(pPointer) _mm_load_ps(pPointer)