#include "Skinning.h"
#include "Gamma.h"
#include "Debug.h"
#include "DualQuaternion.h"

float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

struct VertexShaderInput
{
    float4 Position			: POSITION;
	TangentFrame TangentFrameData;	
	float2 TexCoord			: TEXCOORD0; 
	float4 BlendWeights		: BLENDWEIGHT0;
	uint4 BlendIndices		: BLENDINDICES0;
};

struct VertexShaderOutput
{
    float4 Position			: POSITION0;
    float2 TexCoord			: TEXCOORD0;
	float4 Color			: TEXCOORD1;
	float3x3 TangentFrame	: TEXCOORD2;
};

texture NormalMap : NORMAL_MAP;	
sampler NormalSampler = sampler_state
{
    Texture = <NormalMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;

	sRGBTexture = false;
};

texture DiffuseMap : DIFFUSE_MAP;	
sampler DiffuseSampler = sampler_state
{
    Texture = <DiffuseMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;

	sRGBTexture = true;
};

VertexShaderOutput LinearBlendSkinning_VS( VertexShaderInput input )
{
	VertexShaderOutput result;
	result.TexCoord = input.TexCoord; 
	result.Color = float4( 0.75, 0.75, 0.75, 1 );
	
	// get blended matrix
	float3x4 blendedTransform = GetBlendedMatrix( input.BlendIndices, input.BlendWeights );

	// get world space tangent frame vectors
	result.TangentFrame = GetTangentFrame( (float3x3)blendedTransform, input.TangentFrameData );

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
	float3 blendedPosition = DQTransformPoint( blendedDQ[0], blendedDQ[1], input.Position.xyz );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	// get world space tangent frame vectors
	result.TangentFrame = GetTangentFrame( blendedDQ[0], input.TangentFrameData );
	
	return result;
}

 
float4 Model_PS( VertexShaderOutput input ) : COLOR0
{
	float3 textureNormal = tex2D( NormalSampler, input.TexCoord ).xyz*2-1;
	float3 normal = normalize( mul( textureNormal, input.TangentFrame ) );
	
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



