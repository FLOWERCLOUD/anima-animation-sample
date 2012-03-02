#pragma once
#include "stdafx.h"
#include "math.h"
#include "Serialization.h"

class PoseBufferInterface : public Serialization::StreamSerializable
{
public:
	virtual int Size() const = 0;
	virtual void* operator[]( int boneIndex ) = 0;
};

template< typename BoneTransform >
class PoseBufferGeneric : public PoseBufferInterface
{
	friend class SkeletonBuilder;

	std::vector<BoneTransform> mBuffer;

public:
	PoseBufferGeneric()
	{
		mBuffer.reserve( SkeletonTraits<BoneTransform>::MaxBoneCount );
	}

	void* operator[]( int boneIndex )
	{
		return &mBuffer[boneIndex];
	}

	int Size() const
	{
		return mBuffer.size() *  sizeof(BoneTransform);
	}

	virtual bool ToStream( std::ostream& stream )
	{
		Serialization::ToStream( mBuffer, stream );
		return stream.good();
	}

	virtual bool FromStream( std::istream& stream )
	{
		Serialization::FromStream( stream, mBuffer );
		return true;
	}

};

