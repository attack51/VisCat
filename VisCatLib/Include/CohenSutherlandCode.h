#pragma once

#include "dllexport.h"
class Float4MM;

//Cohen–Sutherland code
#define CS_LEFT_INDEX 0
#define CS_RIGHT_INDEX 1
#define CS_BOTTOM_INDEX 2
#define CS_TOP_INDEX 3
#define CS_NEAR_INDEX 4
#define CS_FAR_INDEX 5

#define CS_CODE_LEFT	(0x01<<CS_LEFT_INDEX)
#define CS_CODE_RIGHT	(0x01<<CS_RIGHT_INDEX)
#define CS_CODE_BOTTOM	(0x01<<CS_BOTTOM_INDEX)
#define CS_CODE_TOP		(0x01<<CS_TOP_INDEX)
#define CS_CODE_NEAR	(0x01<<CS_NEAR_INDEX)
#define CS_CODE_FAR		(0x01<<CS_FAR_INDEX)
#define CS_CODE_ALL		0x3f

bool VISCAT_LIB_API GetCohenSutherlandCodeSSE(Float4MM* pPoints, unsigned char* pIntersectCsCode);
bool VISCAT_LIB_API GetCohenSutherlandCodeScalar(Float4MM* pPoints, unsigned char* pIntersectCsCode);