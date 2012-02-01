#include "Skeleton.h"
#include <cassert>
#include <set>
#include <map>
#include "aiScene.h"


using namespace std;

Skeleton::Skeleton(void)
{
}


Skeleton::~Skeleton(void)
{
}

void Skeleton::addTransform( int parent, const aiMatrix4x4& transform, const aiMatrix4x4& bindingTransform, const std::string& name )
{
	assert( mTransforms.size() == mParents.size() );
	assert( parent < static_cast<int>( mTransforms.size() ) && parent >= -1 );

	aiMatrix4x4 inverseBindingTransform( bindingTransform );
	inverseBindingTransform.Inverse();

	mTransforms.push_back( transform );
	mBindingTransforms.push_back( inverseBindingTransform );
	mParents.push_back( parent );
	mNodeNames.push_back( name );
}

aiMatrix4x4 Skeleton::getWorldTransform( int bone ) const
{
	aiMatrix4x4 result = mBindingTransforms[bone] * mTransforms[bone];
	int p = mParents[bone];

	while( p >= 0 )
	{
		result = mTransforms[p] * result;
		p = mParents[p];
	}

	return result;

}

void Skeleton::setLocalTransform( int bone, const aiMatrix4x4& transform )
{
	mTransforms[bone] = transform;
}

int Skeleton::getBoneIndex( const string& name ) const
{
	vector<string>::const_iterator it = std::find( mNodeNames.begin(), mNodeNames.end(), name );
	return it != mNodeNames.end() ? it - mNodeNames.begin() : -1;
}

int Skeleton::getBoneCount() const
{
	assert( mParents.size() == mTransforms.size() );
	return mParents.size();
}

void SkeletonFactory::findAnimatedNodes( const aiScene* scene, map<string, aiBone*>& result )
{
	assert( scene );
	assert( result.empty() );

	// find all bones that influence the meshes first
	for( unsigned int m=0; m<scene->mNumMeshes; ++m )
	{
		aiMesh* mesh = scene->mMeshes[m];
		assert( mesh );

		for( unsigned int b=0; b<mesh->mNumBones; ++b )
		{
			aiBone* bone = mesh->mBones[b];
			assert( bone );

			result[bone->mName.data] = bone;	
		}
	}
}


void SkeletonFactory::flattenHierarchy( const aiNode* node, int parent, const map<string, aiBone*>& animatedNodes, vector<SkeletonNode>& result )
{
	if( !node )
		return;

	string nodeName = string( node->mName.data );
	map<string, aiBone*>::const_iterator itBone = animatedNodes.find( nodeName );
	bool isAnimated = itBone != animatedNodes.end();

	SkeletonNode _n =
	{
		node,
		isAnimated ? itBone->second : NULL,
		parent,
		nodeName,
		isAnimated
	};

	result.push_back( _n );
	parent = result.size()-1;

	for( unsigned int i=0; i<node->mNumChildren; ++i )
		flattenHierarchy( node->mChildren[i], parent, animatedNodes, result );
}

void  SkeletonFactory::markParents( std::vector<SkeletonNode>& hierarchy )
{
	for( unsigned int i=0; i<hierarchy.size(); ++i )
	{
		SkeletonNode& currentNode = hierarchy[i];
		if( currentNode.used )
		{
			char p = currentNode.parent;
			while( p >= 0 )
			{
				SkeletonNode& n = hierarchy[p];
				if( n.used ) break;

				n.used = true;
				p = n.parent;
			}
		}
	}
}

void SkeletonFactory::filterHierarchy( std::vector<SkeletonNode>& hierarchy, Skeleton& result )
{
	// TODO: combine nodes if possible
	map<int, int> nodeMapping;
	nodeMapping[-1] = -1;

	for( unsigned int i=0; i<hierarchy.size(); ++i )
	{
		SkeletonNode& n = hierarchy[i];
		if( n.used )
		{
			result.addTransform(  nodeMapping[n.parent], n.node->mTransformation, n.bone ? n.bone->mOffsetMatrix : aiMatrix4x4(), n.name );
			nodeMapping[i] = result.getBoneCount()-1;
		}
	}
}


Skeleton* SkeletonFactory::extractSkeleton( const aiScene* scene )
{
	// find animated nodes first
	map<string, aiBone*> animatedNodes;
	findAnimatedNodes( scene, animatedNodes );

	// now flatten hierarchy into array and mark all animated nodes
	std::vector<SkeletonNode> hierarchy;
	flattenHierarchy( scene->mRootNode, -1, animatedNodes, hierarchy );

	// now mark all parents of animated nodes as used
	markParents( hierarchy );

	// last step: filter out unused nodes and fill in skeleton
	Skeleton* result = new Skeleton();
	filterHierarchy( hierarchy, *result );


	return result;
}