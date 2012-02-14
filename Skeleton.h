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
	virtual const char* GetShaderTechnique() const = 0;
};

template< class BoneTransform >
class SkeletonGeneric : public SkeletonInterface
{
	friend class SkeletonBuilder;

	std::vector<int> mParents;
	std::vector<BoneTransform> mTransforms;
	std::vector<BoneTransform> mBindingTransforms;

	virtual BoneTransform Concatenate( const BoneTransform& first, const BoneTransform& second ) const
	{
		return second * first;
	}

public:
	typedef BoneTransform BoneTransformType;

	virtual int GetMaxBoneCount() const
	{
		return SkeletonTraits<BoneTransform>::MaxBoneCount;
	}

	virtual const char* GetShaderTechnique() const 
	{
		return SkeletonTraits<BoneTransform>::ShaderTechnique;
	}

	virtual int GetBoneCount() const
	{
		return mTransforms.size();
	}

	const std::vector<BoneTransform>& GetLocalTransforms() const
	{
		return mTransforms;
	}

	
	const std::vector<BoneTransform>& GetBindingTransforms() const
	{
		return mBindingTransforms;
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

template< typename BoneTransform >
struct SkeletonTraits
{
	static const int MaxBoneCount = 0;
};

template<>
struct SkeletonTraits<Math::Matrix3x4>
{
	static const int MaxBoneCount = 66;
	static const char* ShaderTechnique;
};

template<>
struct SkeletonTraits<Math::DualQuaternion>
{
	static const int MaxBoneCount = 100;
	static const char* ShaderTechnique;

};