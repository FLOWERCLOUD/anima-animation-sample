#include "Camera.h"
#include "Input.h"
#include <iostream>

#define KEY_DOWN 0x8000 
#define MOVEMENT_SPEED 0.1f
#define ROTATION_SPEED 0.150f

Camera::Camera( const Input& input )
	: m_Azimuth( Math::Pi/2.f )
	, m_Zenith( Math::Pi/2.f )
	, m_Input( input )
{
	m_Projection = Math::Matrix::Perspective( 45.0f, 1024.f / 768.f, 1.0f, 1000.0f );
	m_Position = Math::Vector( 0, 0, 0 );
	GetCursorPos( &m_LastCursorPos );
}


Camera::~Camera(void)
{
}


void Camera::update( float dt )
{
	int _up = GetKeyState( VK_UP ) & KEY_DOWN;
	int _down = GetKeyState( VK_DOWN ) & KEY_DOWN;
	int _left = GetKeyState( VK_LEFT ) & KEY_DOWN;
	int _right = GetKeyState( VK_RIGHT ) & KEY_DOWN;
 
		/*
	bool _up = m_Input.GetKey( VK_UP );
	bool _down =  m_Input.GetKey(VK_DOWN );
	bool _left =  m_Input.GetKey( VK_LEFT );
	bool _right =  m_Input.GetKey( VK_RIGHT );
 */

	float dAzimuth = -m_Input.GetMouse().x * dt * ROTATION_SPEED;
	float dZenith = m_Input.GetMouse().y * dt * ROTATION_SPEED;;
	

	m_Azimuth += dAzimuth;
	m_Zenith += dZenith;

	
	const float cA = cos( m_Azimuth );
	const float sA = sin( m_Azimuth );

	const float cZ = cos( m_Zenith );
	const float sZ = sin( m_Zenith );

	Math::Vector forward(
		cA * sZ,  
		cZ,
		sA * sZ
	);
	/*
	Math::Vector up( 
		0.f,
		sZ,
		cZ
	);
	*/

	Math::Vector up(
		cA * cZ,
		sZ,
		sA * cZ
	);



	Math::Vector left = forward.Cross( up );

	Math::Vector s = left.Scale( _left ? 1.0f : ( _right ? -1.0f : 0.0f ) ).Scale( dt * MOVEMENT_SPEED );
	Math::Vector f = forward.Scale( _up ? 1.0f : ( _down ? -1.0f : 0.0f ) ).Scale( dt * MOVEMENT_SPEED );
	
	m_Position += s + f;
	
	m_View =  Math::Matrix::LookAt( m_Position, m_Position + forward, up );
	std::cout << m_Position.GetX() << ", " << m_Position.GetY() << ", " << m_Position.GetZ() << std::endl;
	//m_LastCursorPos = curCursorPos;

}

