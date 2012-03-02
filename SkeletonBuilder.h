#pragma once
#include "stdafx.h"
#include "Skeleton.h"

class Skeleton;

class SkeletonBuilder
{
	struct SkeletonNode
	{
		const aiNode* node;
		const aiBone* bone;
		int parent;
		std::string name;
		bool used;
	};

	std::map<std::string, aiBone*> mAnimatedNodes;
	std::vector<SkeletonNode> mFullHierarchy;
	std::vector<SkeletonNode> mReducedHierarchy;

	const aiScene* mScene;

	void findAnimatedNodes( std::map<std::string, aiBone*>& result ) const;
	void flattenHierarchy( const aiNode* node, int parent, const std::map<std::string, aiBone*>& animatedNodes, std::vector<SkeletonNode>& result ) const;
	void markParents( std::vector<SkeletonNode>& hierarchy ) const;
	void filterHierarchy( const std::vector<SkeletonNode>& fullHierarchy, std::vector<SkeletonNode>& result ) const;

	void buildSkeleton( SkeletonGeneric<Math::Matrix3x4>* result, PoseBufferGeneric<Math::Matrix3x4>* poseBuffer ) const;
	void buildSkeleton( SkeletonGeneric<Math::DualQuaternion>* result, PoseBufferGeneric<Math::DualQuaternion>* poseBuffer ) const;
public:
	SkeletonBuilder( const aiScene* scene );
	
	template< class BoneTransform > 
	void BuildSkeleton( SkeletonInterface* result, PoseBufferInterface* poseBuffer ) const
	{
		buildSkeleton( 
			dynamic_cast< SkeletonGeneric<BoneTransform>* >( result ),
			dynamic_cast< PoseBufferGeneric<BoneTransform>* >( poseBuffer )
		);
	}

	int GetNodeIndex( std::string nodeName ) const;
};
