#include "Animation.h"
#include "Skeleton.h"
#include "math.h"


Animation::Animation( const aiAnimation* animation )
	: mAnimation( animation )
	, mPlayForward( true )
	, mTime( 0 )
{
	mDuration = static_cast<float>(
		mAnimation->mDuration * (mAnimation->mTicksPerSecond == 0 ? 1.0f : mAnimation->mDuration)
	);
	assert( mAnimation );
}


Animation::~Animation(void)
{
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
		aiNodeAnim* na = mAnimation->mChannels[c];
		int boneIndex = targetSkeleton.getBoneIndex( na->mNodeName.data );
		assert( boneIndex >= 0 );
		if( boneIndex >= 0 )
		{
			float curTicks = static_cast<float>( mTime * (mAnimation->mTicksPerSecond==0 ? 1.0f : mAnimation->mTicksPerSecond) );

			aiQuaternion rotation;
			evaluateChannel( na->mRotationKeys, na->mNumRotationKeys, curTicks, rotation );
			rotation.Normalize();

			aiVector3D position;
			evaluateChannel( na->mPositionKeys, na->mNumPositionKeys, curTicks, position );

			aiMatrix4x4 m( rotation.GetMatrix() );			
			m.a4 = position.x;
			m.b4 = position.y;
			m.c4 = position.z;

			targetSkeleton.setLocalTransform( boneIndex, m );
		}
	}

}