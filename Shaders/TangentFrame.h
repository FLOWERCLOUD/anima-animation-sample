#ifndef __TANGENT_FRAME_H__
#define __TANGENT_FRAME_H__

#include "Quaternion.h"
#include "Debug.h"

struct TangentFrame_Vectors
{
	float3 Normal			: TEXCOORD5;
	float3 Tangent			: TEXCOORD6;
	float3 Binormal			: TEXCOORD7;
};

struct TangentFrame_QTangent
{
	float4 Rotation			: NORMAL;
};

struct TangentFrame
{
	TangentFrame_Vectors vectors;
	TangentFrame_QTangent qtangent;
};

float2x3 QuaternionToTangentBitangent( float4 q )
{
	return float2x3(
		1 - 2*(q.y*q.y + q.z*q.z),			2*(q.x*q.y + q.w*q.z),			2*(q.x*q.z - q.w*q.y),
		2*(q.x*q.y - q.w*q.z),				1 - 2*(q.x*q.x + q.z*q.z),		2*(q.y*q.z + q.w*q.x)
	);
}

#ifdef DEBUG

float3x3 GetTangentFrame( float3x3 worldTransform, TangentFrame tangentFrame )
{
	float3x3 tf;
	if( ShaderTest < 1 )
	{
		tf = QuaternionToMatrix( tangentFrame.qtangent.Rotation );
		tf[2] *= (tangentFrame.qtangent.Rotation.w < 0 ? -1 : 1);
	}
	else
	{
		tf = float3x3( 
			tangentFrame.vectors.Binormal,
			tangentFrame.vectors.Tangent, 
			tangentFrame.vectors.Normal 
		);
	}

	return mul( tf, transpose(worldTransform) );
}

float3x3 GetTangentFrame( float4 worldTransform, TangentFrame tangentFrame )
{
	float3x3 result;
	if( ShaderTest < 1 )
	{
		float4 q = QuaternionMultiply( worldTransform,  tangentFrame.qtangent.Rotation );
		result = QuaternionToMatrix( q );
		result[2] *= (tangentFrame.qtangent.Rotation.w < 0 ? -1 : 1); 
	}
	else
	{
		// ideally we'd convert the qtangent to a rotation matrix here.
		float3x3 wt = QuaternionToMatrix( worldTransform );
		float3x3 tf = float3x3( 
			tangentFrame.vectors.Binormal,
			tangentFrame.vectors.Tangent, 
			tangentFrame.vectors.Normal 
		);

		result = mul(tf, wt);
	}

	return result;
}

#else // !defined(DEBUG)

float3x3 GetTangentFrame( float3x3 worldTransform, TangentFrame tangentFrame )
{
	float2x3 tBt = QuaternionToTangentBitangent( tangentFrame.qtangent.Rotation );
	float3 t = mul( worldTransform, tBt[0] );
	float3 b = mul( worldTransform, tBt[1] );

	return float3x3(
		t,
		b,
		cross(t,b) * (tangentFrame.qtangent.Rotation.w < 0 ? -1 : 1)
	);
}

float3x3 GetTangentFrame( float4 worldTransform, TangentFrame tangentFrame )
{
	float4 q = QuaternionMultiply( worldTransform,  tangentFrame.qtangent.Rotation );
	float2x3 tBt = QuaternionToTangentBitangent( q );

	return float3x3(
		tBt[0],
		tBt[1],
		cross(tBt[0],tBt[1]) * (tangentFrame.qtangent.Rotation.w < 0 ? -1 : 1)
	);
}
#endif // DEBUG

#endif // __TANGENT_FRAME_H__