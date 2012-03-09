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

bool SkeletalModel::MeshData::ToStream( std::ostream& stream )
{
	Serialization::ToStream( mVertexSize, stream );
	Serialization::ToStream( mVertexCount, stream );
	Serialization::ToStream( mTriangleCount, stream );
	Serialization::ToStream( mIndexFormat, stream );

	Serialization::ToStream( mVertexData, stream );
	Serialization::ToStream( mIndexData, stream );
	Serialization::ToStream( mVertexElements, stream );

	Serialization::ToStream( mAlbedoMap, stream );
	Serialization::ToStream( mNormalMap, stream );

	return stream.good();
}

bool SkeletalModel::MeshData::FromStream( std::istream& stream )
{
	Serialization::FromStream( stream, mVertexSize );
	Serialization::FromStream( stream, mVertexCount );
	Serialization::FromStream( stream, mTriangleCount );
	Serialization::FromStream( stream, mIndexFormat );

	Serialization::FromStream( stream, mVertexData );
	Serialization::FromStream( stream, mIndexData );
	Serialization::FromStream( stream, mVertexElements );

	Serialization::FromStream( stream, mAlbedoMap );
	Serialization::FromStream( stream, mNormalMap );

	return true;
}

SkeletalModel::SkeletalModel( const std::string& fileName )
	: mFileName( fileName )
	, mIsLoaded( false )
	, mCurrentAnimation( NULL )
	, mShaderTest( 0 )
	, mCurrentAnimationMethod( SAM_LINEAR_BLEND_SKINNING )
{
	mSkeletons[SAM_LINEAR_BLEND_SKINNING] = new SkeletonGeneric<Math::Matrix3x4>();
	mPoseBuffers[SAM_LINEAR_BLEND_SKINNING] = new PoseBufferGeneric<Math::Matrix3x4>();

	mSkeletons[SAM_DUAL_QUATERNION] = new SkeletonGeneric<Math::DualQuaternion>();
	mPoseBuffers[SAM_DUAL_QUATERNION] = new PoseBufferGeneric<Math::DualQuaternion>();
}


SkeletalModel::~SkeletalModel(void)
{
	mMeshes.clear();
}

void SkeletalModel::SetRoot( const Math::Vector& translation, const Math::Quaternion& rotation )
{
	mSkeletons[mCurrentAnimationMethod]->SetLocalTransform( 0, translation, rotation, Math::Vector(1.f,1.f,1.f) );
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

		// load textures
		{
			DWORD albedoMapFilter = D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER | D3DX_FILTER_SRGB_IN | D3DX_FILTER_SRGB_OUT;
			DWORD normalMapFilter = D3DX_FILTER_LINEAR;

			DX_CHECK( D3DXCreateTextureFromFileInMemoryEx( context->Device(), &mesh.Data.mAlbedoMap[0], mesh.Data.mAlbedoMap.size(), 
				D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, albedoMapFilter,
				D3DX_DEFAULT, 0, NULL, NULL, &mesh.mDiffuseMap ) );

			DX_CHECK( D3DXCreateTextureFromFileInMemoryEx( context->Device(), &mesh.Data.mNormalMap[0], mesh.Data.mNormalMap.size(), 
				D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, normalMapFilter,
				D3DX_DEFAULT, 0, NULL, NULL, &mesh.mNormalMap ) );
		}

		// load shaders
		{
			unsigned int maxFloats = 0;
			for( unsigned int m=0; m<SAM_COUNT; ++m )
				maxFloats = max( maxFloats, mPoseBuffers[m]->Size() / sizeof(float) );

			char maxFloats4s[8];
			sprintf_s( maxFloats4s, sizeof(maxFloats4s), "%d", maxFloats / 4 );
			D3DXMACRO macros[] =
			{ 
				{ "MAX_SKINNING_VECTORS", maxFloats4s },
#ifdef DEBUG
				{ "DEBUG", "1" },
#endif
				NULL 
			};

			LPD3DXBUFFER buf;	
			const char* shaderName = "../Shaders/Model.fx";
			HRESULT hr = D3DXCreateEffectFromFile( context->Device(), shaderName, macros, NULL, NULL, NULL, &mesh.mEffect, &buf );

			if( FAILED(hr) && buf)
			{
				const char* errorText = reinterpret_cast<char*>( buf->GetBufferPointer() );
				char errorTitle[1024];
				sprintf_s( errorTitle, "Error compiling shader '%s'", shaderName );
				
#ifdef DEBUG
				// TODO: replace DXTRACE_ERR with vs console print so double clicking on file/line of error opens correct file
				DXTRACE_ERR(errorText, hr );
				__debugbreak();
#else
				MessageBox( NULL, errorText, errorTitle, MB_OK );
				
#endif
				buf->Release();
			}
			else 
			{
				// set static shader parameters
				D3DXHANDLE hDiffuseMap = mesh.mEffect->GetParameterBySemantic( NULL, "DIFFUSE_MAP" );
				DX_CHECK( mesh.mEffect->SetTexture( hDiffuseMap, mesh.mDiffuseMap ) );

				D3DXHANDLE hNormalMap = mesh.mEffect->GetParameterBySemantic( NULL, "NORMAL_MAP" );
				DX_CHECK( mesh.mEffect->SetTexture( hNormalMap, mesh.mNormalMap ) );

				D3DXHANDLE hLightDirection = mesh.mEffect->GetParameterBySemantic( NULL, "LIGHTDIRECTION" );
				D3DXVECTOR4 lightDirection = Math::Vector( 0.5, 1, 0 ).Normal();
				DX_CHECK( mesh.mEffect->SetVector( hLightDirection, &lightDirection ) );

				// store handles for dynamic shader parameters
				mesh.EffectParameters.mViewProjection = mesh.mEffect->GetParameterBySemantic( NULL, "VIEWPROJECTION" );
				mesh.EffectParameters.mBoneTransforms = mesh.mEffect->GetParameterBySemantic( NULL, "BONE_TRANSFORMS" );
				mesh.EffectParameters.mShaderTest = mesh.mEffect->GetParameterBySemantic( NULL, "SHADER_TEST" );

				for( int t=0; t<SAM_COUNT; ++t )
				{
					mesh.EffectParameters.mTechniques[t] = mesh.mEffect->GetTechniqueByName( mSkeletons[t]->GetShaderTechnique() );
				}
			}
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

	std::string binFileName = mFileName + 
#ifdef DEBUG
		".debug.bin";
#else
		".bin";
#endif

	ifstream binFileReader( binFileName, std::ios::binary );
	bool success = false;

	if( binFileReader.good() )
	{
		success = FromStream( binFileReader );
		binFileReader.close();
	}
	if( !success )
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

		// import skeleton, animation and mesh data
		{
			SkeletonBuilder skeletonBuilder( scene );
			AnimationBuilder animationBuilder( scene, skeletonBuilder );
			MeshBuilder meshBuilder( scene, skeletonBuilder );

			// build all skeletons
			skeletonBuilder.BuildSkeleton<Math::Matrix3x4>(	mSkeletons[SAM_LINEAR_BLEND_SKINNING], mPoseBuffers[SAM_LINEAR_BLEND_SKINNING] );
			skeletonBuilder.BuildSkeleton<Math::DualQuaternion>( mSkeletons[SAM_DUAL_QUATERNION], mPoseBuffers[SAM_DUAL_QUATERNION] );

			animationBuilder.BuildAnimations( mAnimations );
			meshBuilder.BuildMeshes( mMeshes );
		}
		// save data to bin file
		{
			std::ofstream binFileWriter( binFileName, std::ios::binary );
			if( binFileWriter.good() )
			{
				ToStream( binFileWriter );
			}
			binFileWriter.close();
		}
	}
	
	// upload to gpu
	AcquireResources( context );

	return true;

}

void SkeletalModel::PlayAnimation( unsigned int animationIndex, float playbackSpeed )
{
	if( animationIndex >= 0 && animationIndex < mAnimations.size() )
	{
		mCurrentAnimation = animationIndex;
		mAnimations[mCurrentAnimation].Play( playbackSpeed );
		mAnimationPaused = false;
	}
}

void SkeletalModel::PauseAnimation()
{
	mAnimationPaused = true;
}

bool SkeletalModel::ToggleAnimationPlayback()
{
	mAnimationPaused = !mAnimationPaused;
	return !mAnimationPaused;
}

int SkeletalModel::ToggleShaderTest()
{
	mShaderTest = ++mShaderTest % 2;
	return mShaderTest;
}

int SkeletalModel::ToggleAnimationMethod()
{

	mCurrentAnimationMethod = static_cast<SkeletalAnimationMethod>( (mCurrentAnimationMethod+1) % SAM_COUNT );
	return mCurrentAnimationMethod;
}

void SkeletalModel::Render( RenderContext* context )
{
	context->Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	context->Device()->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE);
	context->Device()->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE);
	context->Device()->SetRenderState( D3DRS_SRGBWRITEENABLE, TRUE );

	for( unsigned int m=0; m<mMeshes.size(); ++m )
	{
		const Mesh& mesh = mMeshes[m];
		if( mesh.mEffect )
		{
			// set dynamic shader parameters
			{
				Math::Matrix viewProjection = context->GetViewMatrix() * context->GetProjectionMatrix();
				DX_CHECK( mesh.mEffect->SetMatrix( mesh.EffectParameters.mViewProjection, &viewProjection.data ) );
		
				PoseBufferInterface& poseBuffer = *mPoseBuffers[mCurrentAnimationMethod];
				const float* poseData =  reinterpret_cast<const float*>( poseBuffer[0] );
				DX_CHECK( mesh.mEffect->SetFloatArray( mesh.EffectParameters.mBoneTransforms, poseData, poseBuffer.Size() / sizeof(float) ) );
			
				DX_CHECK( mesh.mEffect->SetInt( mesh.EffectParameters.mShaderTest, mShaderTest ) );
				DX_CHECK( mesh.mEffect->SetTechnique( mesh.EffectParameters.mTechniques[mCurrentAnimationMethod] ) );
			}

			// set geometry data
			{
				DX_CHECK( context->Device()->SetVertexDeclaration( mesh.mVertexDeclaration ) );
				DX_CHECK( context->Device()->SetStreamSource( 0, mesh.mVertexBuffer, 0, mesh.Data.mVertexSize ) );
				DX_CHECK( context->Device()->SetIndices( mesh.mIndexBuffer ) );
			}

			// now render
			{
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
		}
	}
	
	context->Device()->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );
	DX_CHECK( context->Device()->SetStreamSource( 0, NULL, 0, 0 ) );
	DX_CHECK( context->Device()->SetIndices( NULL ) );

}

void SkeletalModel::Update( float dt )
{
	SkeletonInterface& skeleton = *mSkeletons[mCurrentAnimationMethod];
	PoseBufferInterface& poseBuffer = *mPoseBuffers[mCurrentAnimationMethod];

	if( mCurrentAnimation >= 0 )
	{
		// evaluate animation and pose even when animation is paused, in order to get consistent timing
		mAnimations[mCurrentAnimation].Update( mAnimationPaused ? 0 : dt );
		mAnimations[mCurrentAnimation].EvaluatePose( skeleton );

		for( int i=0; i<skeleton.GetBoneCount(); ++i )
		{
			skeleton.GetWorldTransform( i, poseBuffer );
		}
	}
}

bool SkeletalModel::ToStream( std::ostream& stream )
{
	Serialization::ToStream( Serialization::STREAM_VERSION, stream );
	Serialization::ToStream( Serialization::STREAM_MAGIC, stream );

	for( int m=0; m < SAM_COUNT; ++m )
	{
		mSkeletons[m]->ToStream( stream );
		mPoseBuffers[m]->ToStream( stream );
	}

	Serialization::ToStream( mMeshes.size(), stream );
	for( unsigned int m=0; m<mMeshes.size(); ++m )
		mMeshes[m].Data.ToStream( stream );

	
	Serialization::ToStream( mAnimations.size(), stream );
	for( unsigned int a=0; a<mAnimations.size(); ++a )
		mAnimations[a].ToStream( stream );


	Serialization::ToStream( mCurrentAnimation, stream );
	Serialization::ToStream( mAnimationPaused, stream );
	Serialization::ToStream( mShaderTest, stream );
	Serialization::ToStream( mCurrentAnimationMethod, stream );

	Serialization::ToStream( Serialization::STREAM_MAGIC, stream );

	return stream.good();
}

bool SkeletalModel::FromStream( std::istream& stream )
{
	int streamVersion = Serialization::FromStream<int>( stream );
	int magic = Serialization::FromStream<int>( stream );

	if( streamVersion != Serialization::STREAM_VERSION || magic != Serialization::STREAM_MAGIC ) 
		return false;

 	for( int m=0; m < SAM_COUNT; ++m )
	{
		mSkeletons[m]->FromStream( stream );
		mPoseBuffers[m]->FromStream( stream );
	}

	// meshes
	{
		int meshCount = Serialization::FromStream<int>( stream );

		mMeshes.resize( meshCount );
		for( unsigned int m=0; m<mMeshes.size(); ++m )
			mMeshes[m].Data.FromStream( stream );
	}

	// animations
	{
		int animationCount = Serialization::FromStream<int>( stream );

		mAnimations.resize( animationCount );
		for( unsigned int a=0; a<mAnimations.size(); ++a )
			mAnimations[a].FromStream( stream );
	}
	

	// current animation
	Serialization::FromStream( stream, mCurrentAnimation );
	Serialization::FromStream( stream, mAnimationPaused );
	Serialization::FromStream( stream,mShaderTest );
	Serialization::FromStream( stream,mCurrentAnimationMethod );

	int magicTail = Serialization::FromStream<int>( stream );
	return magicTail == Serialization::STREAM_MAGIC;
}