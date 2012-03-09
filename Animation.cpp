#include "Animation.h"
#include "Skeleton.h"
#include "math.h"

NodeAnimation::NodeAnimation( )
	: mBoneIndex( -1 )
{
}

bool NodeAnimation::ToStream( std::ostream& stream )
{
	Serialization::ToStream( mBoneIndex, stream );
	Serialization::ToStream( mPositionKeys, stream );
	Serialization::ToStream( mScalingKeys, stream );
	Serialization::ToStream( mRotationKeys, stream );
	return stream.good();
}

bool NodeAnimation::FromStream( std::istream& stream )
{
	Serialization::FromStream( stream, mBoneIndex );
	Serialization::FromStream( stream, mPositionKeys );
	Serialization::FromStream( stream, mScalingKeys );
	Serialization::FromStream( stream, mRotationKeys );
	return true;
}

Animation::Animation()
	: mTime( 0 )
	, mPlaybackSpeed( 1.f )
	, mPlaybackState( STOPPED )
	, mDuration( 0 )
{
}


Animation::~Animation(void)
{
}

 
void Animation::Play( float playbackSpeed )
{
	mPlaybackSpeed = playbackSpeed;
	mPlaybackState = PLAYING_FORWARD;
}

void Animation::Update( float dt )
{
	if( mPlaybackState != STOPPED )
		mTime += (mPlaybackState == PLAYING_FORWARD ? dt : -dt) * mPlaybackSpeed;

	if( mTime > mDuration )
	{
		mTime = mDuration;
		mPlaybackState = PLAYING_BACKWARD;
	}
	else if( mTime < 0 )
	{
		mTime = 0.f; 
		mPlaybackState = PLAYING_FORWARD;
	}
}

bool Animation::ToStream( std::ostream& stream )
{
	Serialization::ToStream( mAnimatedNodes.size(), stream );
	for( unsigned int n=0; n<mAnimatedNodes.size(); ++n )
		mAnimatedNodes[n].ToStream( stream );
	
	Serialization::ToStream( mTime, stream );
	Serialization::ToStream( mDuration, stream );
	Serialization::ToStream( mPlaybackSpeed, stream );
	Serialization::ToStream( mPlaybackState, stream );

	return stream.good();
}

bool Animation::FromStream( std::istream& stream )
{
	unsigned int numAnimatedNodes = Serialization::FromStream<unsigned int>( stream );
	mAnimatedNodes.resize( numAnimatedNodes );

	for( unsigned int n=0; n<mAnimatedNodes.size(); ++n )
		mAnimatedNodes[n].FromStream( stream );
	
	Serialization::FromStream( stream, mTime );
	Serialization::FromStream( stream, mDuration );
	Serialization::FromStream( stream, mPlaybackSpeed );
	Serialization::FromStream( stream, mPlaybackState );
	return true;
}

