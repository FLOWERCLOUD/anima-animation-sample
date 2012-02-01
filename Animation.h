#pragma once
#include "stdafx.h"

class Skeleton;

class Animation
{
	const aiAnimation* mAnimation;
	float mTime;

	bool mPlayForward;

	void Interpolate( aiQuaternion& result, const aiQuaternion& a, const aiQuaternion& b, float c )
	{
		a.Interpolate( result, a, b, c );
	}

	void Interpolate( aiVector3D& result, const aiVector3D& a, const aiVector3D& b, float c )
	{
		result = (1.f-c)*a + c * b;
	}

	template<typename T, typename R>
	void evaluateChannel( T* data, int dataSize, float t, R& result )
	{
		int i=0;
		for( i=0; i<dataSize ; ++i )
		{
			if( t < data[i].mTime ) break;
		}

		int l = max( 0, i-1 );
		int r = min( i, dataSize-1 );

		double tDiff = l==r ? 1.0 : data[r].mTime - data[l].mTime;
		float c = static_cast<float>( (t-data[l].mTime)  / tDiff );

		c = max( 0, min( 1, c ) );
		Interpolate( result, data[l].mValue, data[r].mValue, c );
	}

public:
	Animation( const aiAnimation* animation );
	~Animation(void);

	void Update( float dt );
	void EvaluatePose( Skeleton& targetSkeleton );
};

