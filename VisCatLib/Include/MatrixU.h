#pragma once

//외부에서 setting할때 주는 matrix.
//4*16=64 byte이면 ok

#include "dllexport.h"

class Float3;
class VISCAT_LIB_API MatrixU
{
public:
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;

	inline MatrixU() {}
	 MatrixU(float _m11, float _m12, float _m13, float _m14,
			float _m21, float _m22, float _m23, float _m24,
			float _m31, float _m32, float _m33, float _m34,
			float _m41, float _m42, float _m43, float _m44);
	
	static MatrixU Identity();
	static MatrixU Mul(const MatrixU* lhs, const MatrixU* rhs);
	static MatrixU ViewMatrix(const Float3* eye, const Float3* at, const Float3* up);
	static MatrixU ProjectMatrix(float fovy, float aspect, float zn, float zf);
};