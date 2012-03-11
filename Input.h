#pragma once

#include "stdafx.h"

class Input
{
public:
	struct KeyState
	{
		USHORT PreviousState;
		USHORT CurrentState;

		KeyState() : PreviousState(RI_KEY_BREAK), CurrentState(RI_KEY_BREAK){}
	};

	typedef std::map<USHORT, KeyState> KeyMap;

private:
	static const UINT MAX_KEYBOARD_EVENTS_PER_FRAME = 16;

	POINT mMouseDelta;
	KeyMap mKeys;
	short mDeltaWheel;

	struct
	{
		POINT Mouse;
		short mDeltaWheel;

		UINT KeyboardEventCount;
		struct 
		{
			USHORT Key;
			USHORT Flags;
		}KeyboardEvents[MAX_KEYBOARD_EVENTS_PER_FRAME];

	} mBufferedInput;

	std::vector<BYTE> mInputBuffer;
	bool mBlocked;

public:
	Input();
	~Input();

	void OnRawInput( HRAWINPUT hRawInput );
	void Update( float dt );
	void Unblock();

	POINT GetMouse() const { return mMouseDelta; }
	bool IsKeyPressed( USHORT VKey ) const;
	const KeyMap& GetKeys() const { return mKeys; }
	short GetWheelDelta(void) const;
};

