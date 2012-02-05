#pragma once

#include "stdafx.h" 
#include "math.h"

struct aiScene;
struct aiNode;
struct aiBone;
class SkeletonBuilder;

class Skeleton
{
	friend class SkeletonBuilder;

public:
	static const int MAX_BONES_PER_MESH = 62;

private:
	std::vector<int> mParents;
	std::vector<Math::Matrix4x3> mTransforms;
	std::vector<Math::Matrix4x3> mBindingTransforms;

public:
	Skeleton(void);
	~Skeleton(void);

	const std::vector<Math::Matrix4x3>& getLocalTransforms() const;
	const std::vector<int>& getParents() const;

	Math::Matrix4x3 getWorldTransform( int bone ) const;
	void setLocalTransform( int bone, const Math::Matrix4x3& transform );
	
	int getBoneCount() const;
};



