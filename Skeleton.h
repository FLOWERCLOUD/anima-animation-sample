#pragma once

#include "stdafx.h" 
#include "math.h"
#include "PoseBuffer.h"
#include "Serialization.h"

struct aiScene;
struct aiNode;
struct aiBone;
class SkeletonBuilder;

class SkeletonInterface : public Serialization::StreamSerializable
{
public:
	virtual int GetBoneCount() const = 0;
	virtual int GetMaxBoneCount() const = 0;
	virtual void SetLocalTransform( int bone,  const Math::Vector& translation, const Math::Quaternion& rotation, const Math::Vector& scale ) = 0;
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


	virtual void SetLocalTransform( int bone,  const Math::Vector& translation, const Math::Quaternion& rotation, const Math::Vector& scale )
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

	bool ToStream( std::ostream& stream )
	{
		Serialization::ToStream( mParents, stream );
		Serialization::ToStream( mTransforms, stream );
		Serialization::ToStream( mBindingTransforms, stream );
		return stream.good();
	}

	bool FromStream( std::istream& stream )
	{
		Serialization::FromStream( stream, mParents );
		Serialization::FromStream( stream, mTransforms );
		Serialization::FromStream( stream, mBindingTransforms );
		return true;
	}
};

// specialized version of concatenate for dual quaternions with approximate antipodality correction
template<>
Math::DualQuaternion SkeletonGeneric<Math::DualQuaternion>::Concatenate( const Math::DualQuaternion& first, const Math::DualQuaternion& second ) const
{
	float s = first.real.Dot( second.real ) > 0 ? 1.0f : -1.0f;
	return second * (first*s);
}

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