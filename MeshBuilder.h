#pragma once
#include "stdafx.h"
#include "Model.h"

class SkeletonBuilder;
class DataConverter;

class MeshBuilder
{
	const SkeletonBuilder& mSkeletonBuilder;
	const aiScene* mScene;

	typedef std::map<const aiMesh*, std::vector<DataConverter*> > ConverterMap;
	ConverterMap mDataConverters;

	void CreateDataConverters( ConverterMap& result ) const;
	int  GetVertexSize( const aiMesh* mesh ) const;
	void ReadTexture( std::vector<BYTE>& result, const char* fileName );

public:
	MeshBuilder( const aiScene* scene, const SkeletonBuilder& skeletonBuilder );
	~MeshBuilder();

	void BuildMeshes( std::vector<SkeletalModel::Mesh>& meshes );
};

