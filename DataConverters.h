#pragma once

#include "stdafx.h"
#include "math.h"

#include "SkeletonBuilder.h"

class DataConverter
{
protected:
	D3DDECLUSAGE mUsageType;
	int mUsageIndex;
	int mOffset;

public:
	DataConverter( D3DDECLUSAGE usageType, int usageIndex, int offsetInBytes )
		: mUsageType( usageType )
		, mUsageIndex( usageIndex )
		, mOffset( offsetInBytes )
	{
	}

	virtual int Size() = 0;
	virtual void CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type ) = 0;
	virtual void CopyData( BYTE* destination, int index ) = 0;
};

template< typename T >
class ArrayDataConverter : public DataConverter
{
	const T* mSourceData;
	const int mSourceSize;

public:
	ArrayDataConverter( D3DDECLUSAGE usageType, int usageIndex, int& offsetInBytes, const T* sourceData, int sourceSize )
	: DataConverter( usageType, usageIndex, offsetInBytes )
	, mSourceData( sourceData )
	, mSourceSize( sourceSize ) {} 

	const T& GetElement( int index )
	{
		assert( index >= 0 && index < mSourceSize );
		return mSourceData[index];
	}
};

struct aiVector3DConverter : public ArrayDataConverter<aiVector3D>
{
	aiVector3DConverter( D3DDECLUSAGE usageType, int usageIndex, int& offsetInBytes, const aiVector3D* sourceData, int sourceSize )
		: ArrayDataConverter<aiVector3D>( usageType, usageIndex, offsetInBytes, sourceData, sourceSize ) {}
	
	int Size() { return 3 * sizeof(float); }

	virtual void CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type )
	{
		D3DVERTEXELEMENT9 _result = { 0, mOffset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, mUsageType, mUsageIndex };
		out_Type.push_back( _result );
	}

	void CopyData( BYTE* destination, int elementIndex  )
	{
		const aiVector3D& element = GetElement( elementIndex );

		float _data[] = { element[0], element[1], element[2] };
		memcpy( destination + mOffset, _data, Size() );
	}

};

struct aiVector3DToFloat2Converter : public ArrayDataConverter<aiVector3D>
{
	aiVector3DToFloat2Converter( D3DDECLUSAGE usageType, int usageIndex, int& offsetInBytes, const aiVector3D* sourceData, int sourceSize )
		: ArrayDataConverter<aiVector3D>( usageType, usageIndex, offsetInBytes, sourceData, sourceSize ) {}
	
	int Size() { return 2 * sizeof(float); }

	virtual void CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type )
	{
		D3DVERTEXELEMENT9 _result = { 0, mOffset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, mUsageType, mUsageIndex };
		out_Type.push_back( _result );
	}

	void CopyData( BYTE* destination, int elementIndex  )
	{
		const aiVector3D& element = GetElement( elementIndex );

		float _data[] = { element[0], element[1] };
		memcpy( destination + mOffset, _data, Size() );
	}

};

struct aiColor4DConverter : public ArrayDataConverter<aiColor4D>
{
	aiColor4DConverter( D3DDECLUSAGE usageType, int usageIndex, int& offsetInBytes, const aiColor4D* sourceData, int sourceSize )
	: ArrayDataConverter<aiColor4D>( usageType, usageIndex, offsetInBytes, sourceData, sourceSize ) {} 
	
	int Size() { return 4 * sizeof(BYTE); }

	virtual void CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type )
	{
		D3DVERTEXELEMENT9 _result = { 0, mOffset, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, mUsageType, mUsageIndex };
		out_Type.push_back( _result );
	}

	void CopyData( BYTE* destination, int elementIndex  )
	{
		const aiColor4D& element = GetElement( elementIndex );

		BYTE _data[] = 
		{
			Math::normalizedFloatToByte( element[0] ),
			Math::normalizedFloatToByte( element[1] ),
			Math::normalizedFloatToByte( element[2] ),
			Math::normalizedFloatToByte( element[3] ),
		};
			
		memcpy( destination + mOffset, _data, Size() );
	}
};

struct aiSkinningConverter : public DataConverter
{
	static const int MAX_INFLUENCES_PER_VERTEX = 4; // WARNING: tied to D3DDECLTYPE of vertex declaration. See CopyType()

	struct VertexInfluence
	{
		int BoneIndex;
		float Weight;
	};

	struct BoneWeightCompare
	{
		bool operator()(const VertexInfluence& x, const VertexInfluence& y)
		{
			return x.Weight < y.Weight;
		}
	};

	struct BoneIndexCompare
	{
		bool operator()(const VertexInfluence& x, const VertexInfluence& y)
		{
			return x.BoneIndex < y.BoneIndex;
		}
	};

	struct BoneWeightAccumulate
	{
		float operator()( float value, const VertexInfluence& x )
		{
			return value + x.Weight;
		}
	};

	typedef std::map<int, std::vector<VertexInfluence> > VertexInfluenceMap;
	VertexInfluenceMap mVertexInfluenceMap;

	aiSkinningConverter( int& offsetInBytes, aiBone** bones, unsigned int boneCount, const SkeletonBuilder& skeletonBuilder )
	: DataConverter( D3DDECLUSAGE_BLENDINDICES, 0, offsetInBytes ) 
	{
		// build map first
		for( unsigned int b=0; b<boneCount; ++b )
		{
			int boneIndex = skeletonBuilder.GetNodeIndex( bones[b]->mName.data );
			assert( boneIndex >= 0 );

			for( unsigned int w=0; w<bones[b]->mNumWeights; ++w )
			{
				const aiVertexWeight& aiVW = bones[b]->mWeights[w];
				VertexInfluence vi = { boneIndex, aiVW.mWeight };
				mVertexInfluenceMap[aiVW.mVertexId].push_back( vi );
			}
		}

		// now filter weights (we only allow MAX_INFLUENCES_PER_VERTEX bone influences per vertex )
		for( VertexInfluenceMap::iterator it = mVertexInfluenceMap.begin(); it != mVertexInfluenceMap.end(); ++it )
		{
			const int vertexIndex = it->first;
			std::vector<VertexInfluence>& influences = it->second;

			// only keep most significant weights
			std::sort( influences.begin(), influences.end(), BoneWeightCompare() );
			VertexInfluence dummy = { 0, 0 };
			influences.resize( MAX_INFLUENCES_PER_VERTEX, dummy );

			// renormalize weight
			float totalWeight = std::accumulate( influences.begin(), influences.end(), 0.f,BoneWeightAccumulate() );
			for( int i=0; i<MAX_INFLUENCES_PER_VERTEX; ++i )
				influences[i].Weight /= totalWeight;
		}
	} 
	
	int IndicesSize() { return 4 * sizeof(BYTE); }
	int WeightsSize() { return 4 * sizeof(float); }
	virtual int Size() { return IndicesSize() + WeightsSize(); }

	virtual void CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type )
	{
		D3DVERTEXELEMENT9 boneIndices = { 0, mOffset, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 };
		D3DVERTEXELEMENT9 boneWeights = { 0, mOffset + 4*sizeof(BYTE), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 };
		
		out_Type.push_back( boneIndices );
		out_Type.push_back( boneWeights );
	}

	void CopyData( BYTE* destination, int elementIndex  )
	{
		VertexInfluenceMap::const_iterator it = mVertexInfluenceMap.find( elementIndex );
		assert( it != mVertexInfluenceMap.end() );
		const std::vector<VertexInfluence>& influences = it->second;

		BYTE boneIndices[MAX_INFLUENCES_PER_VERTEX];
		float boneWeights[MAX_INFLUENCES_PER_VERTEX];

		for( int i=0; i<MAX_INFLUENCES_PER_VERTEX; ++i )
		{
			boneIndices[i] = influences[i].BoneIndex;
			boneWeights[i] = influences[i].Weight;
		}

 		memcpy( destination + mOffset, boneIndices, IndicesSize() );
		memcpy( destination + mOffset + IndicesSize(), boneWeights, WeightsSize() );
	}
};

struct TangentFrameToQTangentConverter : public DataConverter
{
	const aiVector3D* mNormals;
	const aiVector3D* mTangents;
	const aiVector3D* mBinormals;

	int mTangentFrameCount;

	TangentFrameToQTangentConverter(  D3DDECLUSAGE usageType, int usageIndex, int& offsetInBytes, const aiVector3D* normals, const aiVector3D* tangents, const aiVector3D* binormals, int tangentFrameCount ) 
		: DataConverter( usageType, usageIndex, offsetInBytes )
		, mNormals( normals )
		, mTangents( tangents )
		, mBinormals( binormals )
		, mTangentFrameCount( tangentFrameCount )
	{};

	virtual int TangentFrameToQTangentConverter::Size() { return sizeof( Math::Quaternion ); }

	virtual void TangentFrameToQTangentConverter::CopyType( std::vector<D3DVERTEXELEMENT9>& out_Type )
	{
		D3DVERTEXELEMENT9 qtangentElement =	{ 0, mOffset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, mUsageType, mUsageIndex };
		out_Type.push_back( qtangentElement );
	}

	void TangentFrameToQTangentConverter::CopyData( BYTE* destination, int elementIndex  )
	{
		assert( elementIndex >= 0 && elementIndex < mTangentFrameCount );
		
		Math::Matrix3x4 tangentFrame(
			mBinormals[elementIndex].x,		mBinormals[elementIndex].y,		mBinormals[elementIndex].z,		0,
			mTangents[elementIndex].x,		mTangents[elementIndex].y,		mTangents[elementIndex].z,		0,
			mNormals[elementIndex].x,		mNormals[elementIndex].y,		mNormals[elementIndex].z,		0
		);
		
		Math::Matrix3x4 tangentFrame_Original = tangentFrame;

		// flip y axis in case the tangent frame encodes a reflection
		float scale = tangentFrame.Determinant() > 0 ? 1.0f : -1.0f;

		tangentFrame.data[2][0] *= scale;
		tangentFrame.data[2][1] *= scale;
		tangentFrame.data[2][2] *= scale;

		Math::Quaternion tangentFrameQuaternion = tangentFrame;	
		
		// make sure we don't end up with 0 as w component
		{
			const float threshold = 0.000001f;
			const float renomalization = sqrt( 1.0f - threshold * threshold );

			if( abs(tangentFrameQuaternion.data.w) <= threshold )
			{
				tangentFrameQuaternion.data.w =  tangentFrameQuaternion.data.w > 0 ? threshold : -threshold;
				tangentFrameQuaternion.data.x *= renomalization;
				tangentFrameQuaternion.data.y *= renomalization;
				tangentFrameQuaternion.data.z *= renomalization;
			}
		}

		// encode reflection into quaternion's w element by making sign of w negative if y axis needs to be flipped, positive otherwise
		float qs = (scale<0 && tangentFrameQuaternion.data.w>0.f) || (scale>0 && tangentFrameQuaternion.data.w<0) ? -1.f : 1.f;

		tangentFrameQuaternion.data.x *= qs;
		tangentFrameQuaternion.data.y *= qs;
		tangentFrameQuaternion.data.z *= qs;
		tangentFrameQuaternion.data.w *= qs;

		memcpy( destination + mOffset, &tangentFrameQuaternion.data, sizeof(tangentFrameQuaternion) );
	}
};
