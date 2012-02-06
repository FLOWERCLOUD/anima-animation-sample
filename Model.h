#pragma once
#include "stdafx.h"
#include <string>
#include "DisplayList.h"
#include "Skeleton.h"
#include "DataConverters.h"
#include "Animation.h"

class SkeletonBuilder;
class MeshBuilder;
struct aiScene;

class Model : public DisplayList::Node
{
	friend class MeshBuilder;

	struct MeshData
	{
		int mVertexCount;
		int mVertexSize;

		int mTriangleCount;
		D3DFORMAT mIndexFormat;

		std::vector<BYTE> mVertexData;
		std::vector<BYTE> mIndexData;
		std::vector<D3DVERTEXELEMENT9> mVertexElements;

		MeshData() : mVertexCount(0), mVertexSize(0), mTriangleCount(0), mIndexFormat(D3DFMT_UNKNOWN) {};
	};

	struct Mesh
	{
		IDirect3DVertexBuffer9* mVertexBuffer;
		IDirect3DVertexDeclaration9* mVertexDeclaration;
		IDirect3DIndexBuffer9* mIndexBuffer;

		ID3DXEffect* mEffect;

		MeshData Data;

		Mesh() : mVertexBuffer(NULL), mVertexDeclaration(NULL), mIndexBuffer(NULL), mEffect(NULL) {}
		
	};

	Skeleton mSkeleton;
	std::vector<Math::Matrix4x3> mPoseBuffer;
	std::vector<Mesh> mMeshes;
	std::vector<Animation*> mAnimations;

	Animation* mCurrentAnimation;
	int mAnimationPaused;

	int mShaderTest;

	int CreateDataConverters( aiMesh* mesh, SkeletonBuilder* skeletonBuilder, std::vector<DataConverter*>& result );

public:
	Model( const std::string& fileName );
	~Model(void);

	bool Load( RenderContext* context );
	void AcquireResources( RenderContext* context );
	void ReleaseResources( RenderContext* context );


	void SetRoot( const Math::Matrix& root );
	void PlayAnimation( unsigned int animationIndex, float playbackSpeed = 0.f );
	void PauseAnimation();
	void ToggleAnimationPlayback();
	
	void ToggleShaderTest();


	void Render( RenderContext* context );
	void Update( float dt );

	bool mIsLoaded;
	std::string mFileName;
};

