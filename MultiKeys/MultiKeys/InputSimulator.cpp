#include "stdafx.h"

#include "InputSimulator.h"



Multikeys::InputSimulator::InputSimulator()
{
	// Setup simulated keyboard
	simulatedKeyboardUnicode.type = INPUT_KEYBOARD;
	simulatedKeyboardUnicode.ki.dwExtraInfo = NULL;
	simulatedKeyboardUnicode.ki.wVk = 0;
	simulatedKeyboardUnicode.ki.time = 0;
	simulatedKeyboardUnicode.ki.dwFlags = KEYEVENTF_UNICODE;

	// and the double simulated keyboard (for surrogate pairs)
	simulatedDoubleKeyboard = new INPUT[2];
	for (int i = 0; i < 2; i++)
	{
		simulatedDoubleKeyboard[i].type = INPUT_KEYBOARD;
		simulatedDoubleKeyboard[i].ki.dwExtraInfo = NULL;
		simulatedDoubleKeyboard[i].ki.time = 0;
		simulatedDoubleKeyboard[i].ki.wVk = 0;			// causes it to send a message with vk=e7
		simulatedDoubleKeyboard[i].ki.dwFlags = KEYEVENTF_UNICODE;
	}

	// Simulated keyboard for sending virtual-key codes
	simulatedKeyboardVirtualKey.type = INPUT_KEYBOARD;
	simulatedKeyboardVirtualKey.ki.dwExtraInfo = NULL;
	simulatedKeyboardVirtualKey.ki.time = 0;

	// Remarks: wVk holds a virtual-key code in the range of 1 to 254, but must be 0 if
	//		the unicode flag is set. wScan is the scancode, but hold the unicode code point
	//		instead if the unicode flag is set.

}



UINT Multikeys::InputSimulator::SendVirtualKey(BYTE modifiers, USHORT vKey)
{
	// TODO: implement modifiers

	// We're sending a signal without scancode. This may go very wrong.
	if (DEBUG)
	{
		WCHAR * text = new WCHAR[128];
		swprintf_s(text, 128, L"New virtual key is %hx", vKey);		// hx is unsigned short hex
		OutputDebugString(text);
		delete[] text;
	}

	simulatedKeyboardVirtualKey.ki.wVk = vKey;
	return SendInput(1, &simulatedKeyboardVirtualKey, sizeof(INPUT));
}



UINT Multikeys::InputSimulator::SendUnicodeCharacter(UINT32 codepoint)
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

		return SendInput(2, simulatedDoubleKeyboard, sizeof(INPUT));
	}

	simulatedKeyboardUnicode.ki.wScan = codepoint;		// This is the unicode character

	return SendInput(1, &simulatedKeyboardUnicode, sizeof(INPUT));		// 1. No. of inputs; 2. array of inputs; 3. size of one structure
}



UINT Multikeys::InputSimulator::SendKeyboardInput(Keystroke key)
{
	if (key.vKey == 0)
	{
		// We must send a unicode instead
		return SendUnicodeCharacter(key.codepoint);
	}

	// Ok, normal virtual key
	return SendVirtualKey(key.modifiers, key.vKey);
}