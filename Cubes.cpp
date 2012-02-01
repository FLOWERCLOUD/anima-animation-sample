#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>

#include "resource.h"
#include "Camera.h"

#include "Cubes.h"
#include "RenderContext.h"
#include "FramerateCounter.h"
#include "UI.h"
#include <assert.h>
#include <Shellapi.h>
#include "TestEnvironment.h"
#include "Model.h"
#include "CubeRenderer0.h"
#include "Input.h"


#ifndef HID_USAGE_PAGE_GENERIC
	#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
	#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	
	AnimaApplication::CreateInstance( hInstance, hPrevInstance, lpCmdLine, nCmdShow );
	AnimaApplication::Instance()->Run();
	AnimaApplication::DestroyInstance();  

	return 0;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	return AnimaApplication::Instance()->OnMessage( hWnd, msg, wParam, lParam );
}

AnimaApplication* AnimaApplication::m_pInstance = 0;

AnimaApplication::~AnimaApplication()
{
	if( m_pInstance )
	{
		UnregisterClass( "MY_WINDOWS_CLASS", m_WindowClass.hInstance );

		m_pUserInterface->ReleaseResources( m_pRenderContext );
		delete m_pUserInterface;

	//	m_pCubeRenderer->ReleaseResources( m_pRenderContext );
	//	delete m_pCubeRenderer;

		delete m_pFramerateCounter;
		delete m_pRenderContext;
	}
}

void GLUT_OnKeyUp( unsigned char key, int x, int y )
{
}

void GLUT_OnRenderFrame()
{
	AnimaApplication::Instance()->NextFrame();
}

HRESULT AnimaApplication::CreateInstance( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	assert( AnimaApplication::m_pInstance == 0 );

	HWND	   hWnd;
	WNDCLASSEX winClass;
	memset( &winClass, 0x0, sizeof(WNDCLASSEX) );
	
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "DX9 Smiling Cubes",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 1024, 768, NULL, NULL, hInstance, NULL );

	if( hWnd == NULL )
		return E_FAIL;

	// register raw input device
	RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    Rid[0].dwFlags = RIDEV_INPUTSINK;   
    Rid[0].hwndTarget = hWnd;
    RegisterRawInputDevices( Rid, 1, sizeof(Rid[0]) );


	ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

	LPWSTR *szArglist;
	int nArgs;
	
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	TestEnvironment* testEnvironment = new TestEnvironment( nArgs, szArglist );
	Math::Random::Init( testEnvironment->GetRandomSeed() );

	AnimaApplication::m_pInstance = new AnimaApplication( winClass, hWnd );

	Instance()->m_pTestEnvironment = testEnvironment;
	Instance()->m_pRenderContext = new RenderContext( hWnd, DISPLAY_WIDTH, DISPLAY_HEIGHT );
	Instance()->m_pFramerateCounter = new FramerateCounter;

	Instance()->m_pCubeRenderer = new CubeRenderer0( m_pInstance->m_pRenderContext );
	Instance()->m_pCubeRenderer->AcquireResources( Instance()->m_pRenderContext );
	Instance()->m_pCubeRenderer->AddCubes( 10000 );

	Instance()->m_pUserInterface = new UserInterface( m_pInstance->m_pRenderContext, m_pInstance->m_pCubeRenderer, m_pInstance->m_pFramerateCounter );
	Instance()->m_pUserInterface->AcquireResources( Instance()->m_pRenderContext );

	Instance()->m_pCubeRenderer->SetNext( m_pInstance->m_pUserInterface );
	Instance()->m_pInput = new Input();
	Instance()->m_pCamera = new Camera( *Instance()->m_pInput );
	// set up renderer 

	
	Instance()->m_Model = new Model( "C:\\Users\\Theo\\Desktop\\cubes.dae" );
	Instance()->m_Model->load( Instance()->m_pRenderContext );
	Instance()->m_Model->SetNext( Instance()->m_pCubeRenderer );
	return S_OK;
}

void AnimaApplication::DestroyInstance()
{
	assert( AnimaApplication::m_pInstance );
	delete AnimaApplication::m_pInstance;
	AnimaApplication::m_pInstance = 0;
}


void AnimaApplication::Run()
{
	MSG        uMsg;
    memset(&uMsg,0,sizeof(uMsg));

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
        {
			NextFrame(); 
		}
	}
}

void AnimaApplication::NextFrame()
{
	m_DeltaTime.Update();

	m_pInput->Update( m_DeltaTime.Elapsed() );

	m_pFramerateCounter->FrameStart();

	m_pCamera->update( m_DeltaTime.Elapsed() );
	m_pCubeRenderer->Update( m_DeltaTime.Elapsed() );
	m_Model->Update( m_DeltaTime.Elapsed() );

	m_pRenderContext->SetViewMatrix( m_pCamera->ViewMatrix() );
	m_pRenderContext->RenderFrame( m_Model );

	m_pFramerateCounter->FrameEnd();
	
//	Sleep(10);
}

LRESULT AnimaApplication::OnMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{	

        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
				case 0x51: // 'Q'
					PostQuitMessage(0);
					break;

					/*
				case VK_LEFT:
					if( !m_pTestEnvironment->IsValid() )
						m_pCubeRenderer->RemoveCubes( 1000 );
					break;
				case VK_RIGHT:
					if( !m_pTestEnvironment->IsValid() )
						m_pCubeRenderer->AddCubes( 1000 );
					break;

				case VK_UP:
					if( !m_pTestEnvironment->IsValid() )
						m_pCubeRenderer->SetScale( m_pCubeRenderer->GetScale() + 0.5f );
					break;

				case VK_DOWN:
					if( !m_pTestEnvironment->IsValid() )
						m_pCubeRenderer->SetScale( m_pCubeRenderer->GetScale() - 0.5f );
					break;
					*/
				case 0x52: // 'R'
					TestDeviceLost();
					break;

				case 0x53:	// 'S'
					m_pUserInterface->ToggleStatistics();
					break;
			}
		}
        break;

		case WM_INPUT:
		{
			m_pInput->OnRawInput( (HRAWINPUT)lParam );
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		break;
		
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

void AnimaApplication::OnDeviceLost()
{
//	m_pCubeRenderer->ReleaseResources( m_pRenderContext ); 
	m_pUserInterface->ReleaseResources( m_pRenderContext );
}

void AnimaApplication::OnDeviceReset()
{
//	m_pCubeRenderer->AcquireResources( m_pRenderContext ); 
	m_pUserInterface->AcquireResources( m_pRenderContext );
}

void AnimaApplication::TestDeviceLost()
{
#ifndef OPENGL
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
					
	D3DDISPLAYMODE d3ddm;

	m_pRenderContext->Device()->GetDisplayMode( 0, &d3ddm );

	d3dpp.Windowed               = TRUE;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat       = d3ddm.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_pRenderContext->Device()->Reset( &d3dpp );
#endif
}