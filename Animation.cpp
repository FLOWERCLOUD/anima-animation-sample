#include "Animation.h"
#include "Skeleton.h"


Animation::Animation( const aiAnimation* animation )
	: mAnimation( animation )
	, mPlayForward( true )
	, mTime( 0 )
{
	assert( mAnimation );
}


Animation::~Animation(void)
{
}

 


void Animation::Update( float dt )
{
	mTime += mPlayForward ? dt : -dt;

	if( mTime > mAnimation->mDuration )
		mPlayForward = false;
	else if( mTime < 0 )
		mPlayForward = true;
}

void Animation::EvaluatePose( Skeleton& targetSkeleton )
{
	for( unsigned int c = 0; c < mAnimation->mNumChannels; ++c )
	{	
		aiNodeAnim* na = mAnimation->mChannels[c];
		int boneIndex = targetSkeleton.getBoneIndex( na->mNodeName.data );
		if( boneIndex >= 0 )
		{
			float curTicks = mTime * mAnimation->mTicksPerSecond / 10.0;

			aiQuaternion rotation;
			evaluateChannel<aiQuatKey>( na->mRotationKeys, na->mNumRotationKeys, curTicks, rotation );

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