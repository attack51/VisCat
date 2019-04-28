#pragma once

#include "Float4MM.h"
#include "Side4.h"

#define WINGDING_VERTEX_MAX_COUNT 12

class VISCAT_LIB_API Winding
{
private:
	Float4MM m_vertices[WINGDING_VERTEX_MAX_COUNT*2];
public:
	Float4MM m_vertsFractions4[3];	//4*3=ÃÑ 12°³
	Side4 m_vertsSides4[3];			//4*3=ÃÑ 12°³
private:
	int m_readVertexStart;
	int m_readVertexCount;

	int m_writeVertexStart;
	int m_writeVertexCount;

public:
	inline Winding(const Float4MM* a, const Float4MM* b, const Float4MM* c);
	void SwapAndClear();

	inline Float4MM* ReadVertex(int index)
	{ return m_vertices + m_readVertexStart + index; }

	inline void WriteVertex(Float4MM* vertex)
	{
		m_vertices[m_writeVertexStart + m_writeVertexCount] = *vertex;
		++m_writeVertexCount;
	}

	bool IsInsideOfFrustum(int index);
	float GetVertFraction(int index);

	inline int VertexCount() {return m_readVertexCount;}
};