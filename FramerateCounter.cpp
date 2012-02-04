#include "StdAfx.h"
#include "FramerateCounter.h"

FramerateCounter::FramerateCounter(void)
	: m_SampleCount(0)
	, m_CurrentSample(0)
{
	// init samples to 0
	memset( m_Samples, 0x0, sizeof(m_Samples) );

	// get timer resolution
	LARGE_INTEGER ticksPerSec = { 0 };
    QueryPerformanceFrequency( &ticksPerSec );
	m_TicksPerSecond = ticksPerSec.QuadPart;
}


FramerateCounter::~FramerateCounter(void)
{
}

void FramerateCounter::FrameStart()
{
	// start timing
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	m_FrameStart = li.QuadPart;
}

void FramerateCounter::FrameEnd()
{
	// compute delta time and store in samples array
	LARGE_INTEGER curTick;
	QueryPerformanceCounter( &curTick );

	float dt = float(double(curTick.QuadPart - m_FrameStart)/m_TicksPerSecond);
	m_Samples[m_CurrentSample++] = dt;

	m_CurrentSample = m_CurrentSample % NUM_SAMPLES_IN_AVERAGE;
	m_SampleCount = min( m_SampleCount+1, NUM_SAMPLES_IN_AVERAGE );
}

float FramerateCounter::GetDelta()
{
	int i = m_CurrentSample-1;
	if( i<0 ) 
		i = NUM_SAMPLES_IN_AVERAGE-1;

	return m_Samples[i];
}

float FramerateCounter::GetAverage()
{
	float t = 0;
	for( int i=0; i<m_SampleCount; ++i )
	{
		t += m_Samples[i];
	}

	return t / m_SampleCount;
}
