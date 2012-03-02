#if !defined(__MATH_H__)
#define __MATH_H__

#include "stdafx.h"
#include <D3dx9math.h>
#include "Debug.h"

namespace Math
{
	struct Quaternion;

	// This class should probably be refactored to a 4 dimensional vector. 
	struct Vector
	{
		D3DXVECTOR3 data;
		Vector();
		Vector( const aiVector3D& other );
		Vector( float x, float y, float z );

		float GetX() const;
		float GetY() const;
		float GetZ() const;

		operator D3DXVECTOR3();
		operator D3DXVECTOR4();

		Vector Normal() const;
	
		Vector operator-( const Vector& other ) const;
		Vector operator+( const Vector& other ) const;
		Vector operator+=( const Vector& other );

		Vector Scale( const float scale ) const;
		Vector Cross( const Vector& other ) const;
		float Dot( const Vector& other ) const;
		static float Dot( float* a, float* b );

		static Vector Interpolate( const Vector& a, const Vector& b, float t );
		
	};

	/* 4x4 Matrix, DirectX layout: column major with translation elements in the fourth row (=>m[4][0],..m[4][3]) */
	struct Matrix
	{
		D3DXMATRIX data;

		static Matrix RotationYawPitchRoll( Vector yawPitchRoll );
		static Matrix Translation( Vector translation );
		static Matrix Scale( Vector scale );
		static Matrix LookAt( const Vector& eye, const Vector& lookAt, const Vector& up );
		static Matrix Perspective( float fov, float aspect, float n, float f );
		
		Matrix();
		Matrix( const aiMatrix4x4& other );

		Matrix operator*( const Matrix& right ) const;
		
		Matrix Inverse() const;
		Matrix Transpose() const;
		
		Vector Transform( const Vector& v ) const;
		Vector TransformNormal( const Vector& n ) const;
	};

	/* NOTE: this matrix is meant to be used for POST multiplication, e.g. Matrix * vector as opposed of DirectX's default vector * Matrix.
	* the Matrix layout has been defined like this on purpose so individual columns can be passed directly to the shader (hlsl uses column-major 
	* matrices by default) */
	struct Matrix3x4
	{
		float data[3][4];

		Matrix3x4();
		Matrix3x4(	float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23 );

		Matrix3x4( const aiMatrix4x4& other );
		Matrix3x4( const aiMatrix3x3& other );
		Matrix3x4( const D3DXMATRIX& other );
		Matrix3x4( const Vector& translation, const Quaternion& rotation, const Vector& scale );
		Matrix3x4 operator*( const Matrix3x4& right ) const;

		operator Quaternion() const;
		operator float*();		

		float Determinant() const;
		void Print() const;
	
	};

	struct Quaternion
	{
		D3DXQUATERNION data; // rx, ry, rz, s

		Quaternion();
		Quaternion( float x, float y, float z, float w );
		Quaternion( const D3DXQUATERNION& other );
		Quaternion( const aiQuaternion& other );
		Quaternion( const Quaternion& other );

		Quaternion operator*( const Quaternion& right ) const;
		Quaternion operator*( float factor ) const;
		Quaternion operator+( const Quaternion& other ) const;
		Quaternion operator-( const Quaternion& other ) const;
		
		operator Matrix3x4() const;		
		
		float Dot( const Quaternion& other ) const;
		float Norm() const;

		Quaternion Normalize() const;
		static Quaternion Interpolate( const Quaternion& a, const Quaternion& b, float t );
	};

	struct DualQuaternion
	{
		Quaternion real;
		Quaternion dual;

		DualQuaternion();
		DualQuaternion( const Quaternion& realPart, const Quaternion& dualPart );
		DualQuaternion( const Vector& translation, const Quaternion& rotation, const Vector& scale );

		DualQuaternion operator*( const DualQuaternion& right ) const;
		operator Matrix3x4() const;

		void Normalize();
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
		static void Init( int seed );
		static float Next();

		template< typename T >
		static T Next( T minInclusive, T maxExclusive )
		{
			float v = Next();
			return (T) (v * (maxExclusive - minInclusive) + minInclusive);
		
		}
	};

	template< class T >
	struct Interpolator
	{
		T operator()( const T& a, const T& b, float t )
		{
			return T::Interpolate( a, b, t );
		}
	};

	char normalizedFloatToByte( float value );
	int vectorToInt( D3DXVECTOR4 value, D3DXVECTOR4 min, D3DXVECTOR4 max );

	D3DXVECTOR4 Min( D3DXVECTOR4 a, D3DXVECTOR4 b );
	D3DXVECTOR4 Max( D3DXVECTOR4 a, D3DXVECTOR4 b );

	const float Pi = 3.14159265f;

	template< class T >
	const T& Clamp( const T& value, const T& minValue, const T& maxValue )
	{
		return value < minValue ? minValue : (value > maxValue ? maxValue : value);
	}

}
#endif
