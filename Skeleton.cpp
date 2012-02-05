#include "Skeleton.h"
#include <cassert>
#include <set>
#include <map>
#include "aiScene.h"
#include "Debug.h"
#include "math.h"



using namespace std;

Skeleton::Skeleton(void)
{
}


Skeleton::~Skeleton(void)
{
}

int Skeleton::getBoneCount() const
{
	assert( mParents.size() == mTransforms.size() );
	return mParents.size();
}

Math::Matrix4x3 Skeleton::getWorldTransform( int bone ) const
{
	Math::Matrix4x3 result = mBindingTransforms[bone] * mTransforms[bone];
	int p = mParents[bone];

	while( p >= 0 )
	{
		result = result * mTransforms[p];
		p = mParents[p];
	}
	
	return result;

}

void Skeleton::setLocalTransform( int bone, const Math::Matrix4x3& transform )
{
	mTransforms[bone] = transform;
}

const std::vector<Math::Matrix4x3>& Skeleton::getLocalTransforms() const 
{
	return mTransforms;
}

