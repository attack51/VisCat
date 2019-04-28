#include "stdafx.h"
#include "MatrixMM.h"
#include "Float3.h"
#include "Float4MM.h"
#include "MatrixU.h"
#include "VisCatMath.h"

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)

#include <ppl.h>

MatrixMM::MatrixMM(	float _m11, float _m12, float _m13, float _m14,
					float _m21, float _m22, float _m23, float _m24,
					float _m31, float _m32, float _m33, float _m34,
					float _m41, float _m42, float _m43, float _m44)
{
	 row[0] = _mm_set_ps(_m14, _m13, _m12, _m11);
	 row[1] = _mm_set_ps(_m24, _m23, _m22, _m21);
	 row[2] = _mm_set_ps(_m34, _m33, _m32, _m31);
	 row[3] = _mm_set_ps(_m44, _m43, _m42, _m41);
}

MatrixMM::MatrixMM(const MatrixU& matrix)
{
	row[0] = _mm_loadu_ps((float*)&matrix);
	row[1] = _mm_loadu_ps(((float*)&matrix)+4);
	row[2] = _mm_loadu_ps(((float*)&matrix)+8);
	row[3] = _mm_loadu_ps(((float*)&matrix)+12);
}

MatrixMM::MatrixMM(const MatrixMM& matrix)
{
	row[0] = matrix.row[0];
	row[1] = matrix.row[1];
	row[2] = matrix.row[2];
	row[3] = matrix.row[3];
}

MatrixMM::MatrixMM(const __m128& _row0, const __m128& _row1, const __m128& _row2, const __m128& _row3)
{
	row[0] = _row0;
	row[1] = _row1;
	row[2] = _row2;
	row[3] = _row3;
}

MatrixMM MatrixMM::Identity()
{
	__m128 row0 = _mm_set_ps(0, 0, 0, 1); 
	__m128 row1 = _mm_set_ps(0, 0, 1, 0); 
	__m128 row2 = _mm_set_ps(0, 1, 0, 0); 
	__m128 row3 = _mm_set_ps(1, 0, 0, 0); 

	return MatrixMM(row0, row1, row2, row3);
}

MatrixMM MatrixMM::Mul(const MatrixMM* lhs, const MatrixMM* rhs)
{
	__m128 tempRow0, tempRow1, tempRow2, tempRow3;
	__m128 resultRow[4];

	for(int i=0; i<4; ++i)
	{
		tempRow0 = _mm_mul_ps(_mm_shuffle_ps(lhs->row[i], lhs->row[i], _MM_SHUFFLE(0,0,0,0)), rhs->row[0]);
		tempRow1 = _mm_mul_ps(_mm_shuffle_ps(lhs->row[i], lhs->row[i], _MM_SHUFFLE(1,1,1,1)), rhs->row[1]);
		tempRow2 = _mm_mul_ps(_mm_shuffle_ps(lhs->row[i], lhs->row[i], _MM_SHUFFLE(2,2,2,2)), rhs->row[2]);
		tempRow3 = _mm_mul_ps(_mm_shuffle_ps(lhs->row[i], lhs->row[i], _MM_SHUFFLE(3,3,3,3)), rhs->row[3]);

		tempRow0 = _mm_add_ps(tempRow0, tempRow1);
		tempRow2 = _mm_add_ps(tempRow2, tempRow3);
		resultRow[i] = _mm_add_ps(tempRow0, tempRow2);
	}

	return MatrixMM(resultRow[0], resultRow[1], resultRow[2], resultRow[3]);
}

Float4MM MatrixMM::Transform(const MatrixMM* mat, const __m128& vec)
{
	__m128 tempRow0, tempRow1, tempRow2, tempRow3;

	tempRow0 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0,0,0,0)), mat->row[0]);
	tempRow1 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1,1,1,1)), mat->row[1]);
	tempRow2 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2,2,2,2)), mat->row[2]);
	tempRow3 = _mm_mul_ps(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3,3,3,3)), mat->row[3]);

	tempRow0 = _mm_add_ps(tempRow0, tempRow1);
	tempRow2 = _mm_add_ps(tempRow2, tempRow3);

	__m128 result = _mm_add_ps(tempRow0, tempRow2);
	return Float4MM(result);
}


Float4MM MatrixMM::TransformPoint(const MatrixMM* mat, const Float3* point)
{
	__m128 v = _mm_set_ps(1, point->z,point->y,point->x);
	return Transform(mat, v);
}

Float4MM MatrixMM::TransformDirection(const MatrixMM* mat, const Float3* direction)
{
	__m128 v = _mm_set_ps(0, direction->z,direction->y,direction->x);
	return Transform(mat, v);
}

Float4MM MatrixMM::Transform(const MatrixMM* mat, const Float4MM* vec)
{
	return Transform(mat, vec->xyzw);
}

void MatrixMM::TransformArray(	const MatrixMM* mat, 
								const Float4MM* vecArray, 
								Float4MM* resultArray, 
								int count)
{
	for(int i=0; i<count; ++i)
	{
		resultArray[i] = Transform(mat, vecArray+i);
	}
}


void MatrixMM::TransformPointArray(	const MatrixMM* mat, 
									const Float3* pointArray, 
									Float4MM* resultArray, 
									int count)
{
	for(int i=0; i<count; ++i)
	{
		resultArray[i] = TransformPoint(mat, pointArray+i);
	}
}

void MatrixMM::TransformDirectionArray(	const MatrixMM* mat, 
										const Float3* directionArray, 
										Float4MM* resultArray, 
										int count)
{
	for(int i=0; i<count; ++i)
	{
		resultArray[i] = TransformDirection(mat, directionArray+i);
	}
}
