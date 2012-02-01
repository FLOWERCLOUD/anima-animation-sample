

float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

float4x4 BoneTransforms[16] : BONETRANSFORMS;

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
	float4 Color : TEXCOORD2;
};


VertexShaderOutput Model_VS( VertexShaderInput input )
{

	VertexShaderOutput result;
	result.Normal = input.Normal.xyz;
	result.TexCoord = input.TexCoord;
	result.Color = input.BlendWeights;

	
	float4 posH = float4( input.Position.xyz, 1.f );
	
	float4 blendedPosition =
		mul( posH, BoneTransforms[input.BlendIndices.x] ) * input.BlendWeights.x + 
		mul( posH, BoneTransforms[input.BlendIndices.y] ) * input.BlendWeights.y + 
		mul( posH, BoneTransforms[input.BlendIndices.z] ) * input.BlendWeights.z + 
		mul( posH, BoneTransforms[input.BlendIndices.w] ) * input.BlendWeights.w;

	result.Position =  mul( blendedPosition, ViewProjection );
	result.Normal = input.Normal.xyz;

	return result;
}


float4 Model_PS( VertexShaderOutput input ) : COLOR0
{
	
	const float ambient = 0.3f;
	float diffuse = saturate(dot(input.Normal, LightDirection.xyz ) );

	return float4( input.Color.xyz * (ambient + diffuse), 1 );
//	return float4(float3(1,0,0) * (ambient + diffuse),1);
}

technique Model
{
    pass P0
    {
        VertexShader = compile vs_2_0 Model_VS();
        PixelShader  = compile ps_2_0 Model_PS();
    }
}



