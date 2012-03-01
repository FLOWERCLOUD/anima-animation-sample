#ifndef __QUATERNION_H__
#define __QUATERNION_H__

float4 QuaternionMultiply( float4 q1, float4 q2 )
{
	return float4(
		q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
		q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
		q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w,
		q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z
	);
}

float4 QuaternionInverse( float4 q )
{
	return float4(-q.xyz, q.w) / (q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

float3x3 QuaternionToMatrix( float4 q )
{
	return float3x3(
		1 - 2*(q.y*q.y + q.z*q.z),			2*(q.x*q.y + q.w*q.z),			2*(q.x*q.z - q.w*q.y),
		2*(q.x*q.y - q.w*q.z),				1 - 2*(q.x*q.x + q.z*q.z),		2*(q.y*q.z + q.w*q.x),
		2*(q.x*q.z + q.w*q.y),				2*(q.y*q.z - q.w*q.x),			1 - 2*(q.x*q.x + q.y*q.y)
	);
}

float3 QuaternionTransformPoint( float4 quaternion, float3 pt  )
{
	return pt + 2.0f * cross( quaternion.xyz, cross( quaternion.xyz, pt ) + quaternion.w * pt );
}


#endif // #define __QUATERNION_H__