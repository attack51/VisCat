#pragma once

//외부에서 setting된 matrix를 MatrixMM로 변환해서 사용
//16byte정렬해서 사용

#include "dllexport.h"
#include <pmmintrin.h>

class MatrixU;
class Float3;
class Float4MM;

class _MM_ALIGN16 VISCAT_LIB_API MatrixMM
{
private:
	__m128 row[4];
public:

	inline MatrixMM() {}
	MatrixMM(	float _m11, float _m12, float _m13, float _m14,
				float _m21, float _m22, float _m23, float _m24,
				float _m31, float _m32, float _m33, float _m34,
				float _m41, float _m42, float _m43, float _m44);
	MatrixMM(const MatrixU& matrix);
	MatrixMM(const MatrixMM& matrix);
	MatrixMM(const __m128& _row0, const __m128& _row1, const __m128& _row2, const __m128& _row3);

	//Getter
	const float M11() { return row[0].m128_f32[0]; }
	const float M12() { return row[0].m128_f32[1]; }
	const float M13() { return row[0].m128_f32[2]; }
	const float M14() { return row[0].m128_f32[3]; }

	const float M21() { return row[1].m128_f32[0]; }
	const float M22() { return row[1].m128_f32[1]; }
	const float M23() { return row[1].m128_f32[2]; }
	const float M24() { return row[1].m128_f32[3]; }

	const float M31() { return row[2].m128_f32[0]; }
	const float M32() { return row[2].m128_f32[1]; }
	const float M33() { return row[2].m128_f32[2]; }
	const float M34() { return row[2].m128_f32[3]; }

	const float M41() { return row[3].m128_f32[0]; }
	const float M42() { return row[3].m128_f32[1]; }
	const float M43() { return row[3].m128_f32[2]; }
	const float M44() { return row[3].m128_f32[3]; }

	//Setter
	void SetM11(float value) { row[0].m128_f32[0] = value; }
	void SetM12(float value) { row[0].m128_f32[1] = value; }
	void SetM13(float value) { row[0].m128_f32[2] = value; }
	void SetM14(float value) { row[0].m128_f32[3] = value; }

	void SetM21(float value) { row[1].m128_f32[0] = value; }
	void SetM22(float value) { row[1].m128_f32[1] = value; }
	void SetM23(float value) { row[1].m128_f32[2] = value; }
	void SetM24(float value) { row[1].m128_f32[3] = value; }
	
	void SetM31(float value) { row[2].m128_f32[0] = value; }
	void SetM32(float value) { row[2].m128_f32[1] = value; }
	void SetM33(float value) { row[2].m128_f32[2] = value; }
	void SetM34(float value) { row[2].m128_f32[3] = value; }
	
	void SetM41(float value) { row[3].m128_f32[0] = value; }
	void SetM42(float value) { row[3].m128_f32[1] = value; }
	void SetM43(float value) { row[3].m128_f32[2] = value; }
	void SetM44(float value) { row[3].m128_f32[3] = value; }

	static MatrixMM Identity();
	static MatrixMM Mul(const MatrixMM* lhs, const MatrixMM* rhs);
	
	static Float4MM TransformPoint(const MatrixMM* mat, const Float3* point);
	static Float4MM TransformDirection(const MatrixMM* mat, const Float3* direction);
	static Float4MM Transform(const MatrixMM* mat, const Float4MM* vec);

	static void TransformPointArray(const MatrixMM* mat, 
									const Float3* pointArray, 
									Float4MM* resultArray, 
									int count);

	static void TransformDirectionArray(const MatrixMM* mat, 
										const Float3* directionArray, 
										Float4MM* resultArray, 
										int count);

	static void TransformArray(	const MatrixMM* mat, 
								const Float4MM* vecArray, 
								Float4MM* resultArray, 
								int count);
	
private:
	static Float4MM Transform(const MatrixMM* mat, const __m128& vec);
};