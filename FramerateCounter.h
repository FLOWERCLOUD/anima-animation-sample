#pragma once

/* Computes average framerate over configurable number of frames. Uses high performance counter */
class FramerateCounter
{
	static const int NUM_SAMPLES_IN_AVERAGE = 100;
	float mSamples[NUM_SAMPLES_IN_AVERAGE];	// delta time samples

	int mCurrentSample;
	int mSampleCount;

	LONGLONG mFrameStart;
	LONGLONG mTicksPerSecond;	// resolution of high performance counter

public:
	FramerateCounter(void);
	~FramerateCounter(void);

	void FrameStart();	// call this at frame start
	void FrameEnd();	// call this once frame is done rendering

	float GetAverage();	// returns average framerate over NUM_SAMPLES_IN_AVERAGE frames	
	float GetDelta();	// returns last delta time
};

