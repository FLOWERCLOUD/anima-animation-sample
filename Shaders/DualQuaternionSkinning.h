

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

	
	// Antipodality correction. Doesn't seem to be necessary for this model
	/*
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

