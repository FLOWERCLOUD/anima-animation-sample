#include "StdAfx.h"
#include "RenderContext.h"
#include "DisplayList.h"
#include "Cubes.h"

RenderContext::RenderContext( HWND hWnd, int width, int height )
{
	mHWnd = hWnd;

	mD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

	DX_CHECK( mD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) );

    ZeroMemory( &mPresentParameters, sizeof(mPresentParameters) );

    mPresentParameters.Windowed               = TRUE;
    mPresentParameters.SwapEffect             = D3DSWAPEFFECT_FLIP;// D3DSWAPEFFECT_DISCARD;
    mPresentParameters.BackBufferFormat       = d3ddm.Format;
    mPresentParameters.EnableAutoDepthStencil = TRUE;
    mPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
    mPresentParameters.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	mPresentParameters.MultiSampleType		   = D3DMULTISAMPLE_NONE;

	// use perfhud if possible
	{
		UINT AdapterToUse=D3DADAPTER_DEFAULT; 
		D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL; 
 
		for (UINT Adapter=0;Adapter<mD3D->GetAdapterCount();Adapter++)  
		{ 
			D3DADAPTER_IDENTIFIER9  Identifier; 

			DX_CHECK( mD3D->GetAdapterIdentifier(Adapter,0,&Identifier) ); 
			if (strstr(Identifier.Description,"PerfHUD") != 0) 
			{ 
				AdapterToUse=Adapter; 
				DeviceType=D3DDEVTYPE_REF; 
				break; 
			} 
		} 
 
		DX_CHECK( mD3D->CreateDevice( AdapterToUse, DeviceType, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &mPresentParameters, &mDevice ) );
	}

	mViewMatrix = Math::Matrix::LookAt( Math::Vector(0,0,-70), Math::Vector( 0, 0, 0 ), Math::Vector(0,1,0) );
	mProjectionMatrix = Math::Matrix::Perspective( 45.0f, ((float)width)/height, 1.0f, 1000.0f );
}

RenderContext::~RenderContext(void)
{ 
	if( mDevice != NULL )
	{
       mDevice->Release();
	   mDevice = 0;
	}

    if( mD3D != NULL )
	{
        mD3D->Release();
	   mD3D = 0;
	}	
}

void RenderContext::RenderFrame( DisplayList::Node* displayList )
{
	// check for lost device first
	{
		HRESULT coop = mDevice->TestCooperativeLevel();

		switch(coop)
		{
		case D3DERR_DEVICELOST: 
			return;

		case D3DERR_DEVICENOTRESET:

			AnimaApplication::Instance()->OnDeviceLost();
		
			if( FAILED( mDevice->Reset( &mPresentParameters ) ) )
			{
				OutputDebugString( TEXT( "Device Reset failed\n" ) );
				return;
			}

			AnimaApplication::Instance()->OnDeviceReset();
			break;
		}
	}


	// clear frame buffer and render all objects in display list
	{
		mDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
							 D3DCOLOR_RGBA(100, 149, 237,255), 1.0f, 0 );

		mDevice->BeginScene();
	
		DisplayList::Node* cur = displayList;
		while( cur )
		{
			cur->Render( this );
			cur = cur->Next();
		}
	
		mDevice->EndScene();
		mDevice->Present( NULL, NULL, NULL, NULL );
	}
 }
