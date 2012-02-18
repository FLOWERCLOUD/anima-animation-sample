#ifndef MAX_FLOAT_VECTORS_PER_MESH
#define MAX_FLOAT_VECTORS_PER_MESH 66*3
#endif

float4 BoneTransforms[MAX_FLOAT_VECTORS_PER_MESH] : BONE_TRANSFORMS;

float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

int ShaderTest : SHADER_TEST;

#include "DualQuaternionSkinning.h"
#include "LinearBlendSkinning.h"
#include "TangentFrame.h"

struct VertexShaderInput
{
    float4 Position			: POSITION;
	TangentFrame_QTangent TangentFrame;	
	TangentFrame_Vectors TangentFrameReference;
	float2 TexCoord			: TEXCOORD0; 
	float4 BlendWeights		: BLENDWEIGHT0;
	uint4 BlendIndices		: BLENDINDICES0;
};

struct VertexShaderOutput
{
    float4 Position			: POSITION0;
    float2 TexCoord			: TEXCOORD0;
	float4 Color			: TEXCOORD1;

	float3 Normal			: TEXCOORD2;
	float3 Tangent			: TEXCOORD3;
	float3 Binormal			: TEXCOORD4;
};

texture NormalMap : NORMAL_MAP;	
sampler NormalSampler = sampler_state
{
    Texture = <NormalMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};

texture DiffuseMap : DIFFUSE_MAP;	
sampler DiffuseSampler = sampler_state
{
    Texture = <DiffuseMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
};

VertexShaderOutput LinearBlendSkinning_VS( VertexShaderInput input )
{
	VertexShaderOutput result;
	result.TexCoord = input.TexCoord; 
	result.Color = float4( 0.75, 0.75, 0.75, 1 );
	
	// get blended matrix
	float3x4 blendedTransform = GetBlendedMatrix( input.BlendIndices, input.BlendWeights );

	// get tangent space vectors and transform into world space
	{
		float3x3 tangentFrameVectors = GetTangentFrameVectors( input.TangentFrame );
	
		float3 normal = tangentFrameVectors[0];
		float3 tangent = tangentFrameVectors[1];
		float3 binormal = tangentFrameVectors[2];

		result.Normal = mul( blendedTransform, float4( normal, 0 ) );
		result.Tangent = mul( blendedTransform, float4( tangent, 0 ) );
		result.Binormal = mul( blendedTransform, float4( binormal, 0 ) );
	}

	// transform position into clip space
	float3 blendedPosition = mul( blendedTransform, float4( input.Position.xyz, 1 ) );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	return result;
}


VertexShaderOutput DualQuaternionSkinning_VS( VertexShaderInput input )
{
	VertexShaderOutput result = (VertexShaderOutput)0;
	result.TexCoord = input.TexCoord; 
	result.Color = float4( 0.75, 0.75, 0.75, 1 );

	// blend bone DQs
	float2x4 blendedDQ = GetBlendedDualQuaternion( input.BlendIndices, input.BlendWeights );

	// transform position into clip space
	float3 blendedPosition = transformPositionDQ( input.Position.xyz, blendedDQ[0], blendedDQ[1] );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	// get tangent space vectors and transform into world space
	{
		float3x3 tangentFrameVectors = GetTangentFrameVectors( input.TangentFrame );
	
		float3 normal = tangentFrameVectors[0];
		float3 tangent = tangentFrameVectors[1];
		float3 binormal = tangentFrameVectors[2];

		result.Normal = transformNormalDQ( normal, blendedDQ[0], blendedDQ[1] );
		result.Tangent = transformNormalDQ( tangent, blendedDQ[0], blendedDQ[1] );
		result.Binormal = transformNormalDQ( binormal, blendedDQ[0], blendedDQ[1] );
	}

	return result;
}

 
float4 Model_PS( VertexShaderOutput input ) : COLOR0
{
	float3 textureNormal = tex2D( NormalSampler, input.TexCoord ).xyz*2-1;

	float3 normal = normalize(
			textureNormal.y * input.Tangent +
			textureNormal.x * input.Binormal +
			textureNormal.z * input.Normal
		);

	const float ambient = 0.75f;
	float diffuseFactor = 0.25f;

	float diffuse = dot(normal, LightDirection.xyz );
	float4 textureColor = tex2D( DiffuseSampler, input.TexCoord );

	return textureColor * (ambient + diffuse * diffuseFactor);
}

technique LinearBlendSkinning
{
    pass P0
    {
        VertexShader = compile vs_3_0 LinearBlendSkinning_VS();
        PixelShader  = compile ps_3_0 Model_PS();
    }
}

technique DualQuaternionSkinning
{
    pass P0
    {
        VertexShader = compile vs_3_0 DualQuaternionSkinning_VS();
        PixelShader  = compile ps_3_0 Model_PS();
    }
}



