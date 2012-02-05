#include "Input.h"




Input::Input()
{
	memset( &mBufferedInput, 0x0, sizeof(mBufferedInput) ); 
}


Input::~Input(void)
{
}

void Input::Update( float dt )
{
	mMouseDelta = mBufferedInput.Mouse;
	
	mKeys.clear();
	for( unsigned int i=0; i<mBufferedInput.KeyboardEventCount; ++i )
	{
		mKeys[mBufferedInput.KeyboardEvents[i].Key] = mBufferedInput.KeyboardEvents[i].Flags;
	}

	// reset buffered input
	memset( &mBufferedInput, 0x0, sizeof(mBufferedInput) ); 

}

void Input::OnRawInput( HRAWINPUT hRawInput )
{
	UINT requiredSize;
	GetRawInputData( hRawInput, RID_INPUT, NULL, &requiredSize, sizeof(RAWINPUTHEADER) );
	if( mInputBuffer.size() < requiredSize ) mInputBuffer.resize( requiredSize );

	GetRawInputData( hRawInput, RID_INPUT, &mInputBuffer[0], &requiredSize, sizeof(RAWINPUTHEADER) ) ;
	RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>( &mInputBuffer[0] );	

	if (rawInput->header.dwType == RIM_TYPEKEYBOARD && mBufferedInput.KeyboardEventCount < MAX_KEYBOARD_EVENTS_PER_FRAME-1)
	{
		mBufferedInput.KeyboardEvents[mBufferedInput.KeyboardEventCount].Key = rawInput->data.keyboard.VKey;
		mBufferedInput.KeyboardEvents[mBufferedInput.KeyboardEventCount].Flags = rawInput->data.keyboard.Flags;
		mBufferedInput.KeyboardEventCount++;
	}
	else if (rawInput->header.dwType == RIM_TYPEMOUSE)
	{
		mBufferedInput.Mouse.x += rawInput->data.mouse.lLastX ;
		mBufferedInput.Mouse.y += rawInput->data.mouse.lLastY;
	} 
}

bool Input::GetKey( USHORT VKey ) const
{
	return mKeys.find( VKey ) != mKeys.end();
	
}
