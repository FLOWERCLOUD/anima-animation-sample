#if !defined(__MATH_H__)
#define __MATH_H__

#include "stdafx.h"
#include <D3dx9math.h>

/* Simple math functionality. Wraps d3dx math functions, the idea was that the same code can be used across the d3d and opengl build
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
			D3DXMATRIX tmp( &other.a1 );
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
