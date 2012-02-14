#ifndef MAX_FLOAT_VECTORS_PER_MESH
#define MAX_FLOAT_VECTORS_PER_MESH 66*3
#endif

float4 BoneTransforms[MAX_FLOAT_VECTORS_PER_MESH] : BONE_TRANSFORMS;

float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

int ShaderTest : SHADER_TEST;

#include "DualQuaternionSkinning.h"
#include "LinearBlendSkinning.h"

struct VertexShaderInput
{
    float4 Position			: POSITION;
    float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Binormal			: BINORMAL;
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

	// transform tangent space vectors into world space
	result.Normal = mul( blendedTransform, float4( input.Normal, 0 ) );
	result.Tangent = mul( blendedTransform, float4( input.Tangent, 0 ) );
	result.Binormal = mul( blendedTransform, float4( input.Binormal, 0 ) );

	// transform position into clip space
	float3 blendedPosition = mul( blendedTransform, float4( input.Position.xyz, 1 ) );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	return result;
}


VertexShaderOutput DualQuaternionSkinning_VS( VertexShaderInput input )
{
	VertexShaderOutput result;
	result.TexCoord = input.TexCoord; 
	result.Color = float4( 0.75, 0.75, 0.75, 1 );
	
	// blend bone DQs
	float2x4 blendedDQ = GetBlendedDualQuaternion( input.BlendIndices, input.BlendWeights );

	// transform position into clip space
	float3 blendedPosition = transformPositionDQ( input.Position.xyz, blendedDQ[0], blendedDQ[1] );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	// transform tangent space vectors into world space
	result.Normal = transformNormalDQ( input.Normal, blendedDQ[0], blendedDQ[1] );
	result.Tangent = transformNormalDQ( input.Tangent, blendedDQ[0], blendedDQ[1] );
	result.Binormal = transformNormalDQ( input.Binormal, blendedDQ[0], blendedDQ[1] );

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
		
	const float ambient = 0.5f;
	float diffuse = saturate(dot(normal, LightDirection.xyz ) );

	float4 textureColor = tex2D( DiffuseSampler, input.TexCoord );
	return textureColor * (ambient + diffuse);
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



