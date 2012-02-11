#pragma once

#include "stdafx.h" 
#include "math.h"

struct aiScene;
struct aiNode;
struct aiBone;
class SkeletonBuilder;
class PoseBuffer;

template< class BoneTransform, int MaxBoneCount >
class SkeletonGeneric
{
	friend class SkeletonBuilder;

	std::vector<int> mParents;
	std::vector<BoneTransform> mTransforms;
	std::vector<BoneTransform> mBindingTransforms;

	

	virtual BoneTransform Concatenate( const BoneTransform& first, const BoneTransform& second ) const = 0;

public:
	static const int MAX_BONE_COUNT = MaxBoneCount;

	int GetBoneCount() const
	{
		return mTransforms.size();
	}

	const std::vector<BoneTransform>& GetLocalTransforms() const
	{
		return mTransforms;
	}

	void SetLocalTransform( int bone,  const aiVector3D& translation, const aiQuaternion& rotation, const aiVector3D& scale )
	{
		mTransforms[bone] = BoneTransform( translation, rotation, scale );	
	}


	const std::vector<int>& GetParents() const
	{
		return mParents;
	}


	BoneTransform GetWorldTransform( int bone ) const
	{
		BoneTransform result = Concatenate( mBindingTransforms[bone], mTransforms[bone] );
		int p = mParents[bone];

		while( p >= 0 )
		{
			result = Concatenate( result, mTransforms[p] );
			p = mParents[p];
		}
	
		return result;
	}
};

class Skeleton_MatrixBased : public SkeletonGeneric< Math::Matrix4x3, 62 >
{
	Math::Matrix4x3 Concatenate( const Math::Matrix4x3& first, const Math::Matrix4x3& second ) const
	{
		return first * second;
	}
};



