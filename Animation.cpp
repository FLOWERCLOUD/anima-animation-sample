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
	, mPlayForward( true )
	, mTime( 0 )
{
	mDuration = static_cast<float>(
		mAnimation->mDuration * (mAnimation->mTicksPerSecond == 0 ? 1.0f : mAnimation->mDuration)
	);
}


Animation::~Animation(void)
{
	delete mAnimation;
}

 


void Animation::Update( float dt )
{
	mTime += mPlayForward ? dt : -dt;

	if( mTime > mDuration )
	{
		mTime = mDuration;
		mPlayForward = false;
	}
	else if( mTime < 0 )
	{
		mTime = 0.f; 
		mPlayForward = true;
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

		Math::Matrix4x3 m( rotation.GetMatrix(), position );			
		targetSkeleton.setLocalTransform( na->mBoneIndex, m );
	}
}