#include "Model.h"
#include <iostream>
#include "RenderContext.h"
#include "DxErr.h"
#include "SkeletonBuilder.h"
#include "AnimationBuilder.h"
#include "MeshBuilder.h"
#include "Skeleton.h"
#include "Debug.h"
#include "PoseBuffer.h"

using namespace std;
SkeletalModel::SkeletalModel( const std::string& fileName )
	: mFileName( fileName )
	, mIsLoaded( false )
	, mCurrentAnimation( NULL )
	, mShaderTest( 0 )
	, mCurrentAnimationMethod( SAM_LINEAR_BLEND_SKINNING )
{
	mSkeletons[SAM_LINEAR_BLEND_SKINNING] = new Skeleton_Matrix43();
	mPoseBuffers[SAM_LINEAR_BLEND_SKINNING] = new PoseBufferGeneric<Skeleton_Matrix43>();

	mSkeletons[SAM_DUAL_QUATERNION] = new Skeleton_DualQuaternion();
	mPoseBuffers[SAM_DUAL_QUATERNION] = new PoseBufferGeneric<Skeleton_DualQuaternion>();

}


SkeletalModel::~SkeletalModel(void)
{
	mMeshes.clear();
}

void SkeletalModel::SetRoot( const aiVector3D& translation, const aiQuaternion& rotation )
{
	mSkeletons[mCurrentAnimationMethod]->SetLocalTransform( 0, translation, rotation, aiVector3D(1.f,1.f,1.f) );
}

void SkeletalModel::AcquireResources( RenderContext* context )
{
	for( unsigned int m=0; m<mMeshes.size(); ++m )
	{
		
		Mesh& mesh = mMeshes[m];
		context->Device()->CreateVertexDeclaration( &mesh.Data.mVertexElements[0], &mesh.mVertexDeclaration );

		// now create vertex buffer
		{
			DX_CHECK( context->Device()->CreateVertexBuffer( mesh.Data.mVertexData.size(), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mesh.mVertexBuffer, NULL ) );

			BYTE* vertexData;
			DX_CHECK( mesh.mVertexBuffer->Lock( 0, 0, reinterpret_cast<void**>( &vertexData ), 0 ) );
				memcpy( vertexData, &mesh.Data.mVertexData[0], mesh.Data.mVertexData.size() );
			DX_CHECK( mesh.mVertexBuffer->Unlock() );
		}


		// build index buffer
		{

			DX_CHECK( context->Device()->CreateIndexBuffer( mesh.Data.mIndexData.size(), D3DUSAGE_WRITEONLY, mesh.Data.mIndexFormat, D3DPOOL_DEFAULT, &mesh.mIndexBuffer, NULL ) );

			BYTE* indexData;
			DX_CHECK( mesh.mIndexBuffer->Lock( 0, 0, reinterpret_cast<void**>( &indexData ), 0 ) );
				memcpy( indexData, &mesh.Data.mIndexData[0], mesh.Data.mIndexData.size() );
			DX_CHECK( mesh.mIndexBuffer->Unlock() );

		}

		// load shaders
		{

			char maxBoneCountLBS[8], maxBoneCountDQ[8];
			sprintf_s( maxBoneCountLBS, sizeof(maxBoneCountLBS), "%d", mSkeletons[SAM_LINEAR_BLEND_SKINNING]->GetMaxBoneCount() );
			sprintf_s( maxBoneCountLBS, sizeof(maxBoneCountDQ), "%d", mSkeletons[SAM_DUAL_QUATERNION]->GetMaxBoneCount() );
			D3DXMACRO macros[] =
			{ 
				{ "MAX_BONES_PER_MESH_LBS", maxBoneCountLBS }, 
				{ "MAX_BONES_PER_MESH_DQ", maxBoneCountDQ },
				NULL 
			};

			LPD3DXBUFFER buf;			
			HRESULT hr = D3DXCreateEffectFromFile( context->Device(), "../Shaders/Model.fx", macros, NULL, NULL, NULL, &mesh.mEffect, &buf );

			if( FAILED(hr) && buf)
			{
				const char* text = reinterpret_cast<char*>( buf->GetBufferPointer() );
				DXTRACE_ERR( text, hr );
				buf->Release();
			}
		}

		// load textures
		{
			DX_CHECK( D3DXCreateTextureFromFile( context->Device(), "../Textures/frank_D.jpg", &mesh.mDiffuseMap ) );
			DX_CHECK( D3DXCreateTextureFromFile( context->Device(), "../Textures/frank_N.jpg", &mesh.mNormalMap ) );
		}
	}
}

void SkeletalModel::ReleaseResources( RenderContext* context )
{
	for( unsigned int m=0; m<SAM_COUNT; ++m )
	{
		delete mSkeletons[m];
		delete mPoseBuffers[m];
	}

	for( unsigned int m=0; m<mMeshes.size(); ++m )
	{
		Mesh& mesh = mMeshes[m];

		if( mesh.mEffect )
		{
			mesh.mEffect->Release();
			mesh.mEffect = NULL;
		}

		if( mesh.mIndexBuffer )
		{
			mesh.mIndexBuffer->Release();
			mesh.mIndexBuffer = NULL;
		}

		if( mesh.mVertexBuffer )
		{
			mesh.mVertexBuffer->Release();
			mesh.mVertexBuffer = NULL;
		}

		if( mesh.mVertexDeclaration )
		{
			mesh.mVertexDeclaration->Release();
			mesh.mVertexDeclaration = NULL;
		}

		if( mesh.mDiffuseMap )
		{
			mesh.mDiffuseMap->Release();
			mesh.mDiffuseMap = NULL;
		}

		if( mesh.mNormalMap )
		{
			mesh.mNormalMap->Release();
			mesh.mNormalMap = NULL;
		}
	}
}

bool SkeletalModel::Load( RenderContext* context )
{
	// Create an instance of the Importer class
	Assimp::Importer modelImporter;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll 
	// propably to request more postprocessing than we do in this example.
	const aiScene* scene = modelImporter.ReadFile( mFileName, 
		aiProcess_CalcTangentSpace       | 
		aiProcess_Triangulate            |
		aiProcess_JoinIdenticalVertices  |
		aiProcess_ConvertToLeftHanded	|	
		aiProcess_SortByPType);

	// If the import failed, report it
	if( !scene)
	{
		std::cout << modelImporter.GetErrorString() << std::endl;
		MessageBox( NULL,  modelImporter.GetErrorString(), "import failed", MB_OK );

		return false;
	}

	/*
	for( int m=0; m<scene->mNumMaterials; ++m )
	{
		aiMaterial* mat = scene->mMaterials[m];
		for( int p=0; p<mat->mNumProperties; ++p )
		{
			aiMaterialProperty* prop = mat->mProperties[p];
			DebugPrint( "%s\n", prop->mKey.data );
		}
	}
	*/

	// import skeleton, animation and mesh data
	{
		SkeletonBuilder skeletonBuilder( scene );
		AnimationBuilder animationBuilder( scene, skeletonBuilder );
		MeshBuilder meshBuilder( scene, skeletonBuilder );

		// build all skeletons
		skeletonBuilder.BuildSkeleton( dynamic_cast<Skeleton_Matrix43*>( mSkeletons[SAM_LINEAR_BLEND_SKINNING] ) );
		skeletonBuilder.BuildSkeleton( dynamic_cast<Skeleton_DualQuaternion*>( mSkeletons[SAM_DUAL_QUATERNION] ) );
		

		animationBuilder.BuildAnimations( mAnimations );
		meshBuilder.BuildMeshes( mMeshes );
	}
	
	// upload to gpu
	AcquireResources( context );

	return true;

}

void SkeletalModel::PlayAnimation( unsigned int animationIndex, float playbackSpeed )
{
	if( animationIndex >= 0 && animationIndex < mAnimations.size() )
	{
		mCurrentAnimation = mAnimations[animationIndex];
		mCurrentAnimation->Play( playbackSpeed );
		mAnimationPaused = false;
	}
}

void SkeletalModel::PauseAnimation()
{
	mAnimationPaused = true;
}

void SkeletalModel::ToggleAnimationPlayback()
{
	mAnimationPaused = !mAnimationPaused;
}

void SkeletalModel::ToggleShaderTest()
{
	mShaderTest = mShaderTest==0 ? 1 : 0;
}



void SkeletalModel::Render( RenderContext* context )
{
	context->Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	context->Device()->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE);
	context->Device()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE);

	for( unsigned int m=0; m<mMeshes.size(); ++m )
	{
		const Mesh& mesh = mMeshes[m];
		D3DXHANDLE hTechnique = mesh.mEffect->GetTechniqueByName( "Model" );
		D3DXHANDLE hViewProjection = mesh.mEffect->GetParameterBySemantic( NULL, "VIEWPROJECTION" );

		D3DXHANDLE hLightDirection = mesh.mEffect->GetParameterBySemantic( NULL, "LIGHTDIRECTION" );
		D3DXHANDLE hBoneTransforms = mesh.mEffect->GetParameterBySemantic( NULL, "BONE_TRANSFORMS" );
		D3DXHANDLE hShaderTest = mesh.mEffect->GetParameterBySemantic( NULL, "SHADER_TEST" );
		D3DXHANDLE hDiffuseMap = mesh.mEffect->GetParameterBySemantic( NULL, "DIFFUSE_MAP" );
		D3DXHANDLE hNormalMap = mesh.mEffect->GetParameterBySemantic( NULL, "NORMAL_MAP" );

		
		DX_CHECK( context->Device()->SetVertexDeclaration( mesh.mVertexDeclaration ) );
		DX_CHECK( context->Device()->SetStreamSource( 0, mesh.mVertexBuffer, 0, mesh.Data.mVertexSize ) );
		DX_CHECK( context->Device()->SetIndices( mesh.mIndexBuffer ) );

		Math::Matrix viewProjection = context->GetViewMatrix() * context->GetProjectionMatrix();
		DX_CHECK( mesh.mEffect->SetMatrix( hViewProjection, &viewProjection.data ) );

		D3DXVECTOR4 lightDirection = Math::Vector( 0.5, 1, 0 ).Normal();
		DX_CHECK( mesh.mEffect->SetVector( hLightDirection, &lightDirection ) );

		{
			PoseBufferInterface& poseBuffer = *mPoseBuffers[mCurrentAnimationMethod];
			const float* poseData =  reinterpret_cast<const float*>( poseBuffer[0] );
			DX_CHECK( mesh.mEffect->SetFloatArray( hBoneTransforms, poseData, poseBuffer.Size() / sizeof(float) ) );
		}

		DX_CHECK( mesh.mEffect->SetInt( hShaderTest, mShaderTest ) );

		DX_CHECK( mesh.mEffect->SetTexture( hDiffuseMap, mesh.mDiffuseMap ) );
		DX_CHECK( mesh.mEffect->SetTexture( hNormalMap, mesh.mNormalMap ) );

		DX_CHECK( mesh.mEffect->SetTechnique( hTechnique ) );
	
		UINT cPasses;
		DX_CHECK( mesh.mEffect->Begin( &cPasses, 0 ) );
		for( unsigned int iPass = 0; iPass < cPasses; iPass++ )
		{
			DX_CHECK( mesh.mEffect->BeginPass( iPass ) );
			DX_CHECK( context->Device()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, mesh.Data.mVertexCount, 0, mesh.Data.mTriangleCount ) );
			DX_CHECK( mesh.mEffect->EndPass() );
		}

		DX_CHECK( mesh.mEffect->End() );
	}

	DX_CHECK( context->Device()->SetStreamSource( 0, NULL, 0, 0 ) );
	DX_CHECK( context->Device()->SetIndices( NULL ) );
}

void SkeletalModel::Update( float dt )
{
	SkeletonInterface& skeleton = *mSkeletons[mCurrentAnimationMethod];
	PoseBufferInterface& poseBuffer = *mPoseBuffers[mCurrentAnimationMethod];

	if( mCurrentAnimation && !mAnimationPaused )
	{
		mCurrentAnimation->Update( dt );
		mCurrentAnimation->EvaluatePose( skeleton );	
	}

	for( int i=0; i<skeleton.GetBoneCount(); ++i )
	{
		skeleton.GetWorldTransform( i, poseBuffer );
	}

}
 