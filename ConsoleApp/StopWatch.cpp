#include "stdafx.h"
#include "StopWatch.h"
#include "Winbase.h"

StopWatch::StopWatch()
{
	m_freq.LowPart = m_freq.HighPart = 0;
	m_start = m_freq;
	m_end = m_freq;
	m_fElapsedTime = 0;

	QueryPerformanceFrequency(&m_freq);
}

StopWatch::~StopWatch()
{
}

void StopWatch::Start()
{
	QueryPerformanceCounter(&m_start);
}

void StopWatch::End()
{
	QueryPerformanceCounter(&m_end);
	m_fElapsedTime = (m_end.QuadPart - m_start.QuadPart) / (float)m_freq.QuadPart;
}