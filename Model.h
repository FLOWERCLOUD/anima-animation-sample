#pragma once
#include "stdafx.h"
#include <string>
#include "DisplayList.h"
#include "Skeleton.h"
#include "DataConverters.h"
#include "Animation.h"

struct aiScene;

class Model : public DisplayList::Node
{
	Assimp::Importer mModelImporter;
	Skeleton* mSkeleton;

	struct Mesh
	{
		IDirect3DVertexBuffer9* m_pVertexBuffer;
		IDirect3DVertexDeclaration9* m_pVertexDeclaration;
		IDirect3DIndexBuffer9* m_pIndexBuffer;

		ID3DXEffect* m_pEffect;
		int m_VertexSize;
		int m_VertexCount;
		int m_TriangleCount;

		aiMesh* sourceMesh;
	};

	std::vector<Mesh> m_Meshes;
	std::vector<Animation*> m_Animations;

	int CreateDataConverters( aiMesh* mesh, Skeleton* skeleton, std::vector<DataConverter*>& result );

public:
	Model( const std::string& fileName );
	~Model(void);

	bool load( RenderContext* context );

	void Render( RenderContext* context );

	void Update( float dt );
 
	bool mIsLoaded;
	std::string mFileName;

	aiScene* mScene;
};

