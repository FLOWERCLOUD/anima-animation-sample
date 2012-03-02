#pragma once
#include "stdafx.h"
#include "Animation.h"

class SkeletonBuilder;

class AnimationBuilder
{
	const aiScene* mScene;
	const SkeletonBuilder& mSkeletonBuilder;

	template<class SourceKeyType, class ResultKeyType>
	void convertKeys( std::vector<Key<ResultKeyType> >& target, const SourceKeyType* source, unsigned int count, float ticksPerSecond )
	{
		target.resize( count );
		for( unsigned int k=0; k<count; ++k )
		{
			target[k].mTime = static_cast<float>( source[k].mTime * ticksPerSecond );
			target[k].mValue = ResultKeyType( source[k].mValue );
		}
	}

public:
	AnimationBuilder( const aiScene* scene, const SkeletonBuilder& skeletonBuilder );
	~AnimationBuilder(void);

	void BuildAnimations( std::vector<Animation>& result );
};

