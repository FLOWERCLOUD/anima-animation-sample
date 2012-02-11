//#ifndef MAX_FLOAT_VECTORS_PER_MESH
#define MAX_FLOAT_VECTORS_PER_MESH 60*3
//#endif

float4 BoneTransforms[60*3] : BONE_TRANSFORMS : register(c0);

float4x4 ViewProjection	: VIEWPROJECTION;
float4 LightDirection	: LIGHTDIRECTION;

int ShaderTest : SHADER_TEST;

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

float3x4 GetBoneMatrix( int boneIndex )
{
	// return BoneTransforms[boneIndex];
	
	return float3x4(
		BoneTransforms[boneIndex*3 + 0],
		BoneTransforms[boneIndex*3 + 1],
		BoneTransforms[boneIndex*3 + 2]
	);
	
}

VertexShaderOutput Model_VS( VertexShaderInput input )
{
	VertexShaderOutput result;
	result.TexCoord = input.TexCoord; 
	result.Color = float4( 0.75, 0.75, 0.75, 1 );
	
	float3x4 blendedTransform =
		GetBoneMatrix( input.BlendIndices.x ) * input.BlendWeights.x + 
		GetBoneMatrix( input.BlendIndices.y ) * input.BlendWeights.y + 
		GetBoneMatrix( input.BlendIndices.z ) * input.BlendWeights.z + 
		GetBoneMatrix( input.BlendIndices.w ) * input.BlendWeights.w;

	result.Normal = mul( blendedTransform, float4( input.Normal, 0 ) );
	result.Tangent = mul( blendedTransform, float4( input.Tangent, 0 ) );
	result.Binormal = mul( blendedTransform, float4( input.Binormal, 0 ) );

	float3 blendedPosition = mul( blendedTransform, float4( input.Position.xyz, 1 ) );
	result.Position =  mul( float4( blendedPosition.xyz, 1), ViewProjection );

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

technique Model
{
    pass P0
    {
        VertexShader = compile vs_3_0 Model_VS();
        PixelShader  = compile ps_3_0 Model_PS();
    }
}



