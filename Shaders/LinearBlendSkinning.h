#ifndef __LINEAR_BLEND_SKINNING_H__
#define __LINEAR_BLEND_SKINNING_H__

#include "Skinning.h"

float3x4 GetBoneMatrix( int boneIndex )
{
	return float3x4(
		BoneTransforms[boneIndex*3 + 0],
		BoneTransforms[boneIndex*3 + 1],
		BoneTransforms[boneIndex*3 + 2]
	);
}

float3x4 GetBlendedMatrix( uint4 boneIndices, float4 boneWeights )
{
	return
		GetBoneMatrix( boneIndices.x ) * boneWeights.x + 
		GetBoneMatrix( boneIndices.y ) * boneWeights.y + 
		GetBoneMatrix( boneIndices.z ) * boneWeights.z + 
		GetBoneMatrix( boneIndices.w ) * boneWeights.w;
}

#endif