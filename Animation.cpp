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

void Animation::EvaluatePose( Skeleton& targetSkeleton )
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

		aiMatrix3x3 r = rotation.GetMatrix();
	//	r[0][0] *= scale.x; r[1][1] *= scale.y; r[2][2] *= scale.z;

		Math::Matrix4x3 m( r, position );
		targetSkeleton.setLocalTransform( na->mBoneIndex, m );

	}
}