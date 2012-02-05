#include "StdAfx.h"
#include "TestEnvironment.h"
#include <stdio.h>

TestEnvironment::TestEnvironment( int argc, const LPWSTR* argv )
	: mNumCubes( 1000 )
	, mCubeRotation( 0.7f )
	, mCubeScale( 1.0f )
	, mIsValid( false )
	, mRendererVersion( 0 )
	, mRandomSeed( 27520 )
{
	bool hasNumCubes = false;
	bool hasCubeRotation = false;
	bool hasCubeScale = false;

	for( int i=1; i<argc; ++i )
	{
		if( wcscmp( argv[i], L"--NumCubes" ) == 0 )
		{
			++i;
			mNumCubes = _wtoi( argv[i] );
			hasNumCubes = true;
		}

		else if(  wcscmp( argv[i], L"--CubeRotation" ) == 0 )
		{
			++i;
			mCubeRotation = (float) _wtof( argv[i] );
			hasCubeRotation = true;
		}

		else if(  wcscmp( argv[i], L"--CubeScale" ) == 0 )
		{
			++i;
			mCubeScale = (float) _wtof( argv[i] );
			hasCubeScale = true;
		}
		else if(  wcscmp( argv[i], L"--RendererVersion" ) == 0 )
		{
			++i;
			mRendererVersion =  _wtoi( argv[i] );
		}
		else if(  wcscmp( argv[i], L"--RandomSeed" ) == 0 )
		{
			++i;
			mRandomSeed =  _wtoi( argv[i] );
		}

	}

	mIsValid = hasNumCubes && hasCubeRotation && hasCubeScale;
}


TestEnvironment::~TestEnvironment(void)
{
}
