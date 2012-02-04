

#include "stdafx.h"
#include "Debug.h"

#include <stdio.h> 
#include <stdarg.h>


void DebugPrint( const TCHAR* szFormat, ...)
{
	TCHAR szBuff[8192];
    va_list arg;
    va_start(arg, szFormat);
    vsnprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
    va_end(arg);

    OutputDebugString(szBuff);
}

void __DxCheckAndReportError( HRESULT hr, const char* file, int line )
{
	if( hr != S_OK )
	{
		const char* errorString = DXGetErrorString( hr );
		const char* errorDescription = DXGetErrorDescription( hr );

		char buffer[4096];
		sprintf_s( buffer, "Error '%s': %s occured in %s, line %d", errorString, errorDescription, file, line );
		MessageBox( NULL, "DirectX Error", buffer, MB_OK );

		DebugPrint( "%s\n", buffer );
	}
}