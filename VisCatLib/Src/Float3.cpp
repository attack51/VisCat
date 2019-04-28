#include "stdafx.h"
#include "Float3.h"

//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

float Float3::Dot(const Float3* lhs, const Float3* rhs)
{
	__m128 v0 = _mm_loadu_ps((float*)lhs);
	__m128 v1 = _mm_loadu_ps((float*)rhs);

	v0 = _mm_mul_ps(v0, v1);
	v0.m128_f32[3]=0;

    v0 = _mm_hadd_ps(v0, v0);
    v0 = _mm_hadd_ps(v0, v0);

	float result = v0.m128_f32[0];
	return result;
}

Float3 Float3::Cross(const Float3* lhs, const Float3* rhs)
{
	//x = lhs.y*rhs.z - lhs.z*rhs.y
	//y = lhs.z*rhs.x - lhs.x*rhs.z
	//z = lhs.x*rhs.y - lhs.y*rhs.x
	
	__m128 v0 = _mm_loadu_ps((float*)lhs);
	__m128 v1 = _mm_loadu_ps((float*)rhs);

	//1Â÷ ±¸Çö
	//__m128 v0yzx = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 0, 2, 1));
	//__m128 v1zxy = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 1, 0, 2));
	//__m128 leftMul = _mm_mul_ps(v0yzx, v1zxy);

	//__m128 v0zxy = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 1, 0, 2));
	//__m128 v1yzx = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1));
	//__m128 rightMul = _mm_mul_ps(v0zxy, v1yzx);
	//__m128 r = _mm_sub_ps(leftMul, rightMul);
	//return Float3(r.m128_f32[0], r.m128_f32[1], r.m128_f32[2]);

	//more optimize 
	//lhs.yzx * rhs.zxy -> lsh.xyz * rhs.yzx
	__m128 leftMul = _mm_mul_ps(v0, _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1)));
	//lhs.zxy * rhs.yzx -> lhs.yzx * rhs.xyz
	__m128 rightMul = _mm_mul_ps(v1, _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 0, 2, 1)));
	
	__m128 r = _mm_sub_ps(leftMul, rightMul);

	//yzx -> xyz
	return Float3(r.m128_f32[1], r.m128_f32[2], r.m128_f32[0]);
}

float Float3::Length(const Float3* val)
{
	__m128 v = _mm_loadu_ps((float*)val);
	v = _mm_mul_ps(v, v);
	v.m128_f32[3]=0;

	v = _mm_hadd_ps(v, v);
    v = _mm_hadd_ps(v, v);

	v = _mm_sqrt_ss(v);

	return v.m128_f32[0];
}

Float3 Float3::Normalize(const Float3* val)
{
	__m128 v = _mm_loadu_ps((float*)val);

	__m128 lengthInvSq = _mm_mul_ps(v, v);
	lengthInvSq.m128_f32[3]=0;

	lengthInvSq = _mm_hadd_ps(lengthInvSq, lengthInvSq);
    lengthInvSq = _mm_hadd_ps(lengthInvSq, lengthInvSq);

	lengthInvSq = _mm_rsqrt_ps(lengthInvSq);

	v = _mm_mul_ps(v, lengthInvSq);

	return Float3(v.m128_f32[0], v.m128_f32[1], v.m128_f32[2]);
}