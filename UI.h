#pragma once

#include "stdafx.h"
#include "DisplayList.h"

class RenderContext;
class CubeRendererInterface;
class FramerateCounter;

/* Simple class for managing the 'user interface'. Basically just renders some statistics to the screen	*/
class UserInterface : public DisplayList::Node
{
	FramerateCounter* mFrameCounter;
	LPD3DXFONT mFont;

	// Top left corner of text rendering (pixel coordinates)
	static const int Top = 10;
	static const int Left = 10;

	// Width and height of each line of text
	static const int LineHeight = 30;
	static const int LineWidth = 500;

	D3DCOLOR TextColor;
	bool mRenderStatistics;

	// helper function to abstract opengl and dx9
	void RenderText( TCHAR* text, RECT& rect );

public:
	UserInterface( RenderContext* context, FramerateCounter* frameCounter );
	virtual ~UserInterface(void);

	void Render( RenderContext* context );
	void ReleaseResources( RenderContext* context );
	void AcquireResources( RenderContext* context );
	void ToggleStatistics() { mRenderStatistics = !mRenderStatistics; }

};

