#include "StdAfx.h"
#include "RenderContext.h"
#include "DisplayList.h"
#include "Cubes.h"

RenderContext::RenderContext( HWND hWnd, int width, int height )
{
	m_hWnd = hWnd;

	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

	DX_CHECK( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) );

    ZeroMemory( &m_PresentParameters, sizeof(m_PresentParameters) );

    m_PresentParameters.Windowed               = TRUE;
    m_PresentParameters.SwapEffect             = D3DSWAPEFFECT_FLIP;// D3DSWAPEFFECT_DISCARD;
    m_PresentParameters.BackBufferFormat       = d3ddm.Format;
    m_PresentParameters.EnableAutoDepthStencil = TRUE;
    m_PresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
    m_PresentParameters.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_PresentParameters.MultiSampleType		   = D3DMULTISAMPLE_NONE;

	// use perfhud if possible
	{
		UINT AdapterToUse=D3DADAPTER_DEFAULT; 
		D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL; 
 
		for (UINT Adapter=0;Adapter<m_pD3D->GetAdapterCount();Adapter++)  
		{ 
			D3DADAPTER_IDENTIFIER9  Identifier; 

			DX_CHECK( m_pD3D->GetAdapterIdentifier(Adapter,0,&Identifier) ); 
			if (strstr(Identifier.Description,"PerfHUD") != 0) 
			{ 
				AdapterToUse=Adapter; 
				DeviceType=D3DDEVTYPE_REF; 
				break; 
			} 
		} 
 
		DX_CHECK( m_pD3D->CreateDevice( AdapterToUse, DeviceType, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_PresentParameters, &m_pDevice ) );
	}

	m_ViewMatrix = Math::Matrix::LookAt( Math::Vector(0,0,-70), Math::Vector( 0, 0, 0 ), Math::Vector(0,1,0) );
	m_ProjectionMatrix = Math::Matrix::Perspective( 45.0f, ((float)width)/height, 1.0f, 1000.0f );
}

RenderContext::~RenderContext(void)
{ 
	if( m_pDevice != NULL )
	{
       m_pDevice->Release();
	   m_pDevice = 0;
	}

    if( m_pD3D != NULL )
	{
        m_pD3D->Release();
	   m_pD3D = 0;
	}	
}

void RenderContext::RenderFrame( DisplayList::Node* displayList )
{
	// check for lost device first
	{
		HRESULT coop = m_pDevice->TestCooperativeLevel();

		switch(coop)
		{
		case D3DERR_DEVICELOST: 
			return;

		case D3DERR_DEVICENOTRESET:

			AnimaApplication::Instance()->OnDeviceLost();
		
			if( FAILED( m_pDevice->Reset( &m_PresentParameters ) ) )
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
		m_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
							 D3DCOLOR_RGBA(100, 149, 237,255), 1.0f, 0 );

		m_pDevice->BeginScene();
	
		DisplayList::Node* cur = displayList;
		while( cur )
		{
			cur->Render( this );
			cur = cur->Next();
		}
	
		m_pDevice->EndScene();
		m_pDevice->Present( NULL, NULL, NULL, NULL );
	}
 }
