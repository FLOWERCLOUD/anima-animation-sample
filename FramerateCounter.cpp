#include "StdAfx.h"
#include "FramerateCounter.h"

FramerateCounter::FramerateCounter(void)
	: mSampleCount(0)
	, mCurrentSample(0)
{
	// init samples to 0
	memset( mSamples, 0x0, sizeof(mSamples) );

	// get timer resolution
	LARGE_INTEGER ticksPerSec = { 0 };
    QueryPerformanceFrequency( &ticksPerSec );
	mTicksPerSecond = ticksPerSec.QuadPart;
}


FramerateCounter::~FramerateCounter(void)
{
}

void FramerateCounter::FrameStart()
{
	// start timing
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	mFrameStart = li.QuadPart;
}

void FramerateCounter::FrameEnd()
{
	// compute delta time and store in samples array
	LARGE_INTEGER curTick;
	QueryPerformanceCounter( &curTick );

	float dt = float(double(curTick.QuadPart - mFrameStart)/mTicksPerSecond);
	mSamples[mCurrentSample++] = dt;

	mCurrentSample = mCurrentSample % NUM_SAMPLES_IN_AVERAGE;
	mSampleCount = min( mSampleCount+1, NUM_SAMPLES_IN_AVERAGE );
}

float FramerateCounter::GetDelta()
{
	int i = mCurrentSample-1;
	if( i<0 ) 
		i = NUM_SAMPLES_IN_AVERAGE-1;

	return mSamples[i];
}

float FramerateCounter::GetAverage()
{
	float t = 0;
	for( int i=0; i<mSampleCount; ++i )
	{
		t += mSamples[i];
	}

	return t / mSampleCount;
}
