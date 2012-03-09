#include "StdAfx.h"
#include "UI.h"
#include "RenderContext.h"
#include "FramerateCounter.h"
#include <stdio.h>


UserInterface::UserInterface( RenderContext* context, FramerateCounter* frameCounter )
	: mFrameCounter( frameCounter )
	, mRenderStatistics( true )
	, mSkeletalAnimationMethod( 0 )
	, mShaderTest( 0 )
{
	TextColor = D3DCOLOR_RGBA(255, 255, 255,255 );
}

UserInterface::~UserInterface()
{
}

void UserInterface::RenderText( TCHAR* text, RECT& rect )
{
	mFont->DrawText( NULL, text, -1, &rect, 0, TextColor );
}

void UserInterface::Render( RenderContext* context )
{
	if( mRenderStatistics )
	{
		RECT rcLine =  { Left,Top, Left + LineWidth, Top + LineHeight };
	
		TCHAR textBuffer[512];  
		float avgFPS = mFrameCounter->GetAverage();

		// Framerate
		sprintf_s( textBuffer, "Frame time: %.2f ms", avgFPS * 1000.0f );
		RenderText( textBuffer, rcLine );

		// skeletal animation method
		rcLine.left += LineWidth;
		rcLine.right += LineWidth;

		const char* animationMethodNames[] = { "Linear Blend Skinning", "Dual Quaternion Skinning" };
		sprintf_s( textBuffer, "Skinning Method: %s", animationMethodNames[mSkeletalAnimationMethod] );
		RenderText( textBuffer, rcLine );

		// shader test
#ifdef DEBUG
		RECT rcShaderTest  =  { Left,Top + LineHeight, Left + LineWidth, Top + 2* LineHeight };
		sprintf_s( textBuffer, "Shader Test: %d", mShaderTest );
		RenderText( textBuffer, rcShaderTest );
#endif


	}
}

void UserInterface::ReleaseResources(  RenderContext* )
{
	if( mFont )
	{
		mFont->Release();
		mFont = 0;
	}
}

void UserInterface::AcquireResources( RenderContext* context )
{
	D3DXCreateFont( 
		context->Device(), 30, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
		TEXT("Arial"), &mFont 
	);
}


