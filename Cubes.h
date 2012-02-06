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
	static const int DISPLAY_WIDTH = 1280;
	static const int DISPLAY_HEIGHT = 720; 

	RenderContext*  mRenderContext;
	FramerateCounter* mFramerateCounter;
	UserInterface* mUserInterface;
	TestEnvironment* mTestEnvironment;
	Camera* mCamera;
	Input* mInput;
	Model* mModel;

	WNDCLASSEX mWindowClass;
	HWND mWindowHandle;

	AnimaApplication(){}
	AnimaApplication( WNDCLASSEX windowClass, HWND windowHandle ) : mWindowClass( windowClass ), mWindowHandle( windowHandle ){}
	static AnimaApplication* mInstance;

	float mModelRotation;

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
	} mDeltaTime;

	

public: 
	~AnimaApplication();
	static HRESULT CreateInstance( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	static void DestroyInstance();

	static AnimaApplication* Instance() { return mInstance; }
	void Run();

	void NextFrame();	// update and render the next frame

	void SetTestEnvironment( TestEnvironment* testEnvironment ) { mTestEnvironment = testEnvironment; }
	const TestEnvironment* GetTestEnvironment() { return mTestEnvironment; }

	LRESULT OnMessage(HWND   hWnd, UINT   msg, WPARAM wParam,  LPARAM lParam );
	void OnDeviceLost();
	void OnDeviceReset();

	void TestDeviceLost();
};