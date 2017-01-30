#include "stdafx.h"

#include "InputSimulator.h"



Multikeys::InputSimulator::InputSimulator()
{
	// Setup simulated keyboard
	for (int i = 0; i < 512; i++)
	{
		simulatedKeyboards[i].type = INPUT_KEYBOARD;
		simulatedKeyboards[i].ki.dwExtraInfo = 0;
		simulatedKeyboards[i].ki.time = 0;
	}

	// Remarks: wVk holds a virtual-key code in the range of 1 to 254, but must be 0 if
	//		the unicode flag is set. wScan is the scancode, but holds the unicode code point
	//		instead if the unicode flag is set.

}



UINT Multikeys::InputSimulator::SendVirtualKey(BYTE modifiers, USHORT vKey, BOOL keyup)
{
	// Modifiers, how? If?
	if (DEBUG)
	{
		WCHAR * text = new WCHAR[128];
		swprintf_s(text, 128, L"New virtual key is %hx", vKey);		// hx is unsigned short hex
		OutputDebugString(text);
		delete[] text;
	}

	// For virtual key codes, we have them send a strange scancode that the hook can detect and filter out (e0 00)
	simulatedKeyboards[0].ki.wScan = 0;
	simulatedKeyboards[0].ki.dwFlags = (keyup ? KEYEVENTF_KEYUP : 0) | KEYEVENTF_EXTENDEDKEY;
	simulatedKeyboards[0].ki.wVk = vKey;		// <- virtual key
	return SendInput(1, simulatedKeyboards, sizeof(INPUT));		// One key

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

	// Setup for sending unicode
	simulatedKeyboards[0].ki.dwFlags = KEYEVENTF_UNICODE;
	simulatedKeyboards[1].ki.dwFlags = KEYEVENTF_UNICODE;
	simulatedKeyboards[0].ki.wVk = 0;
	simulatedKeyboards[1].ki.wVk = 0;

	if (codepoint > 0xffff)		// means it's a surrogate pair in UTF-16
	{							// (semi-relevant note: 0xffff itself never corresponds to anything)
		// 1. subtract 0x010000 from the codepoint
		// 2. high surrogate = most significant 10 bits from codepoint, added to 0xd800
		// 3. low surrogate = least significant 10 bits from codepoint, added to 0xdc00
		// 4. send the high surrogate first, and the low surrogate next, in succession
		codepoint -= 0x010000;
		simulatedKeyboards[0].ki.wScan = 0xd800 + (codepoint >> 10);		// first message, high surrogate
		simulatedKeyboards[1].ki.wScan = 0xdc00 + (codepoint & 0x3ff);		// second message, low surrogate

		if (!keyup)
			return SendInput(2, simulatedKeyboards, sizeof(INPUT));
		else
		{
			simulatedKeyboards[0].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
			simulatedKeyboards[1].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
			UINT returnValue = SendInput(2, simulatedKeyboards, sizeof(INPUT));	// remember
			simulatedKeyboards[0].ki.dwFlags = KEYEVENTF_UNICODE;					// undo flag
			simulatedKeyboards[1].ki.dwFlags = KEYEVENTF_UNICODE;					// return
			return returnValue;
		}
	}

	// reaching this point means it's not a surrogate pair
	simulatedKeyboards[0].ki.wScan = codepoint;		// This is the unicode character

	if (!keyup)		// 1. No. of inputs; 2. array of inputs; 3. size of one structure
		return SendInput(1, simulatedKeyboards, sizeof(INPUT));
	else
	{
		simulatedKeyboards[0].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
		UINT returnValue = SendInput(1, simulatedKeyboards, sizeof(INPUT));
		simulatedKeyboards[0].ki.dwFlags = KEYEVENTF_UNICODE;
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