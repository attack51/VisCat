#pragma once

#include "dllexport.h"
#include <pmmintrin.h>

class Float3;
class MatrixMM;
class Side4;
enum AllSide;
class Float4MM;

typedef Side4(*GetPlaneSide4Func)(Float4MM*,Float4MM*);

#define _float4CeilX(f4) _mm_ceil_ps((f4).xyzw).m128_f32[0]
#define _float4CeilY(f4) _mm_ceil_ps((f4).xyzw).m128_f32[1]

class _MM_ALIGN16 VISCAT_LIB_API Float4MM
{
public:
	//check from clip coord
	static void ExtractXYZW3(Float4MM *points, __m128* x3, __m128* y3, __m128* z3, __m128* w3);
	static void ExtractXW4(Float4MM *points, __m128* x4, __m128* w4);
	static void ExtractYW4(Float4MM *points, __m128* y4, __m128* w4);
	static void ExtractZW4(Float4MM *points, __m128* z4, __m128* w4);
	static void ExtractZ4(Float4MM *points, __m128* z4);

	static Side4 GetLeftPlaneSide4(Float4MM* points, Float4MM* frac4);
	static Side4 GetRightPlaneSide4(Float4MM* points, Float4MM* frac4);
	static Side4 GetBottomPlaneSide4(Float4MM* points, Float4MM* frac4);
	static Side4 GetTopPlaneSide4(Float4MM* points, Float4MM* frac4);
	static Side4 GetNearPlaneSide4(Float4MM* points, Float4MM* frac4);
	static Side4 GetFarPlaneSide4(Float4MM* points, Float4MM* frac4);

	static AllSide GetPlaneSides(	Float4MM* points, 
									Float4MM* fracs, 
									Side4* sides, 
									int count, 
									GetPlaneSide4Func getPlaneSide4Func);

	static void GetPlaneSidesWithoutAllSide(Float4MM* points, 
											Float4MM* fracs, 
											Side4* sides, 
											int count, 
											GetPlaneSide4Func getPlaneSide4Func);

public:
	__m128 xyzw;

	inline Float4MM() {}
	Float4MM(float x, float y, float z, float w);
	Float4MM(const __m128& original);
	Float4MM(const Float4MM& original);
	Float4MM(const Float3& original, float w);

	//getter
	inline const float X() {return xyzw.m128_f32[0];}
	inline const float Y() {return xyzw.m128_f32[1];}
	inline const float Z() {return xyzw.m128_f32[2];}
	inline const float W() {return xyzw.m128_f32[3];}
	inline const float Get(int component) {return xyzw.m128_f32[component];}

	//setter
	inline void SetX(float x) {xyzw.m128_f32[0]=x;}
	inline void SetY(float y) {xyzw.m128_f32[1]=y;}
	inline void SetZ(float z) {xyzw.m128_f32[2]=z;}
	inline void SetW(float w) {xyzw.m128_f32[3]=w;}
	inline void Set(int component, float value) {xyzw.m128_f32[component]=value;}

	static Float4MM Add(const Float4MM* lhs, const Float4MM* rhs);
	static Float4MM Mul(const Float4MM* lhs, const Float4MM* rhs);
	static Float4MM Mul(const Float4MM* lhs, const float rhs);

	static float Dot3(const Float4MM* lhs, const Float4MM* rhs);	
	static Float4MM Cross3(const Float4MM* lhs, const Float4MM* rhs);//w는 처리안함		
	static float Length3(const Float4MM* val);
	static Float4MM Normalize3(const Float4MM* val);//w는 원본값 유지

	static Float4MM XbaseSlope(Float4MM* start, Float4MM* end);
	static Float4MM YbaseSlope(Float4MM* start, Float4MM* end);	
	static Float4MM Lerp(Float4MM* lhs, Float4MM* rhs, float factor);

	//zw is 0
	static Float4MM GetNdcSpaceHalfShift(int width, int height);
	//zw is 1	
	static void GetScreenScaleAndShift(int width, int height, Float4MM* scale, Float4MM* shift);
	//xy and zw are same
	static void GetScreenScaleAndShift2(int width, int height, Float4MM* scale2, Float4MM* shift2);
	static void ClipToScreenArray(const Float4MM& screenScale, 
								 const Float4MM& screenShift, 
								 const Float4MM* clipVerts, 
							     Float4MM* screenVerts,
								 int vertexCount);

	static void ClipToNdcArray(const Float4MM* clipVerts, Float4MM* ndcVerts, int vertexCount);

	static Float4MM NdcMinMaxToScreenXY(const Float4MM& screenScale2, 
										const Float4MM& screenShift2,
										const Float4MM* ndcVertMin, 
										const Float4MM* ndcVertMax);

friend MatrixMM;
};
