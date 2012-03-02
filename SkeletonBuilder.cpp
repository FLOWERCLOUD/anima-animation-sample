#include "SkeletonBuilder.h"
#include "Skeleton.h"

using namespace std;

SkeletonBuilder::SkeletonBuilder( const aiScene* scene )
	: mScene( scene )
{
	// find all animated nodes first
	findAnimatedNodes( mAnimatedNodes );

	// flatten node hierarchy into array
	flattenHierarchy( mScene->mRootNode, -1, mAnimatedNodes, mFullHierarchy );

	// mark parents of animated nodes as 'used' to make sure they are not discarded in the next step
	markParents( mFullHierarchy );

	// last step: filter unused nodes
	filterHierarchy( mFullHierarchy, mReducedHierarchy );
}

void SkeletonBuilder::findAnimatedNodes( map<string, aiBone*>& result ) const
{
	assert( mScene );
	assert( result.empty() );

	// find all bones that influence the meshes first
	for( unsigned int m=0; m<mScene->mNumMeshes; ++m )
	{
		aiMesh* mesh = mScene->mMeshes[m];
		assert( mesh );

		for( unsigned int b=0; b<mesh->mNumBones; ++b )
		{
			aiBone* bone = mesh->mBones[b];
			assert( bone );

			result[bone->mName.data] = bone;	
		}
	}
}


void SkeletonBuilder::flattenHierarchy( const aiNode* node, int parent, const map<string, aiBone*>& animatedNodes, vector<SkeletonNode>& result ) const
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

void  SkeletonBuilder::markParents( std::vector<SkeletonNode>& hierarchy ) const
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

void SkeletonBuilder::filterHierarchy( const std::vector<SkeletonNode>& fullHierarchy, std::vector<SkeletonNode>& result ) const
{
	// TODO: collapse nodes if possible
	map<int, int> nodeMapping;
	nodeMapping[-1] = -1;

	for( unsigned int i=0; i<fullHierarchy.size(); ++i )
	{
		SkeletonNode n = fullHierarchy[i];
		if( n.used )
		{
			n.parent = nodeMapping[n.parent];
			result.push_back( n );
			
			nodeMapping[i] = result.size()-1;
		}
	}
}

int SkeletonBuilder::GetNodeIndex( std::string nodeName ) const
{
	for( size_t i=0; i<mReducedHierarchy.size(); ++i )
	{
		if( mReducedHierarchy[i].name == nodeName )
			return i;
	}
	return -1;
}

void SkeletonBuilder::buildSkeleton( SkeletonGeneric<Math::Matrix3x4>* result, PoseBufferGeneric<Math::Matrix3x4>* poseBuffer ) const
{	
	// sanity check on skeleton data
	assert( result->mBindingTransforms.size() == result->mParents.size() && result->mParents.size() == result->mTransforms.size() );

	// add all nodes in reduced hierarchy to skeleton
	poseBuffer->mBuffer.resize( mReducedHierarchy.size() );
	for( size_t i=0; i<mReducedHierarchy.size(); ++i )
	{
		const SkeletonNode& n = mReducedHierarchy[i];

		result->mParents.push_back( n.parent );
		result->mTransforms.push_back( n.node->mTransformation );
		result->mBindingTransforms.push_back( n.bone ? n.bone->mOffsetMatrix : aiMatrix4x4() );
	}
}

void SkeletonBuilder::buildSkeleton( SkeletonGeneric<Math::DualQuaternion>* result, PoseBufferGeneric<Math::DualQuaternion>* poseBuffer ) const
{
	// sanity check on skeleton data
	assert( result->mBindingTransforms.size() == result->mParents.size() && result->mParents.size() == result->mTransforms.size() );

	// add all nodes in reduced hierarchy to skeleton
	poseBuffer->mBuffer.resize( mReducedHierarchy.size() );
	for( size_t i=0; i<mReducedHierarchy.size(); ++i )
	{
		const SkeletonNode& n = mReducedHierarchy[i];

		{
			aiQuaternion rotation;
			aiVector3D position;
			aiVector3D scale( 1, 1, 1 );
			n.node->mTransformation.DecomposeNoScaling( rotation, position );

			result->mTransforms.push_back( 
				Math::DualQuaternion( Math::Vector(position), Math::Quaternion(rotation), Math::Vector(scale) ) 
			);
		}
		
		result->mParents.push_back( n.parent );

		Math::DualQuaternion dq;
		if( n.bone )
		{
			aiQuaternion rotation;
			aiVector3D position;
			aiVector3D scale( 1, 1, 1 );
			n.bone->mOffsetMatrix.DecomposeNoScaling( rotation, position );

			dq = Math::DualQuaternion( position, rotation, scale );
		}

		result->mBindingTransforms.push_back( dq );
	}
}
