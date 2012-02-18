

struct TangentFrame_Vectors
{
	float3 Normal			: TEXCOORD5;
	float3 Tangent			: TEXCOORD6;
	float3 Binormal			: TEXCOORD7;
};

struct TangentFrame_QTangent
{
	float4 Rotation			: NORMAL;
	float Reflection		: TANGENT;
};

float3x3 GetTangentFrameVectors( TangentFrame_QTangent tangentFrame )
{
	const float x = tangentFrame.Rotation.x, y = tangentFrame.Rotation.y, z = tangentFrame.Rotation.z, w = tangentFrame.Rotation.w;
	const float r = tangentFrame.Reflection;

	return float3x3(
		(1 - 2*(y*y + z*z)) * r,		2*(x*y - w*z) * r,			2*(x*z + w*y) * r,
		2*(x*y + w*z),					1 - 2*(x*x + z*z),			2*(y*z - w*x),
		2*(x*z - w*y),					2*(y*z + w*x),				1 - 2*(x*x + y*y)
	);
}

float3x3 GetTangentFrameVectors( TangentFrame_Vectors tangentFrame )
{
	return float3x3( tangentFrame.Normal, tangentFrame.Tangent, tangentFrame.Binormal );
}