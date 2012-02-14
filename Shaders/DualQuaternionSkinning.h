

float2x4 GetBoneDualQuaternion( uint boneIndex )
{
	return float2x4(
		BoneTransforms[boneIndex*2 + 0],
		BoneTransforms[boneIndex*2 + 1]
	);
}

float2x4 GetBlendedDualQuaternion( uint4 boneIndices, float4 boneWeights )
{
	float2x4 dq0 = GetBoneDualQuaternion( boneIndices.x );
	float2x4 dq1 = GetBoneDualQuaternion( boneIndices.y );
	float2x4 dq2 = GetBoneDualQuaternion( boneIndices.z );
	float2x4 dq3 = GetBoneDualQuaternion( boneIndices.w );

	/*
	// Antipodality correction. Doesn't seem to be necessary for this model
	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	if (dot(dq0[0], dq2[0]) < 0.0) dq2 *= -1.0;	
	if (dot(dq0[0], dq3[0]) < 0.0) dq3 *= -1.0;
	*/

	float2x4 blendedDQ =
		dq0 * boneWeights.x + 
		dq1 * boneWeights.y + 
		dq2 * boneWeights.z + 
		dq3 * boneWeights.w;

	float normDQ = length(blendedDQ[0]);
	return blendedDQ / normDQ;
}


float3x4 DQToMatrix(float4 real, float4 dual)
{	
	float3x4 M;

	float w = real.w, x = real.x, y = real.y, z = real.z;
	float t0 = dual.w, t1 = dual.x, t2 = dual.y, t3 = dual.z;
		
	M[0][0] = w*w + x*x - y*y - z*z;	M[0][1] = 2*x*y - 2*w*z;			M[0][2] = 2*x*z + 2*w*y;
	M[1][0] = 2*x*y + 2*w*z;			M[1][1] = w*w + y*y - x*x - z*z;	M[1][2] = 2*y*z - 2*w*x; 
	M[2][0] = 2*x*z - 2*w*y;			M[2][1] = 2*y*z + 2*w*x;			M[2][2] = w*w + z*z - x*x - y*y;
	
	M[0][3] = -2*t0*x + 2*w*t1 - 2*t2*z + 2*y*t3;
	M[1][3] = -2*t0*y + 2*t1*z - 2*x*t3 + 2*w*t2;
	M[2][3] = -2*t0*z + 2*x*t2 + 2*w*t3 - 2*t1*y;
	
	return M;	
}

float3 transformPositionDQ( float3 position, float4 realDQ, float4 dualDQ )
{
	return position + 2 * cross( realDQ.xyz, cross( realDQ.xyz, position ) + realDQ.w * position )
		+ 2 * ( realDQ.w * dualDQ.xyz - dualDQ.w * realDQ.xyz + cross( realDQ.xyz, dualDQ.xyz ) );
}

float3 transformNormalDQ( float3 normal, float4 realDQ, float4 dualDQ )
{
	return normal + 2.0 * cross( realDQ.xyz, cross( realDQ.xyz, normal ) + realDQ.w * normal );
}


