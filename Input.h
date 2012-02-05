#pragma once

#include "stdafx.h"

class Input
{
	static const UINT MAX_KEYBOARD_EVENTS_PER_FRAME = 16;

	typedef std::map<USHORT, USHORT> KeyInputType;

	POINT mMouseDelta;
	KeyInputType mKeys;

	struct
	{
		POINT Mouse;

		UINT KeyboardEventCount;
		struct 
		{
			USHORT Key;
			USHORT Flags;
		}KeyboardEvents[MAX_KEYBOARD_EVENTS_PER_FRAME];

	} mBufferedInput;

	std::vector<BYTE> mInputBuffer;
public:
	Input();
	~Input();

	void OnRawInput( HRAWINPUT hRawInput );
	void Update( float dt );

	POINT GetMouse() const { return mMouseDelta; }
	bool GetKey( USHORT VKey ) const;

};

