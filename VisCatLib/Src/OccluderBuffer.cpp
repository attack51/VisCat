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

COccluderBuffer::COccluderBuffer(VISCAT_Resolution resolution)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_parallelCount = min(MAX_PARALLEL_COUNT, sysinfo.dwNumberOfProcessors);

	InitDepthLevelCountAndWidthHeight(resolution);
	InitBuffers();

	Clear();
}

void COccluderBuffer::InitDepthLevelCountAndWidthHeight(VISCAT_Resolution resolution)
{
	//32x16일때 32,16,8,4 이렇게 깊이가 4
	m_depthLevelCount=(int)resolution;

	m_width = new int[m_depthLevelCount];
	m_height = new int[m_depthLevelCount];
	m_levelPixelCounts = new int[m_depthLevelCount];
	m_iWidth4 = (__m128i*)_mm_malloc(m_depthLevelCount*sizeof(__m128i), 16);
	m_iHeight4 = (__m128i*)_mm_malloc(m_depthLevelCount*sizeof(__m128i), 16);

	m_screenScale2 = (Float4MM*)_mm_malloc(m_depthLevelCount*sizeof(Float4MM), 16);
	m_screenShift2 = (Float4MM*)_mm_malloc(m_depthLevelCount*sizeof(Float4MM), 16);
	m_8x2NeighborOffset4 = (__m128i*)_mm_malloc(m_depthLevelCount*sizeof(__m128i), 16);

	int currentWidth = 4;
	int currentHeight = 2;
	for(int depthLevel=m_depthLevelCount-1; depthLevel>=0; --depthLevel)
	{
		m_width[depthLevel] = currentWidth;
		m_height[depthLevel] = currentHeight;
		m_levelPixelCounts[depthLevel ] = currentWidth*currentHeight;
		m_iWidth4[depthLevel] = _mm_set1_epi32(currentWidth);
		m_iHeight4[depthLevel] = _mm_set1_epi32(currentHeight);
		m_8x2NeighborOffset4[depthLevel] = _mm_set_epi32(currentWidth+4, currentWidth, 4, 0);

		Float4MM::GetScreenScaleAndShift2(	currentWidth, 
			currentHeight, 
			&m_screenScale2[depthLevel], 
			&m_screenShift2[depthLevel]);

		currentWidth = currentWidth*2;
		currentHeight = currentHeight*2;		
	}

	Float4MM::GetScreenScaleAndShift(GetWidth(0), GetHeight(0), &m_screenScale, &m_screenShift);
	m_ndcSpaceHalfShift = Float4MM::GetNdcSpaceHalfShift(GetWidth(0), GetHeight(0));
}

void COccluderBuffer::InitBuffers()
{
	int halfGuardBand=8;//8*4byte=32byte
	int guardBand=halfGuardBand*2;
	int bufferPixelCount=GetPixelCounts(0)*guardBand*m_parallelCount;
	m_buffer = (float*)_mm_malloc(bufferPixelCount*sizeof(float), 16);

	m_parallelBufferOffset = new int[m_parallelCount];
	for(int i=0; i<m_parallelCount; ++i)
	{
		m_parallelBufferOffset[i] = halfGuardBand+(GetPixelCounts(0)+guardBand)*i;
	}
	
	m_cascadingBufferOffset = new int[m_depthLevelCount-1];
	int cascadingTotalPixelCount=0;
	for(int i=1; i<m_depthLevelCount; ++i)
	{
		m_cascadingBufferOffset[i-1] = cascadingTotalPixelCount;
		cascadingTotalPixelCount+=GetPixelCounts(i);
	}
	assert((cascadingTotalPixelCount%4)==0);

	m_cascadingMinBuffer = (float*)_mm_malloc(cascadingTotalPixelCount*sizeof(float), 16);
	m_cascadingMaxBuffer = (float*)_mm_malloc(cascadingTotalPixelCount*sizeof(float), 16);
}

COccluderBuffer::~COccluderBuffer()
{
	_mm_free(m_buffer);
	_mm_free(m_cascadingMinBuffer);
	_mm_free(m_cascadingMaxBuffer);

	delete[] m_width;
	delete[] m_height;
	delete[] m_levelPixelCounts;

	delete[] m_parallelBufferOffset;
	delete[] m_cascadingBufferOffset;
	
	_mm_free(m_iWidth4);
	_mm_free(m_iHeight4);
	_mm_free(m_screenScale2);
	_mm_free(m_screenShift2);
	_mm_free(m_8x2NeighborOffset4);
}

void COccluderBuffer::SetViewProjMatrix(const MatrixU* vpMatrix)
{
	m_vpMatrix = MatrixMM(*vpMatrix);
}

void COccluderBuffer::FrameStart()
{
	m_currentPin=0;
}

void COccluderBuffer::FrameEnd()
{
	BatchDraw();
	MergeParallelBuffers();
	CreateCascadingBuffer();
}

void COccluderBuffer::BatchDraw()
{
	DrawCallPin* pLastPin = &m_drawCallPin[m_currentPin-1];
	int totalTriangleCount = pLastPin->TriangleOffsetEnd;
	if(totalTriangleCount==0) return;

	int loopCursors[MAX_PARALLEL_COUNT+1];
	GetSeperatedLoopCursors(totalTriangleCount, loopCursors, m_parallelCount);

	//for(int p=0; p<m_parallelCount; ++p)
	Concurrency::parallel_for(0, m_parallelCount, [&](int p)
	{
		int drawCallPin = SearchDrawCallPin(loopCursors[p]);
		for(int triangleIndex=loopCursors[p]; triangleIndex<loopCursors[p+1]; ++triangleIndex)
		{
			if(m_drawCallPin[drawCallPin].TriangleOffsetEnd <= triangleIndex) ++drawCallPin;

			DrawCallPin* pCurrentPin = &m_drawCallPin[drawCallPin];

			int ind0 = m_indices[triangleIndex*3+0] + pCurrentPin->VertexOffsetStart;
			int ind1 = m_indices[triangleIndex*3+1] + pCurrentPin->VertexOffsetStart;
			int ind2 = m_indices[triangleIndex*3+2] + pCurrentPin->VertexOffsetStart;

			DrawTriangle(pCurrentPin->CullMode, p, ind0, ind1, ind2);
		}
	});
	//}
}

int COccluderBuffer::SearchDrawCallPin(int triangleIndex)
{
	int first=0;
	int last=m_currentPin;
	int searchIndex=first + (last-first)/2;
	while(last-first > 1)
	{		
		if(m_drawCallPin[searchIndex].TriangleOffsetStart > triangleIndex)
		{
			last = searchIndex;
			searchIndex=first + (last-first)/2;
		}
		else
		{
			if(	m_drawCallPin[searchIndex].TriangleOffsetEnd > triangleIndex) return searchIndex;
			else
			{
				first = searchIndex;
				searchIndex=first + (last-first)/2;
			}
		}
	}
	return searchIndex;
}

void COccluderBuffer::MergeParallelBuffers()
{
	__m128 mergedPixel4, pixel4;
	for(int y=0; y<GetHeight(0); ++y)
	{
		for(int x=0; x<GetWidth(0); x+=4)
		{
			int pixelIndex = ScreenPointToIndex(Point((short)x, (short)y), 0);

			float* baseDepthPointer = GetParallelDepthPointer(pixelIndex, 0);
			
			mergedPixel4 = GetDepth4(baseDepthPointer);

			for(int p=1; p<m_parallelCount; ++p)
			{
				float* currentDepthPointer = GetParallelDepthPointer(pixelIndex, p);
				pixel4 = GetDepth4(currentDepthPointer);
				mergedPixel4 = _mm_min_ps(mergedPixel4, pixel4);
			}

			_mm_store_ps(baseDepthPointer, mergedPixel4);
		}
	}
}

void COccluderBuffer::Clear()
{
	__m128 clearValue = _mm_set1_ps(1.0f);

	Concurrency::parallel_for(0, m_parallelCount, [&](int p)
	{
		float* depthPointer=GetParallelDepthStartPointer(p);
		for(int pixelIndex=0; pixelIndex<GetPixelCounts(0); pixelIndex+=4)
		{
			_mm_store_ps(depthPointer+pixelIndex, clearValue);
		}
	});
}

bool COccluderBuffer::IsCullFaceInScreenCoord(VISCAT_CullMode cullMode, Winding* pWinding)
{
	Float4MM* pV0 = pWinding->ReadVertex(0);
	Float4MM* pV1 = pWinding->ReadVertex(1);
	Float4MM* pV2 = pWinding->ReadVertex(2);

	//D1 = v1.xy - v0.xy
	//D2 = v2.xy - v0.xy
	//O = D1.xy*D2.yx
	//c = O.x - O.y
	__m128 v1xyv2xy = _mm_shuffle_ps(pV1->xyzw, pV2->xyzw, _MM_SHUFFLE(1,0,1,0));
	__m128 v0xyxy = _mm_shuffle_ps(pV0->xyzw, pV0->xyzw, _MM_SHUFFLE(1,0,1,0));

	__m128 D1xyD2xy = _mm_sub_ps(v1xyv2xy, v0xyxy);
	__m128 c = _mm_mul_ps(D1xyD2xy, _mm_shuffle_ps(D1xyD2xy,D1xyD2xy,_MM_SHUFFLE(0,1,2,3)));
	c = _mm_hsub_ps(c, c);

	//음수이면 왼손으로 감긴 방향이 화면방향이라는 의미.
	//양수이면 왼손으로 감긴 방향이 화면 반대 방향이라는 의미.
	//왼손으로 감으면 face기준으로 시계방향이기 때문에 ccw는 감긴 뒷면을,
	//cw는 감긴면을 culling한다는 의미가 된다
	float coilOrientation=c.m128_f32[0];

	if(coilOrientation==0) return true;
	if(VISCAT_CullMode::CCW == cullMode) 
	{
		return coilOrientation<0;
	}
	if(VISCAT_CullMode::CW == cullMode)
	{
		return coilOrientation>0;
	}

	return false;
}

bool COccluderBuffer::ClipWindingVertices(Winding* pWinding, PlaneSide4Func planeSide4Func)
{
	AllSide allSide=Float4MM::GetPlaneSides(pWinding->ReadVertex(0),
											pWinding->m_vertsFractions4,
											pWinding->m_vertsSides4,
											pWinding->VertexCount(),
											planeSide4Func);

	if(AllSide::AllOutside==allSide) return false;	
	if(AllSide::AllInside==allSide) return true;	//no need swap with this plane.

	int currentIndex, nextIndex;
	currentIndex=0;

	for(int i=1; i<=pWinding->VertexCount(); ++i)
	{
		nextIndex = i%pWinding->VertexCount();

		Float4MM* currentVert = pWinding->ReadVertex(currentIndex);	
		Float4MM* nextVert = pWinding->ReadVertex(nextIndex);

		if(pWinding->IsInsideOfFrustum(currentIndex))
		{
			pWinding->WriteVertex(currentVert);
		}

		if(pWinding->IsInsideOfFrustum(currentIndex) != pWinding->IsInsideOfFrustum(nextIndex))
		{
			float currentFrac=pWinding->GetVertFraction(currentIndex);
			float nextFrac=pWinding->GetVertFraction(nextIndex);
			float lerpFactor = GetLerpFactor(currentFrac,nextFrac);

			Float4MM newVert = Float4MM::Lerp(currentVert, nextVert, lerpFactor);
			pWinding->WriteVertex(&newVert);
		}

		currentIndex = nextIndex;
		currentVert = nextVert;		
	}

	pWinding->SwapAndClear();
	assert(pWinding->VertexCount()>=3);

	return true;
}

void COccluderBuffer::DrawIndex(const Float3* vertices, 
								const ushort* indices, 
								const MatrixU* worldMatrix,
								VISCAT_CullMode cullMode,
								int vertexOffset, 
								int vertexCount, 
								int primitiveCount)
{
	MatrixMM worldMatrixMM(*worldMatrix);
	MatrixMM wvpMatrix = MatrixMM::Mul(&worldMatrixMM, &m_vpMatrix);

	int triangleOffset;	
	int vertexBufferOffset;
	int useVertexCount=vertexCount-vertexOffset;

	if(m_currentPin==0)
	{
		triangleOffset=0;
		vertexBufferOffset=0;
	}
	else
	{
		DrawCallPin* pPrevPin  = &m_drawCallPin[m_currentPin-1];
		triangleOffset=pPrevPin->TriangleOffsetEnd;
		vertexBufferOffset = pPrevPin->VertexOffsetEnd;
	}

	int indexOffset = triangleOffset*3;
	int indexCount = primitiveCount*3;

	if((vertexBufferOffset + useVertexCount) > VERTEX_BUFFER_SIZE) return;
	if((indexOffset+indexCount)>INDEX_BUFFER_SIZE) return;

	memcpy(m_indices+indexOffset, indices, indexCount*sizeof(ushort));

	for(int i=0; i<useVertexCount; ++i)
	{
		m_transformedVertices[vertexBufferOffset+i] = 
			MatrixMM::TransformPoint(&wvpMatrix, vertices+vertexOffset+i);
	}

	m_drawCallPin[m_currentPin].Set(triangleOffset, primitiveCount, vertexBufferOffset, useVertexCount, cullMode);
	++m_currentPin;
}



void COccluderBuffer::DrawTriangle(VISCAT_CullMode cullMode, 
								   int parallelIndex,
								   int ind0, 
								   int ind1, 
								   int ind2)
{	
	//미생성 삼각형
	if(ind0 == ind1 || ind0 == ind2 || ind1 == ind2) return;

	//clip coord
	Winding winding(&m_transformedVertices[ind0], &m_transformedVertices[ind1], &m_transformedVertices[ind2]);

	unsigned char intersetCsCode;
	if(false==GetCohenSutherlandCodeSSE(winding.ReadVertex(0), &intersetCsCode)) return;

	if((intersetCsCode & CS_CODE_NEAR)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetNearPlaneSide4)) return;
	if((intersetCsCode & CS_CODE_FAR)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetFarPlaneSide4)) return;
	if((intersetCsCode & CS_CODE_LEFT)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetLeftPlaneSide4)) return;
	if((intersetCsCode & CS_CODE_RIGHT)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetRightPlaneSide4)) return;
	if((intersetCsCode & CS_CODE_TOP)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetTopPlaneSide4)) return;
	if((intersetCsCode & CS_CODE_BOTTOM)>0)	if(false==ClipWindingVertices(&winding, Float4MM::GetBottomPlaneSide4)) return;

	assert(winding.VertexCount()>=3);

	Float4MM::ClipToScreenArray(m_screenScale, 
								m_screenShift, 
								winding.ReadVertex(0), 
								winding.ReadVertex(0),
								winding.VertexCount());

	if(IsCullFaceInScreenCoord(cullMode, &winding)) return;

	DrawWinding(&winding, parallelIndex);	
}

float COccluderBuffer::GetPixel(Point screenPoint)
{
	return *(GetDepthPointer(screenPoint));
}

MinMax COccluderBuffer::GetPixelMinMax(Point screenPoint, int depthLevel)
{
	if(depthLevel==0) 
	{
		float value = GetPixel(screenPoint);
		return MinMax(value, value);
	}

	float* pMin;
	float* pMax;
	GetMinMaxDepthPointer(screenPoint, depthLevel, &pMin, &pMax);

	return MinMax(*pMin, *pMax);
}