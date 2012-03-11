#include "Input.h"
#include "Debug.h"



Input::Input()
{
	mBlocked = true;
	memset( &mBufferedInput, 0x0, sizeof(mBufferedInput) ); 
}


Input::~Input(void)
{
}

void Input::Unblock()
{
	mBlocked = false;
}

void Input::Update( float dt )
{	
	// update old keyboard state
	for( KeyMap::iterator it = mKeys.begin(); it != mKeys.end(); ++it )
		it->second.PreviousState =it->second.CurrentState;

	// update new mouse and keyboard state
	mMouseDelta = mBufferedInput.Mouse;
	for( unsigned int i=0; i<mBufferedInput.KeyboardEventCount; ++i )
		mKeys[mBufferedInput.KeyboardEvents[i].Key].CurrentState = mBufferedInput.KeyboardEvents[i].Flags;

	mDeltaWheel = mBufferedInput.mDeltaWheel;

	// reset buffered input
	memset( &mBufferedInput, 0x0, sizeof(mBufferedInput) ); 

}

void Input::OnRawInput( HRAWINPUT hRawInput )
{
	if( !mBlocked )
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

			// get the delta value of the mouse wheel
			mBufferedInput.mDeltaWheel = rawInput->data.mouse.usButtonData;
		} 
	}
}

bool Input::IsKeyPressed( USHORT VKey ) const
{
	KeyMap::const_iterator it = mKeys.find( VKey );
	return it != mKeys.end() && !(it->second.CurrentState & RI_KEY_BREAK);
}

short Input::GetWheelDelta(void) const
{
	return mDeltaWheel;
}