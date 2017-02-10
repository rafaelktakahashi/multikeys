#pragma once


#include "stdafx.h"
#include "Scancodes.h"


#define VIRTUAL_MODIFIER_LCTRL		0x80
#define VIRTUAL_MODIFIER_RCTRL		0x40
#define VIRTUAL_MODIFIER_LALT		0x20
#define VIRTUAL_MODIFIER_RALT		0x10
#define VIRTUAL_MODIFIER_LWIN		0x08
#define VIRTUAL_MODIFIER_RWIN		0x04
#define VIRTUAL_MODIFIER_LSHIFT		0x02
#define VIRTUAL_MODIFIER_RSHIFT		0x01

// enum classes are strongly typed
enum class KeystrokeOutputType
{
	UnicodeOutput,
	VirtualOutput,
	MacroOutput,
	StringOutput,
	ScriptOutput,
	DeadKeyOutput,
	NoOutput
};

struct IKeystrokeOutput		/*Interface*/
{
protected:
	INPUT unicodePrototype;
	INPUT VirtualKeyPrototypeDown;
	INPUT VirtualKeyPrototypeUp;

	IKeystrokeOutput()
	{
		// Initialize prototypes

		// When keyeventf_unicode is set, virtual key must be 0,
		// and the UTF-16 code value is put into wScan
		// Surrogate pairs require two consecutive inputs
		unicodePrototype.type = INPUT_KEYBOARD;
		unicodePrototype.ki.dwExtraInfo = 0;
		unicodePrototype.ki.dwFlags = KEYEVENTF_UNICODE;
		unicodePrototype.ki.time = 0;
		unicodePrototype.ki.wVk = 0;


		// Virtual keys are sent with the scancode e0 00 because the hook
		// will filter these out (to avoid responding to injected keys)
		VirtualKeyPrototypeDown.type = INPUT_KEYBOARD;
		VirtualKeyPrototypeDown.ki.dwExtraInfo = 0;
		VirtualKeyPrototypeDown.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
		VirtualKeyPrototypeDown.ki.time = 0;
		VirtualKeyPrototypeDown.ki.wScan = 0;

		VirtualKeyPrototypeUp = VirtualKeyPrototypeDown;
		VirtualKeyPrototypeUp.ki.dwFlags |= KEYEVENTF_KEYUP;
	}

public:

	virtual KeystrokeOutputType getType() = 0;

	virtual BOOL simulate(BOOL keyup, BOOL repeated = FALSE) = 0;
};

struct UnicodeOutput : IKeystrokeOutput
{

protected:

	INPUT * keystrokes;
	USHORT inputCount;
	UINT codepoint;			// Non-surrogate codepoint is for checking dead keys

public:

	// UINT _codepoint - A single Unicode code point identifying the character that this
	//		command can send
	// DEPRECATED; remove class in a future version
	UnicodeOutput(UINT _codepoint) : IKeystrokeOutput(), codepoint(_codepoint)
	{
		if (_codepoint <= 0xffff)
		{
			// One UTF-16 code unit
			inputCount = 1;
			keystrokes = new INPUT(unicodePrototype);
			keystrokes->ki.wScan = _codepoint;
		}
		else
		{
			// UTF-16 surrogate pair
			inputCount = 2;
			keystrokes = new INPUT[2];
			for (int i = 0; i < 2; i++)
			{ keystrokes[i] = INPUT(unicodePrototype); }

			_codepoint -= 0x10000;
			keystrokes[0].ki.wScan =
				0xd800 + (_codepoint >> 10);		// High surrogate
			keystrokes[1].ki.wScan =
				0xdc00 + (_codepoint & 0x3ff);		// Low surrogate
		}
	}	// end of constructor

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::UnicodeOutput;
	}

	// It does not seem to be necessary to send keyups when sending unicode characters
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (!keyup)
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

};

// The difference between this and MacroOutput is that this one will keep being sent while user
// holds down the mapped key. Macros only activate once.
struct VirtualKeyOutput : IKeystrokeOutput
{

protected:

	INPUT * keystrokesDown;
	INPUT * keystrokesUp;
	USHORT inputCount;

public:

	// USHORT _virtualKeyCode - the virtual key code to be sent
	// BYTE _modifiers - flags with modifiers to be sent
	// DEPRECATED; remove class in a future version
	VirtualKeyOutput(USHORT _virtualKeyCode, BYTE _modifiers) : IKeystrokeOutput()
	{
		// Count number of set bits
		// Doesn't need to be superfast
		USHORT modifierCount = 0;
		for (unsigned int i = 0; i < 8; i++)
			if ((_modifiers >> i) & i)
				modifierCount++;

		// We need one INPUT for each modifier, plus one for the key itself
		USHORT currentIndex = 0;
		keystrokesDown = new INPUT[modifierCount + 1];
		keystrokesUp = new INPUT[modifierCount + 1];
		// That's also the input count:
		inputCount = modifierCount + 1;
		for (int i = 0; i < inputCount; i++)
		{
			keystrokesDown[i] = INPUT(VirtualKeyPrototypeDown);
			keystrokesUp[i] = INPUT(VirtualKeyPrototypeUp);
		}

		// Careful - keystrokes up are in inverse order, with index 0 corresponding to the key itself
		{
			if ((_modifiers & VIRTUAL_MODIFIER_LCTRL) == VIRTUAL_MODIFIER_LCTRL)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_LCONTROL;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LCONTROL;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_RCTRL) == VIRTUAL_MODIFIER_RCTRL)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_RCONTROL;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RCONTROL;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_LALT) == VIRTUAL_MODIFIER_LALT)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_LMENU;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LMENU;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_RALT) == VIRTUAL_MODIFIER_RALT)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_RMENU;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RMENU;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_LWIN) == VIRTUAL_MODIFIER_LWIN)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_LWIN;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LWIN;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_RWIN) == VIRTUAL_MODIFIER_RWIN)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_RWIN;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RWIN;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_LSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_LSHIFT;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LSHIFT;
				currentIndex++;
			}
			if ((_modifiers & VIRTUAL_MODIFIER_RSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
			{
				keystrokesDown[currentIndex].ki.wVk = VK_RSHIFT;
				keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RSHIFT;
				currentIndex++;
			}
		}
		keystrokesDown[currentIndex].ki.wVk = _virtualKeyCode;
		keystrokesUp[currentIndex - currentIndex].ki.wVk = _virtualKeyCode;

	}	// end of constructor

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::VirtualOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (keyup)
			return (SendInput(inputCount, keystrokesUp, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else
			return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}
};

struct MacroOutput : IKeystrokeOutput
{

protected:

	INPUT * keystrokes;
	USHORT inputCount;

public:

	// DWORD * keypressSequence - array of DWORDs, each containing the virtual key code to be
	//		sent, and also the high bit (the 31st, from 0) set in case of a keyup. Every keypress
	//		in this array will be sent in order on execution.
	// USHORT _inpoutCount - number of elements in keypressSequence
	MacroOutput(const DWORD * keypressSequence, const USHORT _inputCount) : IKeystrokeOutput()
																		, inputCount(_inputCount)
	{
		if (keypressSequence == nullptr)
			return;

		keystrokes = new INPUT[_inputCount];

		BOOL keyup = 0;
		USHORT virtualKeyCode = 0;
		for (unsigned int i = 0; i < _inputCount; i++)
		{
			keyup = (keypressSequence[i] >> 31) & 1;
			virtualKeyCode = keypressSequence[i] & 0xff;
			keystrokes[i] = INPUT(VirtualKeyPrototypeDown);
			keystrokes[i].ki.wVk = virtualKeyCode;
			if (keyup) keystrokes[i].ki.dwFlags |= KEYEVENTF_KEYUP;
		}
		
	}	// end of constructor

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::MacroOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (keyup)
			return TRUE;
		else if (!repeated)
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

};

struct StringOutput : IKeystrokeOutput
{
	
protected:

	INPUT * keystrokes;
	USHORT inputCount;

public:

	// UINT codepoints - array of UINTs, each containing a single Unicode code point
	//		identifying the character to be sent. All characters in this array will
	//		be sent in order on execution.
	// UINT _inputCount - number of elements in codepoints
	StringOutput(const UINT * codepoints, const UINT _inputCount) : IKeystrokeOutput()
																, inputCount(_inputCount)
	{
		if (codepoints == nullptr) return;

		for (unsigned int i = 0; i < _inputCount; i++)
		{
			if (codepoints[i] > 0xffff)
				inputCount++;			// <- real amount of inputs
		}
		keystrokes = new INPUT[inputCount];

		unsigned int currentIndex = 0;
		for (unsigned int i = 0; i < _inputCount; i++)
		{
			if (codepoints[i] <= 0xffff)
			{
				// one UTF-16 code value, one simulated keypress
				keystrokes[currentIndex] = INPUT(unicodePrototype);
				keystrokes[currentIndex].ki.wScan = codepoints[i];
				currentIndex++;
				continue;		// next for
			}
			else
			{
				// UTF-16 surrogate pair, two simulated keypresses
				USHORT highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
				USHORT lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
				keystrokes[currentIndex] = INPUT(unicodePrototype);
				keystrokes[currentIndex + 1] = INPUT(unicodePrototype);
				keystrokes[currentIndex].ki.wScan = highSurrogate;
				keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
				currentIndex += 2;
				continue;
			}
		}	// end for

	}	// end constructor

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::StringOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (keyup)
			return TRUE;
		else if (!repeated)
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

};

struct ScriptOutput : IKeystrokeOutput
{

protected:

	std::wstring filename;

public:

	// std::wstring filename - The UTF-16 string containing a full path to the executable to be
	//		opened when this command is executed. This command does not close the executable.
	ScriptOutput(std::wstring filename) : IKeystrokeOutput()
	{
		this->filename = filename;
	}

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::ScriptOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (repeated || keyup) return TRUE;

		// start process at filename
		
		HINSTANCE retVal = ShellExecute(NULL, L"open", filename.c_str(), L"", NULL, SW_SHOWNORMAL);
		if ((LONG)retVal <= 32)
			return FALSE;
		// TODO: handle errors

		return TRUE;
	}


};

struct DeadKeyOutput : UnicodeOutput
{
private:
	// 0 : no next command
	// 1 : unblocked command
	// 2 : remapped to unicode without valid substitute
	// 3 : remapped to non-unicode
	// 4 : remapped to unicode with a substitute
	// 5 : another dead key
	USHORT nextCommandType;
public:
	// From UnicodeOutput:
	// INPUT * keystrokeDown, INPUT * keystrokeUp
	// USHORT inputCount, UINT codepoint

	// Pointer to hold the command after this one.
	// NULL until input is received; remains null if the next input
	//		is not remapped (i. e. not blocked)
	IKeystrokeOutput * nextCommand;

	// Replacements from Unicode codepoint to Unicode outputs
	std::unordered_map<UINT, UnicodeOutput*> replacements;


	// UINT independentCodepoint - the Unicode character for this dead key
	// UINT * replacements_from - array of codepoints that consist valid sequences
	// UnicodeOutput** replacements_to - array of commands that the codepoints map to
	// UINT replacements_count - number of items in the previous arrays
	DeadKeyOutput(UINT independentCodepoint,
		UINT * replacements_from,  UnicodeOutput** replacements_to, const UINT replacements_count)
		: UnicodeOutput(independentCodepoint), nextCommand(nullptr), nextCommandType(0)
	{
		for (unsigned int i = 0; i < replacements_count; i++) {
			replacements.insert(
				std::pair<UINT, UnicodeOutput*>(replacements_from[i], replacements_to[i])
			);
		}
	}	// end of constructor


	// This should be called at the time the next input is received
	// If the next keystroke is not blocked, vKey must be informed
	VOID setNextCommand(const IKeystrokeOutput* command, const USHORT vKey = 0)
	{
	}

	// Overwrite:
	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::DeadKeyOutput;
	}

	// Overwrite:
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (nextCommandType == 0)		// Then this key is just being called
										//		from another dead key
		{
			// Send just this key's unicode character
			if (!keyup)
				SendInput(inputCount, keystrokes, sizeof(INPUT));
		}
	}

};






// Dummy output that performs no action when executed (good for modifier keys)
struct NoOutput : IKeystrokeOutput
{
public:

	NoOutput() : IKeystrokeOutput() {}
	KeystrokeOutputType getType() { return KeystrokeOutputType::NoOutput; }
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE) { return TRUE; }
};

