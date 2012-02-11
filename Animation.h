#pragma once
#include "stdafx.h"

class Skeleton;
class AnimationBuilder;

struct NodeAnimation : public aiNodeAnim
{
	NodeAnimation( const aiNodeAnim& nodeAnim, int boneIndex );
	int mBoneIndex;
};

class Animation
{
	friend class AnimationBuilder;
	aiAnimation* mAnimation;

	enum PlaybackState { PLAYING_FORWARD, PLAYING_BACKWARD, STOPPED };

	float mTime;
	float mDuration;
	float mPlaybackSpeed;

	PlaybackState mPlaybackState;

	template<typename T, typename R>
	void evaluateChannel( T* data, int dataSize, float t, R& result );

public:
	Animation( aiAnimation* animation );
	~Animation(void);

	void Play( float playbackSpeed );
	void Update( float dt );

	template< class SkeletonType >
	void EvaluatePose( SkeletonType& targetSkeleton );
};

template<typename T, typename R>
void Animation::evaluateChannel( T* data, int dataSize, float t, R& result )
{
	int i=0;
	for( i=0; i<dataSize ; ++i )
	{
		if( t <= data[i].mTime ) break;
	}

	int l = max( 0, i-1 );
	int r = min( i, dataSize-1 );

	double tDiff = l==r ? 1.0 : data[r].mTime - data[l].mTime;
	float c = static_cast<float>( (t-data[l].mTime)  / tDiff );

	c = max( 0, min( 1, c ) );

	Assimp::Interpolator<R> interpolator;
	interpolator( result, data[l].mValue, data[r].mValue, c );
}

template< class SkeletonType >
void Animation::EvaluatePose( SkeletonType& targetSkeleton )
{
	for( unsigned int c = 0; c < mAnimation->mNumChannels; ++c )
	{	
		NodeAnimation* na = reinterpret_cast<NodeAnimation*>( mAnimation->mChannels[c] );
		assert( na->mBoneIndex >= 0 );
		
		float curTicks = static_cast<float>( mTime * (mAnimation->mTicksPerSecond==0 ? 1.0f : mAnimation->mTicksPerSecond) );

		aiQuaternion rotation;
		evaluateChannel( na->mRotationKeys, na->mNumRotationKeys, curTicks, rotation );
		rotation.Normalize();

		aiVector3D position;
		evaluateChannel( na->mPositionKeys, na->mNumPositionKeys, curTicks, position );

		aiVector3D scale;
		evaluateChannel( na->mScalingKeys, na->mNumScalingKeys, curTicks, scale );

		targetSkeleton.SetLocalTransform( na->mBoneIndex, position, rotation, scale );

	}
}

