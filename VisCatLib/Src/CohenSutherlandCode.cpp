#include "stdafx.h"
#include "CohenSutherlandCode.h"

#include "Float4MM.h"
#include "sse_preset.h"

//sse3
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <ppl.h>
#include <ppltasks.h>

#define GetLeftCsCode(vertex, code) if (vertex.X() < -vertex.W())	code|=CS_CODE_LEFT;
#define GetRightCsCode(vertex, code) if (vertex.X() > vertex.W())	code|=CS_CODE_RIGHT;
#define GetTopCsCode(vertex, code) if (vertex.Y() > vertex.W())		code|=CS_CODE_TOP;
#define GetBottomCsCode(vertex, code) if (vertex.Y() < -vertex.W()) code|=CS_CODE_BOTTOM;
#define GetNearCsCode(vertex, code) if (vertex.Z() < 0)				code|=CS_CODE_NEAR;
#define GetFarCsCode(vertex, code) if (vertex.Z() > vertex.W())		code|=CS_CODE_FAR;

#define OutsideRejectCheck(code) if(code==0x0f) return false;

bool GetCohenSutherlandCodeSSE(Float4MM* pPoints, unsigned char* pIntersectCsCode)
{
	(*pIntersectCsCode)=0;

	__m128 x3, y3, z3, w3;
	Float4MM::ExtractXYZW3(pPoints, &x3, &y3, &z3, &w3);
	__m128 negW3 = _mm_sub_ps(g_zero4, w3);

	unsigned char codes[6];
	codes[CS_LEFT_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmplt_ps(x3, negW3));
	OutsideRejectCheck(codes[CS_LEFT_INDEX]);

	codes[CS_RIGHT_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmpgt_ps(x3, w3));
	OutsideRejectCheck(codes[CS_RIGHT_INDEX]);

	codes[CS_BOTTOM_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmplt_ps(y3, negW3));
	OutsideRejectCheck(codes[CS_BOTTOM_INDEX]);

	codes[CS_TOP_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmpgt_ps(y3, w3));
	OutsideRejectCheck(codes[CS_TOP_INDEX]);

	codes[CS_NEAR_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmplt_ps(z3, g_zero4));
	OutsideRejectCheck(codes[CS_NEAR_INDEX]);

	codes[CS_FAR_INDEX] = (unsigned char)_mm_movemask_ps(_mm_cmpgt_ps(z3, w3));
	OutsideRejectCheck(codes[CS_FAR_INDEX]);

	for(int i=0; i<6; ++i)
	{
		if(codes[i]>0) (*pIntersectCsCode) |= (0x01<<i);
	}
	return true;
}

bool GetCohenSutherlandCodeScalar(Float4MM* pPoints, unsigned char* pIntersectCsCode)
{
	BYTE outAllCsCode=CS_CODE_ALL;
	(*pIntersectCsCode)=0;
	for(int i=0; i<3; ++i)
	{
		BYTE CsCode=0;
		GetLeftCsCode(pPoints[i], CsCode);
		GetRightCsCode(pPoints[i], CsCode);
		GetTopCsCode(pPoints[i], CsCode);
		GetBottomCsCode(pPoints[i], CsCode);
		GetNearCsCode(pPoints[i], CsCode);
		GetFarCsCode(pPoints[i], CsCode);
		
		outAllCsCode &= CsCode;
		(*pIntersectCsCode) |= CsCode;
	}
	
	return outAllCsCode == 0;
}