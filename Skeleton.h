#pragma once

#include "stdafx.h" 

struct aiScene;
struct aiNode;
struct aiBone;
class SkeletonFactory;

class Skeleton
{
	std::vector<int> mParents;
	std::vector<aiMatrix4x4> mTransforms;
	std::vector<aiMatrix4x4> mBindingTransforms;
	std::vector<std::string> mNodeNames;

public:
	Skeleton(void);
	~Skeleton(void);

	void addTransform( int parent, const aiMatrix4x4& transform, const aiMatrix4x4& bindingTransform, const std::string& name );

	const std::vector<aiMatrix4x4>& getLocalTransforms() const;
	const std::vector<std::string>& getBoneNames() const;
	const std::vector<int>& getParents() const;

	aiMatrix4x4 getWorldTransform( int bone ) const;
	void setLocalTransform( int bone, const aiMatrix4x4& transform );
	
	int getBoneIndex( const std::string& name ) const;
	int getBoneCount() const;

	friend class SkeletonFactory;

};

class SkeletonFactory
{
	struct SkeletonNode
	{
		const aiNode* node;
		const aiBone* bone;
		int parent;
		std::string name;
		bool used;
	};

public:
	static void findAnimatedNodes( const aiScene* scene, std::map<std::string, aiBone*>& result );
	static void flattenHierarchy( const aiNode* node, int parent, const std::map<std::string, aiBone*>& animatedNodes, std::vector<SkeletonNode>& result );
	static void markParents( std::vector<SkeletonNode>& hierarchy );

	static void filterHierarchy( std::vector<SkeletonNode>& hierarchy, Skeleton& result );

public:
	static Skeleton* extractSkeleton( const aiScene* scene );
};

