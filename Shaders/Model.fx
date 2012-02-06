#ifndef MAX_BONES_PER_MESH
#define MAX_BONES_PER_MESH 60
#endif

float4x3 BoneTransforms[MAX_BONES_PER_MESH] : BONE_TRANSFORMS;
float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

int ShaderTest : SHADER_TEST;

struct VertexShaderInput
{
    float4 Position			: POSITION;
    float3 Normal			: NORMAL;
	float2 TexCoord			: TEXCOORD0;
	float4 BlendWeights		: BLENDWEIGHT0;
	uint4 BlendIndices		: BLENDINDICES0;
};

struct VertexShaderOutput
{
    float4 Position			: POSITION0;
	float3 Normal			: TEXCOORD1;
    float2 TexCoord			: TEXCOORD0;
	float4 Color			: TEXCOORD2;
};


VertexShaderOutput Model_VS( VertexShaderInput input )
{
	VertexShaderOutput result;
	result.TexCoord = input.TexCoord;
	result.Color = float4( 0.75, 0.75, 0.75, 1 );
	
	float4 posH = float4( input.Position.xyz, 1.f );
	float4 normalH = float4( input.Normal.xyz, 0.f );

	float4x3 blendedTransform =
		BoneTransforms[input.BlendIndices.x] * input.BlendWeights.x + 
		BoneTransforms[input.BlendIndices.y] * input.BlendWeights.y + 
		BoneTransforms[input.BlendIndices.z] * input.BlendWeights.z + 
		BoneTransforms[input.BlendIndices.w] * input.BlendWeights.w;

	float3 blendedPosition = mul( posH, blendedTransform );
	float3 blendedNormal = mul( normalH, blendedTransform );

	result.Normal = normalize( blendedNormal.xyz );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

	return result;
}


float4 Model_PS( VertexShaderOutput input ) : COLOR0
{
	const float ambient = 0.3f;
	float diffuse = saturate(dot(input.Normal, LightDirection.xyz ) );
//	return float4( input.Normal * 0.5 + 0.5, 1);
	return float4( input.Color.xyz * (ambient + diffuse), 1 );
}

technique Model
{
    pass P0
    {
        VertexShader = compile vs_3_0 Model_VS();
        PixelShader  = compile ps_3_0 Model_PS();
    }
}



