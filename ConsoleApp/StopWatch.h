#pragma once 

#include <Windows.h>

class StopWatch
{
public:
	StopWatch();
	virtual ~StopWatch();

public:
	void Start();
	void End();
	const float GetElapsedSecond() const { return m_fElapsedTime;}
	const float GetElapsedMilliSecond() const { return m_fElapsedTime*1000; }

protected:
	LARGE_INTEGER m_freq, m_start, m_end;
	float m_fElapsedTime;
};