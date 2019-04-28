#include "stdafx.h"
#include "MatrixU.h"
#include "Float3.h"
#include <math.h>

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

MatrixU::MatrixU(	float _m11, float _m12, float _m13, float _m14,
				float _m21, float _m22, float _m23, float _m24,
				float _m31, float _m32, float _m33, float _m34,
				float _m41, float _m42, float _m43, float _m44)
{
	m11=_m11; m12=_m12; m13=_m13; m14=_m14;
	m21=_m21; m22=_m22; m23=_m23; m24=_m24;
	m31=_m31; m32=_m32; m33=_m33; m34=_m34;
	m41=_m41; m42=_m42; m43=_m43; m44=_m44;
}

MatrixU MatrixU::Identity()
{
	return MatrixU(	1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
}

MatrixU MatrixU::Mul(const MatrixU* lhs, const MatrixU* rhs)
{
	__m128 tempRow0, tempRow1, tempRow2, tempRow3;
	MatrixU result;

	__m128 rhsRow[4];
	for(int i=0; i<4; ++i)
	{
		rhsRow[i] = _mm_loadu_ps(((float*)rhs)+(i*4));	
	}

	for(int i=0; i<4; ++i)
	{
		__m128 lhsRow = _mm_loadu_ps(((float*)lhs)+(i*4));	

		tempRow0 = _mm_mul_ps(_mm_shuffle_ps(lhsRow, lhsRow, _MM_SHUFFLE(0,0,0,0)), rhsRow[0]);
		tempRow1 = _mm_mul_ps(_mm_shuffle_ps(lhsRow, lhsRow, _MM_SHUFFLE(1,1,1,1)), rhsRow[1]);
		tempRow2 = _mm_mul_ps(_mm_shuffle_ps(lhsRow, lhsRow, _MM_SHUFFLE(2,2,2,2)), rhsRow[2]);
		tempRow3 = _mm_mul_ps(_mm_shuffle_ps(lhsRow, lhsRow, _MM_SHUFFLE(3,3,3,3)), rhsRow[3]);

		tempRow0 = _mm_add_ps(tempRow0, tempRow1);
		tempRow2 = _mm_add_ps(tempRow2, tempRow3);
		
		_mm_storeu_ps(((float*)&result)+(i*4), _mm_add_ps(tempRow0, tempRow2));
	}

	return result;
}

MatrixU MatrixU::ViewMatrix(const Float3* eye, const Float3* at, const Float3* up)
{
	Float3 dir(at->x - eye->x, at->y - eye->y, at->z - eye->z);
	Float3 zaxis = Float3::Normalize(&dir);
	Float3 right = Float3::Cross(up, &zaxis);
	Float3 xaxis = Float3::Normalize(&right);
	Float3 yaxis = Float3::Cross(&zaxis, &xaxis);
    
	float a=(Float3::Dot(&zaxis, eye));
	a=-a;
	return MatrixU(	xaxis.x, yaxis.x, zaxis.x, 0,
					xaxis.y, yaxis.y, zaxis.y, 0,
					xaxis.z, yaxis.z, zaxis.z, 0,
					-Float3::Dot(&xaxis, eye), -Float3::Dot(&yaxis, eye), -Float3::Dot(&zaxis, eye), 1);
}

MatrixU MatrixU::ProjectMatrix(float fovy, float aspect, float zn, float zf)
{
	float yScale = 1/tanf(fovy/2);
	float xScale = yScale / aspect;
	float invZfSubZn = 1/(zf-zn);

	return MatrixU(	xScale, 0, 0, 0,
					0, yScale, 0, 0,
					0, 0, zf*invZfSubZn, 1,
					0, 0, -zn*zf*invZfSubZn,0);
}