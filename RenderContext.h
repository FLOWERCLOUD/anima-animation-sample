#pragma once

#include "stdafx.h"
#include "math.h"

namespace DisplayList
{
	class Node;
}

/* Context abstraction for rendering: Stores the current view/projection matrices and exposes the d3d device to the application
	RenderFrame() traverses a display list and calls 'render' for the objects in the list	*/
class RenderContext
{
	HWND                    mHWnd;

	LPDIRECT3D9             mD3D;
	LPDIRECT3DDEVICE9       mDevice;
	D3DPRESENT_PARAMETERS	mPresentParameters;	// needed when device is reset
	int mDisplayWidth;
	int mDisplayHeight;

	Math::Matrix mViewMatrix;
	Math::Matrix mProjectionMatrix;

public:
	RenderContext( HWND hWnd, int width, int height );
	~RenderContext(void);

	const D3DPRESENT_PARAMETERS& GetPresentationParameters() const { return mPresentParameters; }
	LPDIRECT3DDEVICE9 Device() { return mDevice; }

	void RenderFrame( DisplayList::Node* displayList );

	void SetViewMatrix( Math::Matrix& newViewMatrix ) { mViewMatrix = newViewMatrix; }
	Math::Matrix GetViewMatrix() { return mViewMatrix; }
	Math::Matrix GetProjectionMatrix() { return mProjectionMatrix; }
};

