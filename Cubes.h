#pragma once

#include "stdafx.h"
#include "resource.h"
#include "TestEnvironment.h"

class RenderContext;
class FramerateCounter;
class CubeRendererInterface;
class UserInterface;
class Camera;
class Input;
class Model;

/*	The main application class (Singleton)
		Owns instances of all other classes, creates the application window and handles keyboad input				*/
class AnimaApplication
{
	static const int DISPLAY_WIDTH = 1024;
	static const int DISPLAY_HEIGHT = 768;

	RenderContext*  m_pRenderContext;
	FramerateCounter* m_pFramerateCounter;
	UserInterface* m_pUserInterface;
	TestEnvironment* m_pTestEnvironment;
	Camera* m_pCamera;
	Input* m_pInput;
	Model* m_Model;

	WNDCLASSEX m_WindowClass;
	HWND m_WindowHandle;

	AnimaApplication(){}
	AnimaApplication( WNDCLASSEX windowClass, HWND windowHandle ) : m_WindowClass( windowClass ), m_WindowHandle( windowHandle ){}
	static AnimaApplication* m_pInstance;

	CubeRendererInterface* m_pCubeRenderer;

	class DeltaTime 
	{
		LARGE_INTEGER TicksPerSecond;
		LARGE_INTEGER LastTick;

		float Delta;
	public:
		DeltaTime()
		{
			QueryPerformanceFrequency( &TicksPerSecond );
			QueryPerformanceCounter( &LastTick );
			Delta = 0.f;
		}

		void Update()
		{
			LARGE_INTEGER curTick;
			QueryPerformanceCounter( &curTick );

			Delta = float(double(curTick.QuadPart - LastTick.QuadPart)/TicksPerSecond.QuadPart);
			LastTick = curTick;
		}

		float Elapsed()
		{
			return Delta;
		}
	} m_DeltaTime;

	

public: 
	~AnimaApplication();
	static HRESULT CreateInstance( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	static void DestroyInstance();

	static AnimaApplication* Instance() { return m_pInstance; }
	void Run();

	void NextFrame();	// update and render the next frame

	void SetTestEnvironment( TestEnvironment* testEnvironment ) { m_pTestEnvironment = testEnvironment; }
	const TestEnvironment* GetTestEnvironment() { return m_pTestEnvironment; }

	LRESULT OnMessage(HWND   hWnd, UINT   msg, WPARAM wParam,  LPARAM lParam );
	void OnDeviceLost();
	void OnDeviceReset();

	void TestDeviceLost();
};