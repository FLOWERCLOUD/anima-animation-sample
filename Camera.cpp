#include "Camera.h"
#include "Input.h"
#include <iostream>
#include "RenderContext.h"

#define KEY_DOWN 0x8000 
#define MOVEMENT_SPEED 5.f
#define ROTATION_SPEED 2.5f

Camera::Camera( const Input& input, const RenderContext& renderContext )
	: mAzimuth( Math::Pi/2.f )
	, mZenith( Math::Pi/2.f + 10.f / 180.f * Math::Pi )
	, mInput( input )
{
	float w = static_cast<float>( renderContext.GetPresentationParameters().BackBufferWidth );
	float h = static_cast<float>( renderContext.GetPresentationParameters().BackBufferHeight );

	mProjection = Math::Matrix::Perspective( 45.0f, w / h, 1.0f, 1000.0f );
	mPosition = Math::Vector( 0, 5, -10 );
	GetCursorPos( &mLastCursorPos );
}


Camera::~Camera(void)
{
}


void Camera::update( float dt )
{
	bool _up = mInput.IsKeyPressed( VK_UP );
	bool _down =  mInput.IsKeyPressed(VK_DOWN );
	bool _left =  mInput.IsKeyPressed( VK_LEFT );
	bool _right =  mInput.IsKeyPressed( VK_RIGHT );
	short _deltaWheel = mInput.GetWheelDelta();

	float dAzimuth = -mInput.GetMouse().x * dt * ROTATION_SPEED;
	float dZenith = mInput.GetMouse().y * dt * ROTATION_SPEED;;
	
	mAzimuth += dAzimuth;
	mZenith = Math::Clamp( mZenith+dZenith, 0.f, Math::Pi );
	
	const float cA = cos( mAzimuth );
	const float sA = sin( mAzimuth );

	const float cZ = cos( mZenith );
	const float sZ = sin( mZenith );

	Math::Vector forward(
		cA * sZ,  
		cZ,
		sA * sZ
	);

	Math::Vector up(
		-cA * cZ,
		sZ,
		-sA * cZ
	);

	Math::Vector left = forward.Cross( up );

	Math::Vector s = left.Scale( _left ? 1.0f : ( _right ? -1.0f : 0.0f ) ).Scale( dt * MOVEMENT_SPEED );

	float _scaleFactor = ( _deltaWheel ? _deltaWheel/2.f : _up ? 1.0f : _down ? -1.0f : 0.0f);
	Math::Vector f = forward.Scale( _scaleFactor ).Scale( dt * MOVEMENT_SPEED );
	mPosition += s+f;
	
	mView =  Math::Matrix::LookAt( mPosition, mPosition + forward, up );
}

