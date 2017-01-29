#include "stdafx.h"

#include "InputSimulator.h"



Multikeys::InputSimulator::InputSimulator()
{
	// Setup simulated keyboard
	simulatedKeyboardUnicode.type = INPUT_KEYBOARD;
	simulatedKeyboardUnicode.ki.dwExtraInfo = 0x21;		// doesn't matter
	simulatedKeyboardUnicode.ki.wVk = 0;
	simulatedKeyboardUnicode.ki.time = 0;
	simulatedKeyboardUnicode.ki.dwFlags = KEYEVENTF_UNICODE;

	// and the double simulated keyboard (for surrogate pairs)
	simulatedDoubleKeyboard = new INPUT[2];
	for (int i = 0; i < 2; i++)
	{
		simulatedDoubleKeyboard[i].type = INPUT_KEYBOARD;
		simulatedDoubleKeyboard[i].ki.dwExtraInfo = 0x42;		// doesn't matter
		simulatedDoubleKeyboard[i].ki.time = 0;
		simulatedDoubleKeyboard[i].ki.wVk = 0;			// causes it to send a message with vk=e7
		simulatedDoubleKeyboard[i].ki.dwFlags = KEYEVENTF_UNICODE;
	}

	// For the virtual key codes, we must have them send a strange scancode that the hook can detect and filter out

	// Simulated keyboard for sending virtual-key codes
	simulatedKeyboardVirtualKey.type = INPUT_KEYBOARD;
	simulatedKeyboardVirtualKey.ki.dwExtraInfo = NULL;
	simulatedKeyboardVirtualKey.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	simulatedKeyboardVirtualKey.ki.wScan = 0;			// hope e0 00 doesn't correspond to anything normally
	simulatedKeyboardVirtualKey.ki.time = 0;

	// Simulated keyboard for sending modifiers
	simulatedKeyboardModifiers.type = INPUT_KEYBOARD;
	simulatedKeyboardModifiers.ki.dwExtraInfo = NULL;
	simulatedKeyboardModifiers.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	simulatedKeyboardVirtualKey.ki.wScan = 0;			// same thing
	simulatedKeyboardModifiers.ki.time = 0;

	// Remarks: wVk holds a virtual-key code in the range of 1 to 254, but must be 0 if
	//		the unicode flag is set. wScan is the scancode, but holds the unicode code point
	//		instead if the unicode flag is set.

}



UINT Multikeys::InputSimulator::SendVirtualKey(BYTE modifiers, USHORT vKey, BOOL keyup)
{
	if (DEBUG)
	{
		WCHAR * text = new WCHAR[128];
		swprintf_s(text, 128, L"New virtual key is %hx", vKey);		// hx is unsigned short hex
		OutputDebugString(text);
		delete[] text;
	}

	UINT result = 0;

	// not too proud of this:
	if (!keyup)
	{
		simulatedKeyboardModifiers.ki.dwFlags = 0;
		// Handle modifiers
		if (modifiers & MODIFIER_LSHIFT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LSHIFT;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RSHIFT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RSHIFT;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LCTRL)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LCONTROL;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RCTRL)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RCONTROL;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LALT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LMENU;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RALT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RMENU;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LWIN)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LWIN;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RWIN)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RWIN;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
	}

	simulatedKeyboardVirtualKey.ki.wVk = vKey;
	simulatedKeyboardVirtualKey.ki.dwFlags = (keyup ? KEYEVENTF_KEYUP : 0) | KEYEVENTF_EXTENDEDKEY;
	result += SendInput(1, &simulatedKeyboardVirtualKey, sizeof(INPUT));

	if (keyup)
	{
		simulatedKeyboardModifiers.ki.dwFlags = KEYEVENTF_KEYUP;
		// Handle modifiers
		if (modifiers & MODIFIER_LSHIFT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LSHIFT;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RSHIFT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RSHIFT;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LCTRL)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LCONTROL;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RCTRL)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RCONTROL;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LALT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LMENU;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RALT)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RMENU;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_LWIN)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_LWIN;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
		if (modifiers & MODIFIER_RWIN)
		{
			simulatedKeyboardModifiers.ki.wVk = VK_RWIN;
			result += SendInput(1, &simulatedKeyboardModifiers, sizeof(INPUT));
		}
	}
	
	return result;
}



UINT Multikeys::InputSimulator::SendUnicodeCharacter(UINT32 codepoint, BOOL keyup)
{
	if (DEBUG)
	{
		WCHAR * text = new WCHAR[128];
		if (codepoint <= 0xffff)
		{
			swprintf_s(text, 128, L"New character has code point %x\n", codepoint);
		}
		else
		{
			swprintf_s(text, 128, L"New character has code point %x (%x %x)\n",
				codepoint, 0xd800 + (codepoint >> 10),
				0xdc00 + codepoint & 0x3ff);		// don't mind this
		}

		OutputDebugString(text);
		delete[] text;
	}


	if (codepoint > 0xffff)		// means it's a surrogate pair in UTF-16
	{							// (semi-relevant note: 0xffff itself never corresponds to anything)
		// 1. subtract 0x010000 from the codepoint
		// 2. high surrogate = most significant 10 bits from codepoint, added to 0xd800
		// 3. low surrogate = least significant 10 bits from codepoint, added to 0xdc00
		// 4. send the high surrogate first, and the low surrogate next, in succession
		codepoint -= 0x010000;
		simulatedDoubleKeyboard[0].ki.wScan = 0xd800 + (codepoint >> 10);		// first message, high surrogate
		simulatedDoubleKeyboard[1].ki.wScan = 0xdc00 + (codepoint & 0x3ff);		// second message, low surrogate

		if (!keyup)
			return SendInput(2, simulatedDoubleKeyboard, sizeof(INPUT));
		else
		{
			simulatedDoubleKeyboard[0].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
			simulatedDoubleKeyboard[1].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
			UINT returnValue = SendInput(2, simulatedDoubleKeyboard, sizeof(INPUT));	// remember
			simulatedDoubleKeyboard[0].ki.dwFlags = KEYEVENTF_UNICODE;					// undo flag
			simulatedDoubleKeyboard[1].ki.dwFlags = KEYEVENTF_UNICODE;					// return
			return returnValue;
		}
	}

	// reaching this point means it's not a surrogate pair
	simulatedKeyboardUnicode.ki.wScan = codepoint;		// This is the unicode character

	if (!keyup)		// 1. No. of inputs; 2. array of inputs; 3. size of one structure
		return SendInput(1, &simulatedKeyboardUnicode, sizeof(INPUT));
	else
	{
		simulatedKeyboardUnicode.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
		UINT returnValue = SendInput(1, &simulatedKeyboardUnicode, sizeof(INPUT));
		simulatedKeyboardUnicode.ki.dwFlags = KEYEVENTF_UNICODE;
		return returnValue;
	}
}



UINT Multikeys::InputSimulator::SendKeyboardInput(KEYSTROKE_OUTPUT key)
{
	if (key.flags & KEYEVENTF_UNICODE)
	{
		// We must send a unicode instead
		return SendUnicodeCharacter(key.codepoint, key.flags & KEYEVENTF_KEYUP);
	}

	// Ok, normal virtual key
	return SendVirtualKey(key.modifiers, key.vKey, key.flags & KEYEVENTF_KEYUP);
}