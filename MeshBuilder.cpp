#include "MeshBuilder.h"

MeshBuilder:: MeshBuilder( const aiScene* scene, const SkeletonBuilder& skeletonBuilder )
	: mScene( scene )
	, mSkeletonBuilder( skeletonBuilder )
{
	CreateDataConverters( mDataConverters );
}


MeshBuilder::~MeshBuilder(void)
{
	// delete data converters again
	for( ConverterMap::iterator it = mDataConverters.begin(); it != mDataConverters.end(); ++it )
	{
		std::vector<DataConverter*>& converters = it->second;
		for( unsigned int i=0; i<converters.size(); ++i )
			delete converters[i];
		converters.clear();
	}
}

int  MeshBuilder::GetVertexSize( const aiMesh* mesh ) const
{
	int vertexSize = 0;

	ConverterMap::const_iterator it = mDataConverters.find( mesh );
	if( it != mDataConverters.end() )
	{
		const std::vector<DataConverter*>& converters = it->second;
		for( unsigned int c=0; c<converters.size(); ++c )
			vertexSize += converters[c]->Size();

	}
	return vertexSize;
}

void MeshBuilder::CreateDataConverters( ConverterMap& result ) const
{
	for( unsigned int m=0; m<mScene->mNumMeshes; ++m )
	{
		const aiMesh* mesh = mScene->mMeshes[m];
		const int vertexCount = mesh->mNumVertices;

		int offset = 0;
		std::vector<DataConverter*> converters;

		// positions, normals and tangents/bitangents
		if( mesh->HasPositions() )	
		{
			converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_POSITION, 0, offset, mesh->mVertices, vertexCount ) );
			offset += converters.back()->Size();
		}

		if( mesh->HasNormals()  ) 
		{
			if( mesh->HasTangentsAndBitangents() )
			{
				converters.push_back( new TangentFrameToQTangentConverter( D3DDECLUSAGE_NORMAL, 0, offset, mesh->mNormals, mesh->mTangents, mesh->mBitangents, vertexCount ) );
				offset += converters.back()->Size();

				// add uncompressed tangent space for reference in debug mode
#ifdef DEBUG
				{
					converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_TEXCOORD, 5, offset, mesh->mNormals, vertexCount ) );
					offset += converters.back()->Size();

					converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_TEXCOORD, 6, offset, mesh->mTangents, vertexCount ) );
					offset += converters.back()->Size();

					converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_TEXCOORD, 7, offset, mesh->mBitangents, vertexCount ) );
					offset += converters.back()->Size();
				}
#endif
			}
			else
			{
				converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_NORMAL, 0, offset, mesh->mNormals, vertexCount ) );
				offset += converters.back()->Size();
			}
		}

		for( unsigned int c=0; c < mesh->GetNumColorChannels(); ++c )
		{
			converters.push_back( new aiColor4DConverter( D3DDECLUSAGE_COLOR, 0, offset, mesh->mColors[c], vertexCount ) );
			offset += converters.back()->Size();

		}

		for( unsigned int t=0; t < mesh->GetNumUVChannels(); ++t )
		{
			if( mesh->mNumUVComponents[t] == 2 )
				converters.push_back( new aiVector3DToFloat2Converter( D3DDECLUSAGE_TEXCOORD, t, offset, mesh->mTextureCoords[t], vertexCount ) );
			else
				converters.push_back( new aiVector3DConverter( D3DDECLUSAGE_TEXCOORD, t, offset, mesh->mTextureCoords[t], vertexCount ) );	

			offset += converters.back()->Size();
		}

		if( mesh->HasBones() )
		{
			converters.push_back( new aiSkinningConverter( offset, mesh->mBones, mesh->mNumBones, mSkeletonBuilder ) );
			offset += converters.back()->Size();
		}

		result[mesh] = converters;
	}
}

void MeshBuilder::ReadTexture( std::vector<BYTE>& result, const char* fileName )
{
	std::ifstream textureStream;
	textureStream.open( fileName, std::ios::binary );

	textureStream.seekg (0, std::ios::end);
	const unsigned int numBytes = static_cast<const unsigned int>( textureStream.tellg() );
	textureStream.seekg( 0, std::ios::beg );

	result.resize( numBytes );
	textureStream.read( reinterpret_cast<char*>( &result[0] ), numBytes );
	textureStream.close();
}


void MeshBuilder::BuildMeshes( std::vector<SkeletalModel::Mesh>& meshes )
{
	for( unsigned int m=0; m<mScene->mNumMeshes; ++m )
	{
		aiMesh* mesh = mScene->mMeshes[m];
		meshes.push_back( SkeletalModel::Mesh() );

		SkeletalModel::MeshData& meshData = meshes.back().Data;

		// find out vertex size first
		std::vector<DataConverter*>& converters = mDataConverters[mesh];
		meshData.mVertexSize = GetVertexSize( mesh );
		

		// build vertex declaration
		{
			for( unsigned int i=0; i<converters.size(); ++i )
				converters[i]->CopyType( meshData.mVertexElements );

			D3DVERTEXELEMENT9 endElement = D3DDECL_END();
			meshData.mVertexElements.push_back( endElement ); 
		}

		// fill vertex data buffer
		{
			const int vertexBufferSize = mesh->mNumVertices * meshData.mVertexSize;
			meshData.mVertexData.resize( vertexBufferSize );

			for( unsigned int v=0; v<mesh->mNumVertices; ++v )
			{
				for( unsigned int i=0; i<converters.size(); ++i )
					converters[i]->CopyData( &meshData.mVertexData[v * meshData.mVertexSize], v );
			}
			meshData.mVertexCount = mesh->mNumVertices;
		}

		// fill index buffer
		{
			meshData.mIndexFormat = mesh->mNumVertices < 0xFFFF ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
			const int indexSize = meshData.mIndexFormat==D3DFMT_INDEX16 ?  sizeof(UINT16) : sizeof(UINT32);

			const int indexBufferSize = mesh->mNumFaces * 3 * indexSize;
			meshData.mIndexData.resize( indexBufferSize );
		
			for( unsigned int f=0; f<mesh->mNumFaces; ++f )
			{
				const aiFace& face = mesh->mFaces[f];
				assert( face.mNumIndices == 3 ); // we only accept triangle meshes

				UINT32 indices[3];

				if( meshData.mIndexFormat == D3DFMT_INDEX16 )
				{
					assert( face.mIndices[0] < 0xFFFF && face.mIndices[1] < 0xFFFF && face.mIndices[2] < 0xFFFF );

					UINT16* indices16 = reinterpret_cast<UINT16*>( indices );
					indices16[0] = face.mIndices[0];
					indices16[1] = face.mIndices[1];
					indices16[2] = face.mIndices[2];
				}
				else
				{
					indices[0] = face.mIndices[0];
					indices[1] = face.mIndices[1];
					indices[2] = face.mIndices[2];
				}

				memcpy( &meshData.mIndexData[f * 3 * indexSize], indices, 3*indexSize );
			}

			meshData.mTriangleCount = mesh->mNumFaces;
		}

		// read textures
		{
			ReadTexture( meshData.mAlbedoMap, "../Textures/frank_D.dds" );
			ReadTexture( meshData.mNormalMap, "../Textures/frank_N.dds" );
		}
	}
}
