#include "stdafx.h"
#include "math.h"

using namespace Math;

/////////////////// Vector ///////////////////////////////////
Vector::Vector() 
	: data(0,0,0)
{}
		
Vector::Vector( const aiVector3D& other )
{
	data.x = other.x;
	data.y = other.y;
	data.z = other.z;
}

Vector::Vector( float x, float y, float z ) 
	: data(x,y,z)
{}

float Vector::GetX() const
{
	return data.x; 
}

float Vector::GetY() const 
{ 
	return data.y; 
}

float Vector::GetZ() const 
{ 
	return data.z; 
}

Vector::operator D3DXVECTOR3() 
{ 
	return data;
}

Vector::operator D3DXVECTOR4() 
{ 
	return D3DXVECTOR4( GetX(), GetY(), GetZ(), 1.0f ); 
}

Vector Vector::Normal() const
{
	Vector result;
	D3DXVec3Normalize( &result.data, &data );
	return result;
}

Vector Vector::operator-( const Vector& other ) const
{
	return Vector( GetX() - other.GetX(), GetY() - other.GetY(), GetZ() - other.GetZ() );
}

Vector Vector::operator+( const Vector& other ) const
{
	return Vector( GetX() + other.GetX(), GetY() + other.GetY(), GetZ() + other.GetZ() );
}

Vector Vector::operator+=( const Vector& other ) 
{
	*this = *this + other;
	return *this;
}

Vector Vector::Scale( const float scale ) const
{
	return Vector( GetX() * scale, GetY() * scale, GetZ() * scale );
}

Vector Vector::Cross( const Vector& other ) const
{
	Vector result;
	D3DXVec3Cross( &result.data, &data, &other.data );
	return result;	
}

float Vector::Dot( const Vector& other ) const
{
	return D3DXVec3Dot( &data, &other.data );
}

float Vector::Dot( float* a, float* b )
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

Vector Vector::Interpolate( const Vector& a, const Vector& b, float t )
{
	return a.Scale( 1.f - t ) + b.Scale( t );
}


/////////////////// Matrix ///////////////////////////////////

Matrix Matrix::RotationYawPitchRoll( Vector yawPitchRoll )
{
	Matrix result;
	D3DXMatrixRotationYawPitchRoll( &result.data, yawPitchRoll.GetX(), yawPitchRoll.GetY(), yawPitchRoll.GetZ() );
	return result;
}

Matrix Matrix::Translation( Vector translation )
{
	Matrix result;
	D3DXMatrixTranslation( &result.data, translation.GetX(), translation.GetY(), translation.GetZ() );
	return result;

}

Matrix Matrix::Scale( Vector scale )
{
	Matrix result;
	D3DXMatrixScaling( &result.data, 0.5f, 0.5f, 0.5f );
	return result;
}

Matrix Matrix::LookAt( const Vector& eye, const Vector& lookAt, const Vector& up )
{
	Matrix result;
	D3DXMatrixLookAtLH( &result.data, &eye.data, &lookAt.data, &up.data );
	return result;

}

Matrix Matrix::Perspective( float fov, float aspect, float n, float f )
{
	Matrix result;
	D3DXMatrixPerspectiveFovLH( &result.data, D3DXToRadian( fov ), aspect, n, f );
	return result;
}

Matrix::Matrix() 
{}

Matrix::Matrix( const aiMatrix4x4& other )
{
	D3DXMATRIX tmp( &other[0][0] );
	D3DXMatrixTranspose( &data, &tmp );	
}

Matrix Matrix::operator*( const Matrix& right ) const
{
	Matrix result;
	D3DXMatrixMultiply( &result.data, &data, &right.data );
	return result;
}

Matrix Matrix::Inverse() const
{
	Matrix result;
	D3DXMatrixInverse( &result.data, NULL, &data );
	return result;
}

Matrix Matrix::Transpose() const
{
	Matrix result;
	D3DXMatrixTranspose( &result.data, &data );
	return result;
}

Vector Matrix::Transform( const Vector& v ) const
{
	D3DXVECTOR4 t;
	D3DXVec3Transform( &t, &v.data, &data );
	return Vector(t.x, t.y, t.z);
}

Vector Matrix::TransformNormal( const Vector& n ) const
{
	Vector result;
	D3DXVec3TransformNormal( &result.data, &n.data, &data );
	return result;		
}


/////////////////// Matrix3x4 ///////////////////////////////////


Matrix3x4::operator float*()
{
	return &data[0][0];
}

Matrix3x4::Matrix3x4()
{
	memset( data, 0x0, sizeof(data) );
	data[0][0] = 1.f;
	data[1][1] = 1.f;
	data[2][2] = 1.f;
}

Matrix3x4::Matrix3x4(	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23 )
{
	data[0][0] = m00;	data[0][1] = m01;	data[0][2] = m02;	data[0][3] = m03;
	data[1][0] = m10;	data[1][1] = m11;	data[1][2] = m12;	data[1][3] = m13;
	data[2][0] = m20;	data[2][1] = m21;	data[2][2] = m22;	data[2][3] = m23;
}


Matrix3x4::Matrix3x4( const aiMatrix4x4& other )
{
	memcpy( &data[0][0], &other[0][0], sizeof(Matrix3x4) );
}

Matrix3x4::Matrix3x4( const aiMatrix3x3& other )
{
	memcpy( data[0], &other.a1, 3 * sizeof(float) );
	memcpy( data[1], &other.b1, 3 * sizeof(float) );
	memcpy( data[2], &other.c1, 3 * sizeof(float) );

	data[0][3] = 0; data[1][3] = 0; data[2][3] = 0;
}

Matrix3x4::Matrix3x4( const D3DXMATRIX& other )
{
	data[0][0] = other(0,0);	data[0][1] = other(1,0);	data[0][2] = other(2,0);	data[0][3] = other(3,0);
	data[1][0] = other(0,1);	data[1][1] = other(1,1);	data[1][2] = other(2,1);	data[1][3] = other(3,1);
	data[2][0] = other(0,2);	data[2][1] = other(1,2);	data[2][2] = other(2,2);	data[2][3] = other(3,2);
}

Matrix3x4::Matrix3x4( const Vector& translation, const Quaternion& rotation, const Vector& scale )
{
	*this = rotation;
	
	data[0][0] *= scale.GetX();			data[1][1] *= scale.GetY();			data[2][2] *= scale.GetZ();
	data[0][3] = translation.GetX();	data[1][3] = translation.GetY();	data[2][3] = translation.GetZ();
}

Matrix3x4 Matrix3x4::operator*( const Matrix3x4& right ) const
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

float Matrix3x4::Determinant() const
{
	return 
		data[0][0]*data[1][1]*data[2][2] + 
		data[0][1]*data[1][2]*data[2][0] +
		data[0][2]*data[1][0]*data[2][1] -
		data[0][2]*data[1][1]*data[2][0] -
		data[0][1]*data[1][0]*data[2][2] -
		data[0][0]*data[1][2]*data[2][1];
}

Matrix3x4::operator Quaternion() const
{
	Quaternion result;
	if ( data[0][0] + data[1][1] + data[2][2] > 0.0f ) 
	{
		float t = + data[0][0] + data[1][1] + data[2][2] + 1.0f; 
		float s = 1.f / sqrt( t ) * 0.5f;

		result.data.w = s * t;
		result.data.z = ( data[0][1] - data[1][0] ) * s; 
		result.data.y = ( data[2][0] - data[0][2] ) * s; 
		result.data.x = ( data[1][2] - data[2][1] ) * s;
	} 
	else if ( data[0][0] > data[1][1] && data[0][0] > data[2][2] ) 
	{
		float t = + data[0][0] - data[1][1] - data[2][2] + 1.0f; 
		float s = 1.0f / sqrt( t ) * 0.5f;

		result.data.x = s * t;
		result.data.y = ( data[0][1] + data[1][0] ) * s;
		result.data.z = ( data[2][0] + data[0][2] ) * s; 
		result.data.w = ( data[1][2] - data[2][1] ) * s;
	}
	else if ( data[1][1] > data[2][2] ) 
	{
		float t = - data[0][0] + data[1][1] - data[2][2] + 1.0f; 
		float s = 1.0f / sqrt( t ) * 0.5f;
		result.data.y = s * t;
		result.data.x = ( data[0][1] + data[1][0] ) * s; 
		result.data.w = ( data[2][0] - data[0][2] ) * s; 
		result.data.z = ( data[1][2] + data[2][1] ) * s;
	}
	else 
	{
		float t = - data[0][0] - data[1][1] + data[2][2] + 1.0f; 
		float s = 1.0f / sqrt( t ) * 0.5f;

		result.data.z = s * t;
		result.data.w = ( data[0][1] - data[1][0] ) * s;
		result.data.x = ( data[2][0] + data[0][2] ) * s;
		result.data.y = ( data[1][2] + data[2][1] ) * s;
	}

	return result;
}

void Matrix3x4::Print() const
{
	const float* values = &data[0][0];
	for( int i=0; i<12; ++i )
		DebugPrint( "%.6f ", values[i] );
	DebugPrint( "\n" );
}

/////////////////// Quaternion ///////////////////////////////////
Quaternion::Quaternion()
	: data( 0, 0, 0, 1 )
{}

Quaternion::Quaternion( float x, float y, float z, float w )
	: data( x, y, z, w )
{}

Quaternion::Quaternion( const D3DXQUATERNION& other )
	: data( other )
{}

Quaternion::Quaternion( const aiQuaternion& other )
	: data( other.x, other.y, other.z, other.w )
{}

Quaternion::Quaternion( const Quaternion& other )
	: data( other.data )
{}


/* Painfully slow reference implementation */
Quaternion Quaternion::operator*( const Quaternion& right ) const
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

Quaternion Quaternion::operator*( float factor ) const
{
	return  Quaternion( data.x * factor, data.y * factor, data.z * factor, data.w * factor );
}

Quaternion Quaternion::operator+( const Quaternion& other ) const
{
	return Quaternion( data.x + other.data.x, data.y + other.data.y, data.z + other.data.z, data.w + other.data.w );
}

Quaternion Quaternion::operator-( const Quaternion& other ) const
{
	return Quaternion( data.x - other.data.x, data.y - other.data.y, data.z - other.data.z, data.w - other.data.w );
}

float Quaternion::Dot( const Quaternion& other ) const
{
	return data.x*other.data.x + data.y*other.data.y + data.z*other.data.z + data.w*other.data.w;
}

Quaternion Quaternion::Normalize() const
{
	const float norm = Norm();
	const float d = norm > 0 ? norm : 1.0f;

	return Quaternion( data.x / d, data.y / d, data.z / d, data.w / d );
}

float Quaternion::Norm() const
{
	return sqrt( Dot( *this ) );
}

Quaternion::operator Matrix3x4() const
{
	// this method is optimized for unit quaternions only
	float n = Norm();
	assert( abs( n - 1.f ) < 0.0001f );

	Matrix3x4 result;
	const float x = data.x, y = data.y, z = data.z, w = data.w;

	result.data[0][0] = 1 - 2*(y*y + z*z);		result.data[0][1] = 2*(x*y - w*z);			result.data[0][2] = 2*(x*z + w*y);
	result.data[1][0] = 2*(x*y + w*z);			result.data[1][1] = 1 - 2*(x*x + z*z);		result.data[1][2] = 2*(y*z - w*x);
	result.data[2][0] = 2*(x*z - w*y);			result.data[2][1] = 2*(y*z + w*x);			result.data[2][2] = 1 - 2*(x*x + y*y);


	return result;
}

Quaternion Quaternion::Interpolate( const Quaternion& a, const Quaternion& b, float t )
{
	Quaternion result;
	D3DXQuaternionSlerp( &result.data, &a.data, &b.data, t );
	return result;
}


/////////////////// Quaternion ///////////////////////////////////


DualQuaternion::DualQuaternion()
	: real( 0, 0, 0, 1 )
	, dual( 0, 0, 0, 0 ) 
{}

DualQuaternion::DualQuaternion( const Quaternion& realPart, const Quaternion& dualPart )
	: real( realPart )
	, dual( dualPart )
{}

DualQuaternion::DualQuaternion( const Vector& translation, const Quaternion& rotation, const Vector& scale )
{
	const float qx = rotation.data.x, qy = rotation.data.y, qz = rotation.data.z, qw = rotation.data.w;
	const float tx = translation.GetX(), ty = translation.GetY(), tz = translation.GetZ();

	real = Quaternion( qx, qy, qz, qw );
	dual = Quaternion(
		 0.5f*( tx*qw + ty*qz - tz*qy),
		 0.5f*(-tx*qz + ty*qw + tz*qx),
		 0.5f*( tx*qy - ty*qx + tz*qw),
		-0.5f*( tx*qx + ty*qy + tz*qz)
	);
}

DualQuaternion DualQuaternion::operator*( const DualQuaternion& right ) const
{
	return DualQuaternion( real * right.real, real * right.dual + dual * right.real );
}

DualQuaternion DualQuaternion::operator*( float c ) const
{
	return DualQuaternion( real*c, dual*c );
}

void DualQuaternion::Normalize() 
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

DualQuaternion::operator Matrix3x4() const
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

/////////////////// Random ///////////////////////////////////

void Random::Init( int seed )
{
	srand( seed ); 
}

float Random::Next()
{
	return (float)rand() / RAND_MAX;
}

/////////////////// Misc ///////////////////////////////////

char Math::normalizedFloatToByte( float value )
{
	int v = static_cast<int>( value * 255.0f );
	return max( min( v, 255 ), 0 );
}

int Math::vectorToInt( D3DXVECTOR4 value, D3DXVECTOR4 min, D3DXVECTOR4 max )
{
	return 
		normalizedFloatToByte( (value.x - min.x) / (max.x - min.x)  ) << 24 ||
		normalizedFloatToByte( (value.y - min.y) / (max.y - min.y) ) << 16 ||
		normalizedFloatToByte( (value.z - min.z) / (max.z - min.z) ) << 8 ||
		normalizedFloatToByte( (value.w - min.w) / (max.w - min.w) );

}


D3DXVECTOR4 Math::Min( D3DXVECTOR4 a, D3DXVECTOR4 b )
{
	D3DXVECTOR4 result;

	result.x = min( a.x, b.x );
	result.y = min( a.y, b.y );
	result.z = min( a.z, b.z );
	result.w = min( a.w, b.w );

	return result;
}

D3DXVECTOR4 Math::Max( D3DXVECTOR4 a, D3DXVECTOR4 b )
{
	D3DXVECTOR4 result;

	result.x = max( a.x, b.x );
	result.y = max( a.y, b.y );
	result.z = max( a.z, b.z );
	result.w = max( a.w, b.w );

	return result;
}
