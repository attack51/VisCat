#include "stdafx.h"
#include "Float4MM.h"

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

#include "Float3.h"
#include "Side4.h"
#include "VisCatMath.h"
#include "sse_preset.h"

#include <ppl.h>

Float4MM::Float4MM(float x, float y, float z, float w)
{
	xyzw = _mm_set_ps(w,z,y,x);
}

Float4MM::Float4MM(const __m128& original)
{
	xyzw = original;
}

Float4MM::Float4MM(const Float4MM& original)
{
	xyzw = original.xyzw;
}

Float4MM::Float4MM(const Float3& original, float w)
{
	xyzw = _mm_set_ps(w, original.z,original.y,original.x);
}

Float4MM Float4MM::Add(const Float4MM* lhs, const Float4MM* rhs)
{
	return Float4MM(_mm_add_ps(lhs->xyzw, rhs->xyzw));
}

Float4MM Float4MM::Mul(const Float4MM* lhs, const Float4MM* rhs)
{
	return Float4MM(_mm_mul_ps(lhs->xyzw, rhs->xyzw));
}

Float4MM Float4MM::Mul(const Float4MM* lhs, const float rhs)
{
	__m128 m = _mm_set1_ps(rhs);
	return Float4MM(_mm_mul_ps(lhs->xyzw, m));
}

float Float4MM::Dot3(const Float4MM* lhs, const Float4MM* rhs)
{
	__m128 v = _mm_mul_ps(lhs->xyzw, rhs->xyzw);
	v.m128_f32[3]=0;

    v = _mm_hadd_ps(v, v);
    v = _mm_hadd_ps(v, v);

	return v.m128_f32[0];
}

Float4MM Float4MM::Cross3(const Float4MM* lhs, const Float4MM* rhs)
{
	//lhs.yzx * rhs.zxy -> lsh.xyz * rhs.yzx
	__m128 v0 = _mm_mul_ps(lhs->xyzw, _mm_shuffle_ps(rhs->xyzw, rhs->xyzw, _MM_SHUFFLE(3, 0, 2, 1)));
	//lhs.zxy * rhs.yzx -> lhs.yzx * rhs.xyz
	__m128 v1 = _mm_mul_ps(rhs->xyzw, _mm_shuffle_ps(lhs->xyzw, lhs->xyzw, _MM_SHUFFLE(3, 0, 2, 1)));
	
	v0 = _mm_sub_ps(v0, v1);
	v0 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 0, 2, 1));

	//yzx -> xzy
	return Float4MM(v0);
}

float Float4MM::Length3(const Float4MM* val)
{
	__m128 v = _mm_mul_ps(val->xyzw, val->xyzw);
	v.m128_f32[3]=0;

	v = _mm_hadd_ps(v, v);
    v = _mm_hadd_ps(v, v);

	v = _mm_sqrt_ss(v);

	return v.m128_f32[0];
}

Float4MM Float4MM::Normalize3(const Float4MM* val)
{
	__m128 lengthInvSq = _mm_mul_ps(val->xyzw, val->xyzw);
	lengthInvSq.m128_f32[3]=0;

	lengthInvSq = _mm_hadd_ps(lengthInvSq, lengthInvSq);
    lengthInvSq = _mm_hadd_ps(lengthInvSq, lengthInvSq);

	lengthInvSq = _mm_rsqrt_ps(lengthInvSq);

	__m128 newXYZW = _mm_mul_ps(val->xyzw, lengthInvSq);
	newXYZW.m128_f32[3] = val->xyzw.m128_f32[3];

	return Float4MM(newXYZW);
}

Float4MM Float4MM::XbaseSlope(Float4MM* start, Float4MM* end)
{	
	__m128 difference = _mm_sub_ps(end->xyzw, start->xyzw);
	if(difference.m128_f32[0]==0) return Float4MM(_mm_mul_ps(difference, g_infinite4));

	__m128 x4 = _mm_shuffle_ps(difference, difference, _MM_SHUFFLE(0, 0, 0, 0));
	difference = _mm_div_ps(difference, x4);
	return Float4MM(difference);
}

Float4MM Float4MM::YbaseSlope(Float4MM* start, Float4MM* end)
{	
	__m128 difference = _mm_sub_ps(end->xyzw, start->xyzw);
	if(difference.m128_f32[1]==0) return Float4MM(_mm_mul_ps(difference, g_infinite4));

	__m128 y4 = _mm_shuffle_ps(difference, difference, _MM_SHUFFLE(1, 1, 1, 1));
	difference = _mm_div_ps(difference, y4);
	return Float4MM(difference);
}

Float4MM Float4MM::Lerp(Float4MM* lhs, Float4MM* rhs, float factor)
{
	__m128 factor4 = _mm_set1_ps(factor);
	__m128 invFactor4 = _mm_sub_ps(g_one4, factor4);
	__m128 v = _mm_add_ps(_mm_mul_ps(lhs->xyzw, invFactor4), _mm_mul_ps(rhs->xyzw, factor4));

	return Float4MM(v);
}

//for triangle cohen-sutherland code
void Float4MM::ExtractXYZW3(Float4MM *points, __m128* x3, __m128* y3, __m128* z3, __m128* w3)
{
	__m128 _0x0y_1x1y = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(1,0,1,0));
	__m128 _0z0w_1z1w = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(3,2,3,2));

	*x3 = _mm_shuffle_ps(_0x0y_1x1y, points[2].xyzw, _MM_SHUFFLE(0,0,2,0));
	*y3 = _mm_shuffle_ps(_0x0y_1x1y, points[2].xyzw, _MM_SHUFFLE(1,1,3,1));

	*z3 = _mm_shuffle_ps(_0z0w_1z1w, points[2].xyzw, _MM_SHUFFLE(2,2,2,0));
	*w3 = _mm_shuffle_ps(_0z0w_1z1w, points[2].xyzw, _MM_SHUFFLE(3,3,3,1));
}

void Float4MM::ExtractXW4(Float4MM *points, __m128* x4, __m128* w4)
{
	__m128 _0x0w_1x1w = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(3,0,3,0));
	__m128 _2x2w_3x3w = _mm_shuffle_ps(points[2].xyzw, points[3].xyzw, _MM_SHUFFLE(3,0,3,0));
	*x4 = _mm_shuffle_ps(_0x0w_1x1w, _2x2w_3x3w, _MM_SHUFFLE(2,0,2,0));
	*w4 = _mm_shuffle_ps(_0x0w_1x1w, _2x2w_3x3w, _MM_SHUFFLE(3,1,3,1));
}

void Float4MM::ExtractYW4(Float4MM *points, __m128* y4, __m128* w4)
{
	__m128 _0y0w_1y1w = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(3,1,3,1));
	__m128 _2y2w_3y3w = _mm_shuffle_ps(points[2].xyzw, points[3].xyzw, _MM_SHUFFLE(3,1,3,1));
	*y4 = _mm_shuffle_ps(_0y0w_1y1w, _2y2w_3y3w, _MM_SHUFFLE(2,0,2,0));
	*w4 = _mm_shuffle_ps(_0y0w_1y1w, _2y2w_3y3w, _MM_SHUFFLE(3,1,3,1));
}

void Float4MM::ExtractZW4(Float4MM *points, __m128* z4, __m128* w4)
{
	__m128 _0z0w_1z1w = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(3,2,3,2));
	__m128 _2z2w_3z3w = _mm_shuffle_ps(points[2].xyzw, points[3].xyzw, _MM_SHUFFLE(3,2,3,2));
	*z4 = _mm_shuffle_ps(_0z0w_1z1w, _2z2w_3z3w, _MM_SHUFFLE(2,0,2,0));
	*w4 = _mm_shuffle_ps(_0z0w_1z1w, _2z2w_3z3w, _MM_SHUFFLE(3,1,3,1));
}

void Float4MM::ExtractZ4(Float4MM *points, __m128* z4)
{
	__m128 _0z0w_1z1w = _mm_shuffle_ps(points[0].xyzw, points[1].xyzw, _MM_SHUFFLE(3,2,3,2));
	__m128 _2z2w_3z3w = _mm_shuffle_ps(points[2].xyzw, points[3].xyzw, _MM_SHUFFLE(3,2,3,2));
	*z4 = _mm_shuffle_ps(_0z0w_1z1w, _2z2w_3z3w, _MM_SHUFFLE(2,0,2,0));
}

Side4 Float4MM::GetLeftPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 x4, w4;
	ExtractXW4(points, &x4, &w4);
	//-w
	w4 = _mm_sub_ps(g_zero4, w4);

	//-w - x
	__m128 f = _mm_sub_ps(w4, x4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

Side4 Float4MM::GetRightPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 x4, w4;
	ExtractXW4(points, &x4, &w4);

	//x - w
	__m128 f = _mm_sub_ps(x4, w4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

Side4 Float4MM::GetBottomPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 y4, w4;
	ExtractYW4(points, &y4, &w4);
	//-w
	w4 = _mm_sub_ps(g_zero4, w4);

	//-w - x
	__m128 f = _mm_sub_ps(w4, y4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

Side4 Float4MM::GetTopPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 y4, w4;
	ExtractYW4(points, &y4, &w4);
	
	//y - w
	__m128 f = _mm_sub_ps(y4, w4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

Side4 Float4MM::GetNearPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 z4;
	ExtractZ4(points, &z4);
		
	//-z	
	__m128 f = _mm_sub_ps(g_zero4, z4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

Side4 Float4MM::GetFarPlaneSide4(Float4MM* points, Float4MM* frac4)
{
	__m128 z4, w4;
	ExtractZW4(points, &z4, &w4);

	//z - w
	__m128 f = _mm_sub_ps(z4, w4);
	*frac4 = Float4MM(f);
	return Side4(f);
}

AllSide Float4MM::GetPlaneSides(	Float4MM* points, 
									Float4MM* fracs, 
									Side4* sides, 
									int count, 
									GetPlaneSide4Func getPlaneSide4Func)
{
	int pack4LoopCount = (count+3)/4;
	
	for(int pack4Index=0; pack4Index<pack4LoopCount; ++pack4Index)
	{
		int index = pack4Index*4;
		sides[pack4Index] = getPlaneSide4Func(&points[index],
											  &fracs[pack4Index]);
	}

	return Side4::CombinationAllSide(sides, count);
}

void Float4MM::GetPlaneSidesWithoutAllSide(	Float4MM* points, 
											Float4MM* fracs, 
											Side4* sides, 
											int count, 
											GetPlaneSide4Func getPlaneSide4Func)
{
	int pack4LoopCount = (count+3)/4;
	
	for(int pack4Index=0; pack4Index<pack4LoopCount; ++pack4Index)
	{
		int index = pack4Index*4;
		sides[pack4Index] = getPlaneSide4Func(&points[index],
											  &fracs[pack4Index]);
	}
}

//ndcÁÂÇ¥°è¿¡¼­ pixelÁÂÇ¥°è ÁÂ»óÀ¸·Î ¹ÝÇÈ¼¿ ¹Î´Ù
//ndc¿¡¼­´Â À­ÂÊÀÌ ¾ç¼ö±â ¶§¹®¿¡ y´Â +0.5ÇÑ´Ù
//zw is 0
Float4MM Float4MM::GetNdcSpaceHalfShift(int width, int height)
{
	__m128 shift = _mm_set_ps(0, 0, 0.5f, -0.5f);
	__m128 screenSize = _mm_set_ps(1, 1, (float)height, (float)width);
	return Float4MM(_mm_div_ps(shift, screenSize));
}

//zw is 1
void Float4MM::GetScreenScaleAndShift(int width, int height, Float4MM* scale, Float4MM* shift)
{	
	__m128 scaleHalf = _mm_set_ps(1, 1, -0.5f, 0.5f);
	__m128 shiftHalf = _mm_set_ps(0, 0, 0.5f, 0.5f);

	__m128 screenSize = _mm_set_ps(1, 1, (float)height, (float)width);
	*scale = Float4MM(_mm_mul_ps(screenSize, scaleHalf));
	*shift = Float4MM(_mm_sub_ps(_mm_mul_ps(screenSize, shiftHalf), shiftHalf));
}


//scale2.xyxy, shift.xyxy
//xy and zw are same
void Float4MM::GetScreenScaleAndShift2(int width, int height, Float4MM* scale2, Float4MM* shift2)
{	
	__m128 scaleHalf = _mm_set_ps(-0.5f, 0.5f, -0.5f, 0.5f);
	__m128 shiftHalf = _mm_set_ps(0.5f, 0.5f, 0.5f, 0.5f);

	__m128 screenSize = _mm_set_ps((float)height, (float)width, (float)height, (float)width);
	*scale2 = Float4MM(_mm_mul_ps(screenSize, scaleHalf));
	*shift2 = Float4MM(_mm_mul_ps(screenSize, shiftHalf));
}

void Float4MM::ClipToScreenArray(const Float4MM& screenScale, 
								 const Float4MM& screenShift, 
								 const Float4MM* clipVerts, 
							     Float4MM* screenVerts,
								 int vertexCount)
{
	for(int i=0; i<vertexCount; ++i)
	{
		__m128 w4;
		__m128 temp;
		//ndc
		w4 = _mm_shuffle_ps(clipVerts[i].xyzw, clipVerts[i].xyzw, _MM_SHUFFLE(3, 3, 3, 3));
		temp = _mm_div_ps(clipVerts[i].xyzw, w4);	

		//ndc->screen
		temp = _mm_mul_ps(temp, screenScale.xyzw);
		temp = _mm_add_ps(temp, screenShift.xyzw);

		screenVerts[i] = Float4MM(temp);
	}
}

void Float4MM::ClipToNdcArray(const Float4MM* clipVerts, Float4MM* ndcVerts, int vertexCount)
{
	for(int i=0; i<vertexCount; ++i)
	{
		__m128 w4;
		__m128 temp;
		//ndc
		w4 = _mm_shuffle_ps(clipVerts[i].xyzw, clipVerts[i].xyzw, _MM_SHUFFLE(3, 3, 3, 3));
		temp = _mm_div_ps(clipVerts[i].xyzw, w4);	

		ndcVerts[i] = Float4MM(temp);
	}
}

//return value x is screen x min
//return value y is screen y max (because is reverse)
//return value z is screen x max
//return value w is screen y min
Float4MM Float4MM::NdcMinMaxToScreenXY(	const Float4MM& screenScale2, 
										const Float4MM& screenShift2,
										const Float4MM* ndcVertMin, 
										const Float4MM* ndcVertMax)
{
	__m128 result = _mm_shuffle_ps(ndcVertMin->xyzw, ndcVertMax->xyzw, _MM_SHUFFLE(1,0,1,0));
	result = _mm_mul_ps(result, screenScale2.xyzw);
	result = _mm_add_ps(result, screenShift2.xyzw);

	return Float4MM(result);
}