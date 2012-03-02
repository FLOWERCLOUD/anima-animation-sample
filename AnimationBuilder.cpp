#include "AnimationBuilder.h"
#include "SkeletonBuilder.h"
#include "Debug.h"

using namespace std;

AnimationBuilder::AnimationBuilder( const aiScene* scene, const SkeletonBuilder& skeletonBuilder )
	: mScene( scene )
	, mSkeletonBuilder( skeletonBuilder )
{
	
}


AnimationBuilder::~AnimationBuilder(void)
{
}

void AnimationBuilder::BuildAnimations( std::vector<Animation>& result )
{
	for( size_t a=0; a<mScene->mNumAnimations; ++a )
	{
		aiAnimation* aiAnim = mScene->mAnimations[a];
		const float ticksPerSecond = static_cast<float>( aiAnim->mTicksPerSecond == 0 ? 1.0f : aiAnim->mTicksPerSecond );

		result.push_back( Animation() );
		Animation& animation = result.back();
		animation.mDuration = static_cast<float>( aiAnim->mDuration * ticksPerSecond );

		for( unsigned int n=0; n<aiAnim->mNumChannels; ++n )
		{
			aiNodeAnim* nodeAnim = aiAnim->mChannels[n];
			string nodeName = string(nodeAnim->mNodeName.data);
			int nodeIndex = mSkeletonBuilder.GetNodeIndex( nodeName );

			if( nodeIndex > 0 )
			{
				animation.mAnimatedNodes.push_back( NodeAnimation() );
				NodeAnimation& na = animation.mAnimatedNodes.back();
				
				na.mBoneIndex = nodeIndex;
				convertKeys( na.mPositionKeys, nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, ticksPerSecond );
				convertKeys( na.mRotationKeys, nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, ticksPerSecond );
				convertKeys( na.mScalingKeys, nodeAnim->mScalingKeys, nodeAnim->mNumScalingKeys, ticksPerSecond );
			}
			else
			{
				DebugPrint( "Discarding animation of Bone '%s': The bone is not used in the skeleton\n", nodeName.c_str() );
			}
		}
	}
}
