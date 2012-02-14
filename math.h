#if !defined(__MATH_H__)
#define __MATH_H__

#include "stdafx.h"
#include <D3dx9math.h>
#include "Debug.h"

/* Simple math functionality. Wraps d3dx math functions, the idea was that the same code can be used across the d3d and  opengl build
	Note that the math usage of this application is very basic and not performance critical => as a result no attention has been spent on efficiency	*/
namespace Math
{
	struct Vector
	{
		D3DXVECTOR3 data;
		Vector() : data(0,0,0) {}
		Vector( float x, float y, float z ) : data(x,y,z){}

		float GetX() const { return data.x; }
		float GetY() const { return data.y; }
		float GetZ() const { return data.z; }

		operator D3DXVECTOR3() 
		{ 
			return data; 
		}

		operator D3DXVECTOR4()
		{
			return D3DXVECTOR4( GetX(), GetY(), GetZ(), 1.0f );
		}

		Vector Normal() const
		{
			Vector result;
			D3DXVec3Normalize( &result.data, &data );
			return result;
		}

		Vector operator-( const Vector& other ) const
		{
			return Vector( GetX() - other.GetX(), GetY() - other.GetY(), GetZ() - other.GetZ() );
		}

		Vector operator+( const Vector& other ) const
		{
			return Vector( GetX() + other.GetX(), GetY() + other.GetY(), GetZ() + other.GetZ() );
		}

		Vector operator+=( const Vector& other ) 
		{
			*this = *this + other;
			return *this;
		}

		Vector Scale( const float scale ) const
		{
			return Vector( GetX() * scale, GetY() * scale, GetZ() * scale );
		}

		Vector Cross( const Vector& other ) const
		{
			Vector result;
			D3DXVec3Cross( &result.data, &data, &other.data );
			return result;
			
		}

		float Dot( const Vector& other ) const
		{
			return D3DXVec3Dot( &data, &other.data );
		}
		
		static float Dot( float* a, float* b )
		{
			return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
		}
	};

	struct Matrix
	{
		D3DXMATRIX data;

		static Matrix RotationYawPitchRoll( Vector yawPitchRoll )
		{
			Matrix result;
			D3DXMatrixRotationYawPitchRoll( &result.data, yawPitchRoll.GetX(), yawPitchRoll.GetY(), yawPitchRoll.GetZ() );
			return result;
		}

		static Matrix Translation( Vector translation )
		{
			Matrix result;
			D3DXMatrixTranslation( &result.data, translation.GetX(), translation.GetY(), translation.GetZ() );
			return result;

		}

		static Matrix Scale( Vector scale )
		{
			Matrix result;
			D3DXMatrixScaling( &result.data, 0.5f, 0.5f, 0.5f );
			return result;
		}

		static Matrix LookAt( const Vector& eye, const Vector& lookAt, const Vector& up )
		{
			Matrix result;
			D3DXMatrixLookAtLH( &result.data, &eye.data, &lookAt.data, &up.data );
			return result;
  
		}
		static Matrix Perspective( float fov, float aspect, float n, float f )
		{
			Matrix result;
			D3DXMatrixPerspectiveFovLH( &result.data, D3DXToRadian( fov ), aspect, n, f );
			return result;
		}

		Matrix() {}

		Matrix( const aiMatrix4x4& other )
		{
			D3DXMATRIX tmp( &other[0][0] );
			D3DXMatrixTranspose( &data, &tmp );	
		}

		Matrix operator*( const Matrix& right ) const
		{
			Matrix result;
			D3DXMatrixMultiply( &result.data, &data, &right.data );
			return result;
		}

		Matrix Inverse() const
		{
			Matrix result;
			D3DXMatrixInverse( &result.data, NULL, &data );
			return result;
		}

		Matrix Transpose() const
		{
			Matrix result;
			D3DXMatrixTranspose( &result.data, &data );
			return result;
		}

		Vector Transform( const Vector& v ) const
		{
			D3DXVECTOR4 t;
			D3DXVec3Transform( &t, &v.data, &data );
			return Vector(t.x, t.y, t.z);
		}

		Vector TransformNormal( const Vector& n ) const
		{
			Vector result;
			D3DXVec3TransformNormal( &result.data, &n.data, &data );
			return result;		
		}
	};

	/* NOTE: this matrix is meant to be used for POST multiplication, e.g. Matrix * vector as opposed of DirectX's default vector * Matrix.
	* the Matrix layout has been defined like this on purpose so individual columns can be passed directly to the shader (hlsl uses column-major 
	* matrices by default) */
	struct Matrix3x4
	{
		float data[3][4];

		operator float*()
		{
			return &data[0][0];
		}

		Matrix3x4()
		{
			memset( data, 0x0, sizeof(data) );
			data[0][0] = 1.f;
			data[1][1] = 1.f;
			data[2][2] = 1.f;
		}


		Matrix3x4( const aiMatrix4x4& other )
		{
			memcpy( &data[0][0], &other[0][0], sizeof(Matrix3x4) );
		}

		Matrix3x4( const aiVector3D& translation, const aiQuaternion& rotation, const aiVector3D& scale )
		{
			aiMatrix3x3 r = rotation.GetMatrix();
			memcpy( &data[0], &r[0][0], sizeof(data[0]) );
			memcpy( &data[1], &r[1][0], sizeof(data[1]) );
			memcpy( &data[2], &r[2][0], sizeof(data[2]) );

			data[0][0] *= scale.x;			data[1][1] *= scale.y;			data[2][2] *= scale.z;
			data[0][3] = translation.x;		data[1][3] = translation.y;		data[2][3] = translation.z;
		}

		Matrix3x4 operator*( const Matrix3x4& right ) const
		{
			Matrix3x4 result;
			result.data[0][0] = data[0][0] * right.data[0][0] + data[0][1] * right.data[1][0] + data[0][2] * right.data[2][0];
			result.data[0][1] = data[0][0] * right.data[0][1] + data[0][1] * right.data[1][1] + data[0][2] * right.data[2][1];
			result.data[0][2] = data[0][0] * right.data[0][2] + data[0][1] * right.data[1][2] + data[0][2] * right.data[2][2];
			result.data[0][3] = data[0][0] * right.data[0][3] + data[0][1] * right.data[1][3] + data[0][2] * right.data[2][3] + data[0][3];

			result.data[1][0] = data[1][0] * right.data[0][0] + data[1][1] * right.data[1][0] + data[1][2] * right.data[2][0];
			result.data[1][1] = data[1][0] * right.data[0][1] + data[1][1] * right.data[1][1] + data[1][2] * right.data[2][1];
			result.data[1][2] = data[1][0] * right.data[0][2] + data[1][1] * right.data[1][2] + data[1][2] * right.data[2][2];
			result.data[1][3] = data[1][0] * right.data[0][3] + data[1][1] * right.data[1][3] + data[1][2] * right.data[2][3] + data[1][3];

			result.data[2][0] = data[2][0] * right.data[0][0] + data[2][1] * right.data[1][0] + data[2][2] * right.data[2][0];
			result.data[2][1] = data[2][0] * right.data[0][1] + data[2][1] * right.data[1][1] + data[2][2] * right.data[2][1];
			result.data[2][2] = data[2][0] * right.data[0][2] + data[2][1] * right.data[1][2] + data[2][2] * right.data[2][2];
			result.data[2][3] = data[2][0] * right.data[0][3] + data[2][1] * right.data[1][3] + data[2][2] * right.data[2][3] + data[2][3];

			return result;

		}

		void Print() const
		{
			const float* values = &data[0][0];
			for( int i=0; i<12; ++i )
				DebugPrint( "%.6f ", values[i] );
			DebugPrint( "\n" );
		}

	
	};

	struct Quaternion
	{
		D3DXQUATERNION data; // rx, ry, rz, s

		Quaternion() : data(0,0,0,1) {}
		Quaternion( float x, float y, float z, float w ) : data( x, y, z, w ) {}
		Quaternion( const D3DXQUATERNION& other ) : data( other ) {}
		Quaternion( const Quaternion& other ) : data( other.data ) {}

		Quaternion operator*( const Quaternion& right ) const
		{
			const float a1 = data.w, b1 = data.x, c1 = data.y, d1 = data.z;
			const float a2 = right.data.w, b2 = right.data.x, c2 = right.data.y, d2 = right.data.z;
		   
			return Quaternion(
				a1*b2 + b1*a2 + c1*d2 - d1*c2,
				a1*c2 - b1*d2 + c1*a2 + d1*b2,
				a1*d2 + b1*c2 - c1*b2 + d1*a2,
				a1*a2 - b1*b2 - c1*c2 - d1*d2
			);
		}

		Quaternion operator*( float factor ) const
		{
			return  Quaternion( data.x * factor, data.y * factor, data.z * factor, data.w * factor );
		}

		Quaternion operator+( const Quaternion& other )
		{
			return Quaternion( data.x + other.data.x, data.y + other.data.y, data.z + other.data.z, data.w + other.data.w );
		}

		Quaternion operator-( const Quaternion& other )
		{
			return Quaternion( data.x - other.data.x, data.y - other.data.y, data.z - other.data.z, data.w - other.data.w );
		}

		float Dot( const Quaternion& other ) const
		{
			return data.x*other.data.x + data.y*other.data.y + data.z*other.data.z + data.w*other.data.w;
		}

		Quaternion Normalize() const
		{
			const float norm = Norm();
			const float d = norm > 0 ? norm : 1.0f;

			return Quaternion( data.x / d, data.y / d, data.z / d, data.w / d );
		}

		float Norm() const
		{
			return sqrt( Dot( *this ) );
		}
	};

	struct DualQuaternion
	{
		Quaternion real;
		Quaternion dual;

		DualQuaternion()
			: real( 0, 0, 0, 1 )
			, dual( 0, 0, 0, 0 ) 
		{}

		DualQuaternion( const Quaternion& realPart, const Quaternion& dualPart )
			: real( realPart )
			, dual( dualPart )
		{}

		
		DualQuaternion( const aiVector3D& translation, const aiQuaternion& rotation, const aiVector3D& scale )
		{
			real = Quaternion( rotation.x, rotation.y, rotation.z, rotation.w );
			dual = Quaternion(
				 0.5f*( translation.x*rotation.w + translation.y*rotation.z - translation.z*rotation.y),
				 0.5f*(-translation.x*rotation.z + translation.y*rotation.w + translation.z*rotation.x),
				 0.5f*( translation.x*rotation.y - translation.y*rotation.x + translation.z*rotation.w),
				-0.5f*( translation.x*rotation.x + translation.y*rotation.y + translation.z*rotation.z)
			);
		}

		DualQuaternion operator*( const DualQuaternion& right ) const
		{
			return DualQuaternion( real * right.real, real * right.dual + dual * right.real );
		}

		void Normalize() 
		{
			float realNorm = real.Norm();
			if( realNorm > 0 )
			{
				Quaternion realN = real * (1.f / realNorm);
				Quaternion dualN = dual * (1.f / realNorm);

				real = realN;
				dual = dualN - realN * realN.Dot( dualN );
			}
		}

		operator Matrix3x4() const
		{
			const float rx = real.data.x, ry = real.data.y, rz = real.data.z, rw = real.data.w;
			const float tx = dual.data.x, ty = dual.data.y, tz = dual.data.z, tw = dual.data.w;

			Matrix3x4 result;
			result.data[0][0] = rw*rw + rx*rx - ry*ry - rz*rz;		result.data[0][1] = 2.f*(rx*ry - rw*rz);			result.data[0][2] = 2*(rx*rz + rw*ry);
			result.data[1][0] = 2*(rx*ry + rw*rz);					result.data[1][1] = rw*rw - rx*rx + ry*ry - rz*rz;	result.data[1][2] = 2*(ry*rz - rw*rx);
			result.data[2][0] = 2*(rx*rz - rw*ry);					result.data[2][1] = 2*(ry*rz + rw*rx);				result.data[2][2] = rw*rw - rx*rx - ry*ry + rz*rz;

			result.data[0][3] = -2*tw*rx + 2*rw*tx - 2*ty*rz + 2*ry*tz;
			result.data[1][3] = -2*tw*ry + 2*tx*rz - 2*rx*tz + 2*rw*ty;
			result.data[2][3] = -2*tw*rz + 2*rx*ty + 2*rw*tz - 2*tx*ry;

			return result;
		}
	};

	struct mayaMatrix
	{
		float values[16];
		mayaMatrix( const Matrix& other )
		{
			memcpy( values, other.data.m, sizeof(values) );	
		}

		mayaMatrix( const aiMatrix4x4& other )
		{
			aiMatrix4x4 tmp = other;
			tmp.Transpose();
			memcpy( values, &tmp[0][0], sizeof(values) );
		}

		void print() 
		{
			for( int i=0; i<16; ++i )
				DebugPrint( "%.6f ", values[i] );
			DebugPrint( "\n" );
		}
	};
	
	class Random
	{
	public:
		static void Init( int seed )
		{
			srand( seed ); 
		}

		static float Next()
		{
			return (float)rand() / RAND_MAX;
		}

		template< typename T >
		static T Next( T minInclusive, T maxExclusive )
		{
			float v = Next();
			return (T) (v * (maxExclusive - minInclusive) + minInclusive);
		
		}
	};

	char normalizedFloatToByte( float value );
	int vectorToInt( D3DXVECTOR4 value, D3DXVECTOR4 min, D3DXVECTOR4 max );

	D3DXVECTOR4 Min( D3DXVECTOR4 a, D3DXVECTOR4 b );
	D3DXVECTOR4 Max( D3DXVECTOR4 a, D3DXVECTOR4 b );

	const float Pi = 3.14159265f;

}
#endif
