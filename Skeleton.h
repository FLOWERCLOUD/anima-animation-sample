#pragma once

#include "stdafx.h" 
#include "math.h"
#include "PoseBuffer.h"

struct aiScene;
struct aiNode;
struct aiBone;
class SkeletonBuilder;

class SkeletonInterface
{
public:
	virtual int GetBoneCount() const = 0;
	virtual int GetMaxBoneCount() const = 0;
	virtual void SetLocalTransform( int bone,  const aiVector3D& translation, const aiQuaternion& rotation, const aiVector3D& scale ) = 0;
	virtual void GetWorldTransform( int bone, PoseBufferInterface& poseBuffer ) const = 0;
};

template< class BoneTransform, int MaxBoneCount >
class SkeletonGeneric : public SkeletonInterface
{
	friend class SkeletonBuilder;

	std::vector<int> mParents;
	std::vector<BoneTransform> mTransforms;
	std::vector<BoneTransform> mBindingTransforms;

	virtual BoneTransform Concatenate( const BoneTransform& first, const BoneTransform& second ) const
	{
		return first * second;
	}

public:
	typedef BoneTransform BoneTransformType;
	static const int MAX_BONE_COUNT = MaxBoneCount;

	virtual int GetMaxBoneCount() const
	{
		return MaxBoneCount;
	}

	virtual int GetBoneCount() const
	{
		return mTransforms.size();
	}

	const std::vector<BoneTransform>& GetLocalTransforms() const
	{
		return mTransforms;
	}

	virtual void SetLocalTransform( int bone,  const aiVector3D& translation, const aiQuaternion& rotation, const aiVector3D& scale )
	{
		mTransforms[bone] = BoneTransform( translation, rotation, scale );	
	}


	const std::vector<int>& GetParents() const
	{
		return mParents;
	}
	void GetWorldTransform( int bone, PoseBufferInterface& poseBuffer ) const
	{
		BoneTransform result = Concatenate( mBindingTransforms[bone], mTransforms[bone] );
		int p = mParents[bone];

		while( p >= 0 )
		{
			result = Concatenate( result, mTransforms[p] );
			p = mParents[p];
		}
	
		memcpy( poseBuffer[bone], &result, sizeof(BoneTransform) );
	}
};


typedef SkeletonGeneric< Math::DualQuaternion, 62 > Skeleton_DualQuaternion;

class Skeleton_Matrix34 : public SkeletonGeneric< Math::Matrix3x4, 62 >
{
public:
	virtual Math::Matrix3x4 Concatenate( const Math::Matrix3x4& first, const Math::Matrix3x4& second ) const
	{
		return second * first;
	}
};
