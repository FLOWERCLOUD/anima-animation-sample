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

AnimaApplication* AnimaApplication::mInstance = 0;

AnimaApplication::~AnimaApplication()
{
	if( mInstance )
	{
		UnregisterClass( "MY_WINDOWS_CLASS", mWindowClass.hInstance );

		mUserInterface->ReleaseResources( mRenderContext );
		delete mUserInterface;
		
		mModel->ReleaseResources( mRenderContext );
		delete mModel;

		delete mFramerateCounter;
		delete mRenderContext;
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
	assert( AnimaApplication::mInstance == 0 );

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
                             "DX9 Skinning Sample",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, NULL, NULL, hInstance, NULL );

	if( hWnd == NULL )
		return E_FAIL;

	// register raw input device
	RAWINPUTDEVICE Rid[2];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    Rid[0].dwFlags = RIDEV_INPUTSINK;   
    Rid[0].hwndTarget = hWnd;
 
	// Keyboard
	Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[1].usUsage = 6;
	Rid[1].dwFlags = 0;
	Rid[1].hwndTarget=hWnd;

	RegisterRawInputDevices( Rid, 2, sizeof(Rid[0]) );


	ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

	LPWSTR *szArglist;
	int nArgs;
	
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	TestEnvironment* testEnvironment = new TestEnvironment( nArgs, szArglist );
	Math::Random::Init( testEnvironment->GetRandomSeed() );

	AnimaApplication::mInstance = new AnimaApplication( winClass, hWnd );

	Instance()->mTestEnvironment = testEnvironment;
	Instance()->mRenderContext = new RenderContext( hWnd, DISPLAY_WIDTH, DISPLAY_HEIGHT );
	Instance()->mFramerateCounter = new FramerateCounter;

	Instance()->mUserInterface = new UserInterface( mInstance->mRenderContext, mInstance->mFramerateCounter );
	Instance()->mUserInterface->AcquireResources( Instance()->mRenderContext );

	Instance()->mInput = new Input();
	Instance()->mCamera = new Camera( *Instance()->mInput, *Instance()->mRenderContext );
	// set up renderer 

	Instance()->mModel = new SkeletalModel( "..\\Models\\frank.dae" );
	Instance()->mModel->Load( Instance()->mRenderContext );
	Instance()->mModel->SetNext( Instance()->mUserInterface );
	Instance()->mModel->PlayAnimation( 0, 0.25f );

	Instance()->mModelRotation = 0.f;
	Instance()->mRotateModel = true;

	return S_OK;
}

void AnimaApplication::DestroyInstance()
{
	assert( AnimaApplication::mInstance );
	delete AnimaApplication::mInstance;
	AnimaApplication::mInstance = 0;
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
	mDeltaTime.Update();

	mInput->Update( mDeltaTime.Elapsed() );

	mFramerateCounter->FrameStart();

	mCamera->update( mDeltaTime.Elapsed() );
	mModel->Update( mDeltaTime.Elapsed() );

	mRenderContext->SetViewMatrix( mCamera->ViewMatrix() );
	mRenderContext->RenderFrame( mModel );

	mFramerateCounter->FrameEnd();
	
	if( mRotateModel )
	{
		mModelRotation += 0.0001f; 
	}

	aiQuaternion rotateUpright( 0, 0,  Math::Pi / 2.f );
	aiQuaternion rotateY( 0, mModelRotation, 0 );

	mModel->SetRoot( aiVector3D(0.f, 0.f, 0.f ), rotateUpright*rotateY );
}

LRESULT AnimaApplication::OnMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{	

        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_SPACE:
					mRotateModel = mModel->ToggleAnimationPlayback();
					break;
				case VK_ESCAPE:
				case 0x51: // 'Q'
					PostQuitMessage(0);
					break;

				case 0x52: // 'R'
					TestDeviceLost();
					break;

				case 0x53:	// 'S'
					mUserInterface->ToggleStatistics();
					break;
				case 0x54: // 'T'
					mModel->ToggleShaderTest();
					break;

				case 0x4D: // 'M'
					int animationMethod = mModel->ToggleAnimationMethod();
					mUserInterface->SetSkeletalAnimationMethod( animationMethod );
					break;
			}
		}
        break;

		case WM_INPUT:
		{
			mInput->OnRawInput( (HRAWINPUT)lParam );
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
	mModel->ReleaseResources( mRenderContext ); 
	mUserInterface->ReleaseResources( mRenderContext );
}

void AnimaApplication::OnDeviceReset()
{
	mModel->AcquireResources( mRenderContext ); 
	mUserInterface->AcquireResources( mRenderContext );
}

void AnimaApplication::TestDeviceLost()
{
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
					
	D3DDISPLAYMODE d3ddm;

	mRenderContext->Device()->GetDisplayMode( 0, &d3ddm );

	d3dpp.Windowed               = TRUE;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat       = d3ddm.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	mRenderContext->Device()->Reset( &d3dpp );
}