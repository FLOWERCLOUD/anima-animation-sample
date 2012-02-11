#include "Animation.h"
#include "Skeleton.h"
#include "math.h"

NodeAnimation::NodeAnimation( const aiNodeAnim& nodeAnim, int boneIndex )
	: aiNodeAnim( nodeAnim )
	, mBoneIndex( boneIndex )
{
}



Animation::Animation( aiAnimation* animation )
	: mAnimation( animation )
	, mTime( 0 )
	, mPlaybackSpeed( 1.f )
	, mPlaybackState( STOPPED )
{
	mDuration = static_cast<float>(
		mAnimation->mDuration * (mAnimation->mTicksPerSecond == 0 ? 1.0f : mAnimation->mTicksPerSecond)
	);
}


Animation::~Animation(void)
{
	delete mAnimation;
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

