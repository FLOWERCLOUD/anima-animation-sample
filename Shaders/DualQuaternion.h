#ifndef __DUAL_QUATERNION_H__
#define __DUAL_QUATERNION_H__

float3x4 DQToMatrix( float4 realDQ, float4 dualDQ )
{	
	float3x4 M;

	float w = realDQ.w, x = realDQ.x, y = realDQ.y, z = realDQ.z;
	float t0 = dualDQ.w, t1 = dualDQ.x, t2 = dualDQ.y, t3 = dualDQ.z;
		
	M[0][0] = w*w + x*x - y*y - z*z;	M[0][1] = 2*x*y - 2*w*z;			M[0][2] = 2*x*z + 2*w*y;
	M[1][0] = 2*x*y + 2*w*z;			M[1][1] = w*w + y*y - x*x - z*z;	M[1][2] = 2*y*z - 2*w*x; 
	M[2][0] = 2*x*z - 2*w*y;			M[2][1] = 2*y*z + 2*w*x;			M[2][2] = w*w + z*z - x*x - y*y;
	
	M[0][3] = -2*t0*x + 2*w*t1 - 2*t2*z + 2*y*t3;
	M[1][3] = -2*t0*y + 2*t1*z - 2*x*t3 + 2*w*t2;
	M[2][3] = -2*t0*z + 2*x*t2 + 2*w*t3 - 2*t1*y;
	
	return M;	
}

float3 DQTransformPoint( float4 realDQ, float4 dualDQ, float3 pt )
{
	return pt + 2 * cross( realDQ.xyz, cross( realDQ.xyz, pt ) + realDQ.w * pt )
		+ 2 * ( realDQ.w * dualDQ.xyz - dualDQ.w * realDQ.xyz + cross( realDQ.xyz, dualDQ.xyz ) );
}

#endif // #define __DUAL_QUATERNION_H__