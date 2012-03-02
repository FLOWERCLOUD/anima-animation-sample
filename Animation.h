#pragma once
#include "stdafx.h"
#include "Serialization.h"
#include "math.h"

class Skeleton;
class AnimationBuilder;

template< typename KeyData >
struct Key
{
	KeyData mValue;
	float mTime;
};

struct NodeAnimation : public Serialization::StreamSerializable
{
	NodeAnimation();
	int mBoneIndex;

	std::vector< Key<Math::Vector> > mPositionKeys;
	std::vector< Key<Math::Vector> > mScalingKeys;
	std::vector< Key<Math::Quaternion> > mRotationKeys;

	virtual bool ToStream( std::ostream& stream );
	virtual bool FromStream( std::istream& stream );
};

class Animation : public Serialization::StreamSerializable
{
	friend class AnimationBuilder;

	std::vector<NodeAnimation> mAnimatedNodes;

	enum PlaybackState { PLAYING_FORWARD, PLAYING_BACKWARD, STOPPED };

	float mTime;
	float mDuration;
	float mPlaybackSpeed;

	PlaybackState mPlaybackState;

	template<typename T, typename R>
	void evaluateChannel( const std::vector<T>& data, float t, R& result );

public:
	Animation();
	~Animation(void);

	void Play( float playbackSpeed );
	void Update( float dt );

	template< class SkeletonType >
	void EvaluatePose( SkeletonType& targetSkeleton );

	virtual bool ToStream( std::ostream& stream );
	virtual bool FromStream( std::istream& stream );
};

template<typename T, typename R>
void Animation::evaluateChannel( const std::vector<T>& data, float t, R& result )
{
	const int keyCount = static_cast<int>( data.size() );

	int i=0;
	while( i < keyCount )
	{
		if( t <= data[i].mTime ) 
			break;
		++i;
	}

	int l = max( 0, i-1 );
	int r = min( i, keyCount-1 );

	double tDiff = l==r ? 1.0 : data[r].mTime - data[l].mTime;
	float c = static_cast<float>( (t-data[l].mTime)  / tDiff );

	c = max( 0, min( 1, c ) );

	result = R::Interpolate( data[l].mValue, data[r].mValue, c );
}

template< class SkeletonType >
void Animation::EvaluatePose( SkeletonType& targetSkeleton )
{
	for( unsigned int c = 0; c < mAnimatedNodes.size(); ++c )
	{	
		const NodeAnimation& na = mAnimatedNodes[c];
		assert( na.mBoneIndex >= 0 );

		Math::Quaternion rotation;
		evaluateChannel( na.mRotationKeys, mTime, rotation );
		rotation.Normalize();

		Math::Vector position;
		evaluateChannel( na.mPositionKeys, mTime, position );

		Math::Vector scale;
		evaluateChannel( na.mScalingKeys, mTime, scale );

		targetSkeleton.SetLocalTransform( na.mBoneIndex, position, rotation, scale );
	}
}

