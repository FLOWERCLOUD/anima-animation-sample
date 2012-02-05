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
			D3DXMatrixTranslation( &result.data, -10.0f, 0.0f, 0.0f );
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

		Matrix operator*( const Matrix& right )
		{
			Matrix result;
			D3DXMatrixMultiply( &result.data, &data, &right.data );
			return result;
		}

		Vector Transform( const Vector& v )
		{
			D3DXVECTOR4 t;
			D3DXVec3Transform( &t, &v.data, &data );
			return Vector(t.x, t.y, t.z);
		}

		Vector TransformNormal( const Vector& n )
		{
			Vector result;
			D3DXVec3TransformNormal( &result.data, &n.data, &data );
			return result;		
		}
	};

	struct Matrix4x3
	{
		float data[4][3];

		operator float*()
		{
			return &data[0][0];
		}

		Matrix4x3()
		{
			memset( data, 0x0, sizeof(data) );
			data[0][0] = 1.f;
			data[1][1] = 1.f;
			data[2][2] = 1.f;
		}


		Matrix4x3( const aiMatrix4x4& other )
		{
			aiMatrix4x4 tmp( other );
			tmp.Transpose();

			memcpy( data[0], &tmp.a1, sizeof(data[0] ) );
			memcpy( data[1], &tmp.b1, sizeof(data[1] ) );
			memcpy( data[2], &tmp.c1, sizeof(data[2] ) );
			memcpy( data[3], &tmp.d1, sizeof(data[3] ) );
		}

		Matrix4x3( const aiMatrix3x3& rotation, const aiVector3D& translation )
		{
			data[0][0] = rotation[0][0];	data[0][1] = rotation[1][0];	data[0][2] = rotation[2][0];
			data[1][0] = rotation[0][1];	data[1][1] = rotation[1][1];	data[1][2] = rotation[2][1];
			data[2][0] = rotation[0][2];	data[2][1] = rotation[1][2];	data[2][2] = rotation[2][2];
			data[3][0] = translation.x;		data[3][1] = translation.y;		data[3][2] = translation.z;
		}

		Matrix4x3 operator*( const Matrix4x3& right ) const
		{
			Matrix4x3 result;
			result.data[0][0] = data[0][0] * right.data[0][0] + data[0][1] * right.data[1][0] + data[0][2] * right.data[2][0];
			result.data[0][1] = data[0][0] * right.data[0][1] + data[0][1] * right.data[1][1] + data[0][2] * right.data[2][1];
			result.data[0][2] = data[0][0] * right.data[0][2] + data[0][1] * right.data[1][2] + data[0][2] * right.data[2][2];

			result.data[1][0] = data[1][0] * right.data[0][0] + data[1][1] * right.data[1][0] + data[1][2] * right.data[2][0];
			result.data[1][1] = data[1][0] * right.data[0][1] + data[1][1] * right.data[1][1] + data[1][2] * right.data[2][1];
			result.data[1][2] = data[1][0] * right.data[0][2] + data[1][1] * right.data[1][2] + data[1][2] * right.data[2][2];

			result.data[2][0] = data[2][0] * right.data[0][0] + data[2][1] * right.data[1][0] + data[2][2] * right.data[2][0];
			result.data[2][1] = data[2][0] * right.data[0][1] + data[2][1] * right.data[1][1] + data[2][2] * right.data[2][1];
			result.data[2][2] = data[2][0] * right.data[0][2] + data[2][1] * right.data[1][2] + data[2][2] * right.data[2][2];

			result.data[3][0] = data[3][0] * right.data[0][0] + data[3][1] * right.data[1][0] + data[3][2] * right.data[2][0] + right.data[3][0];
			result.data[3][1] = data[3][0] * right.data[0][1] + data[3][1] * right.data[1][1] + data[3][2] * right.data[2][1] + right.data[3][1];
			result.data[3][2] = data[3][0] * right.data[0][2] + data[3][1] * right.data[1][2] + data[3][2] * right.data[2][2] + right.data[3][2];

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
