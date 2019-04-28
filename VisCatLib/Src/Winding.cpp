#include "stdafx.h"
#include "Winding.h"
#include <assert.h>

inline Winding::Winding(const Float4MM* a, const Float4MM* b, const Float4MM* c)
{
	m_readVertexStart=0;
	m_readVertexCount=3;

	m_writeVertexStart=WINGDING_VERTEX_MAX_COUNT;
	m_writeVertexCount=0;

	m_vertices[0] = *a;
	m_vertices[1] = *b;
	m_vertices[2] = *c;
}

void Winding::SwapAndClear()
{
	int temp=m_readVertexStart;
	m_readVertexStart = m_writeVertexStart;
	m_writeVertexStart=temp;

	m_readVertexCount = m_writeVertexCount;
	m_writeVertexCount=0;
}

bool Winding::IsInsideOfFrustum(int index)
{
	assert(index<m_readVertexCount);

	int arrayIndex = index/4;
	int componentIndex = index%4;
	return m_vertsSides4[arrayIndex].IsInside(componentIndex);
}

float Winding::GetVertFraction(int index)
{
	assert(index<m_readVertexCount);

	int arrayIndex = index/4;
	int componentIndex = index%4;
	return m_vertsFractions4[arrayIndex].Get(componentIndex);	
}