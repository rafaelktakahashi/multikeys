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
	MacroOutput,
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

	virtual BOOL simulate(BOOL keyup, BOOL repeated) = 0;
};



struct MacroOutput : IKeystrokeOutput
{

protected:

	INPUT * keystrokes;
	USHORT inputCount;
	BOOL triggerOnRepeat;

public:

	// DWORD * keypressSequence - array of DWORDs, each containing the virtual key code to be
	//		sent, and also the high bit (the 31st, from 0) set in case of a keyup. Every keypress
	//		in this array will be sent in order on execution.
	// USHORT _inputCount - number of elements in keypressSequence
	// BOOL _triggerOnRepeat - true if this command should be triggered multiple times if user
	//		holds down the key
	MacroOutput(const DWORD * keypressSequence, const USHORT _inputCount, const BOOL _triggerOnRepeat)
		: IKeystrokeOutput(), inputCount(_inputCount), triggerOnRepeat(_triggerOnRepeat)
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

	BOOL simulate(BOOL keyup, BOOL repeated)
	{
		if (keyup)
			return TRUE;
		else if (!repeated || (repeated && triggerOnRepeat))
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

};

struct UnicodeOutput : IKeystrokeOutput
{
	
protected:

	INPUT * keystrokes;
	USHORT inputCount;
	BOOL triggerOnRepeat;

public:

	// UINT codepoints - array of UINTs, each containing a single Unicode code point
	//		identifying the character to be sent. All characters in this array will
	//		be sent in order on execution.
	// UINT _inputCount - number of elements in codepoints
	// BOOL _triggerOnRepeat - true if this command should be triggered multiple times if user
	//		holds down the key
	UnicodeOutput(const UINT * codepoints, const UINT _inputCount, const BOOL _triggerOnRepeat)
		: IKeystrokeOutput(), inputCount(_inputCount), triggerOnRepeat(_triggerOnRepeat)
	{
		if (codepoints == nullptr) return;

		for (unsigned int i = 0; i < _inputCount; i++)
		{
			if (codepoints[i] > 0xffff)
				inputCount++;			// <- actual amount of inputs
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
		return KeystrokeOutputType::UnicodeOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated)
	{
		if (keyup)
			return TRUE;
		else if (!repeated || (repeated && triggerOnRepeat))
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

	inline bool operator=(UnicodeOutput& const rhs)
	{
		if (inputCount != rhs.inputCount) return false;
		// One by one, compare the keystroke inputs by codepoint.
		// We trust that the rest of each INPUT structure is the same
		for (int i = 0; i < inputCount; i++)
			if (this->keystrokes[i].ki.wScan != rhs.keystrokes[i].ki.wScan)
				return false;
		return true;
	}
};

struct ScriptOutput : IKeystrokeOutput
{

protected:

	std::wstring filename;
	std::wstring argument;

public:

	// std::wstring filename - The UTF-16 string containing a full path to the executable to be
	//		opened when this command is executed. This command does not close the executable.
	// std::wstring argument - argument to be passed to executable; currently, it mostly exists
	//		for opening URLs with a browser, but multiple arguments might be supported in the future.
	ScriptOutput(std::wstring filename, std::wstring argument = std::wstring())
		: IKeystrokeOutput(), filename(filename), argument(argument)
	{}

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::ScriptOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated)
	{
		if (repeated || keyup) return TRUE;

		// start process at filename
		
		HINSTANCE retVal =
			ShellExecute(NULL, L"open", filename.c_str(), argument.c_str(), NULL, SW_SHOWNORMAL);
		if ((LONG)retVal <= 32)
			return FALSE;
		// TODO: handle errors

		return TRUE;
	}


};

struct DeadKeyOutput : UnicodeOutput
{
	// Inherits keystrokes, keystroke count and trigger on repeat from UnicodeOutput
private:
	// 0 : no next command
	// 1 : unblocked command
	// 2 : remapped to unicode without valid substitute
	// 3 : remapped to non-unicode
	// 4 : remapped to unicode with a substitute
	// (another dead key will behave like a unicode)
	USHORT _nextCommandType;

	// Pointer to hold the command after this one.
	// NULL until input is received; remains null if the next input
	//		is not remapped (i. e. not blocked)
	IKeystrokeOutput * _nextCommand;

	VOID _setNextCommand(IKeystrokeOutput*const command, const USHORT vKey)
	{
		if (command == nullptr)
		{
			// means that the input is not supposed to be blocked
			// Might implement: Do not send this dead key's character if next key is either
			//		escape or tab.
			_nextCommand = nullptr;
			_nextCommandType = 1;
		}
		else
		{
			// first, make sure that the command is unicode:
			if (command->getType() != KeystrokeOutputType::UnicodeOutput
				&& command->getType() != KeystrokeOutputType::DeadKeyOutput)
			{
				_nextCommand = command;
				_nextCommandType = 3;
				return;
			}

			// then, compare its input sequence with the replacement list
			for (auto iterator = replacements.begin(); iterator != replacements.end(); iterator++)
			{
				// try to find the correct one
				if (iterator->first == command)
				{
					// replace it
					_nextCommand = iterator->second;
					_nextCommandType = 4;
				}
			}
			// didn't find a suitable replacement
			_nextCommand = command;
			_nextCommandType = 2;

		}
	}
public:

	
	// Replacements from Unicode codepoint sequence to Unicode outputs
	std::unordered_map<UnicodeOutput*, UnicodeOutput*> replacements;


	// UINT* independentCodepoints - the Unicode character for this dead key
	// UINT independentCodepointCount - the number of unicode characters for this dead key
	// UnicodeOutput ** replacements_from - array of commands that consist valid sequences
	// UnicodeOutput** replacements_to - array of commands that the codepoints map to
	// UINT replacements_count - number of items in the previous arrays
	DeadKeyOutput(UINT*const independentCodepoints, UINT const independentCodepointsCount,
		UnicodeOutput**const replacements_from,  UnicodeOutput**const replacements_to,
		UINT const replacements_count)
		: UnicodeOutput(independentCodepoints, independentCodepointsCount, true),
		_nextCommand(nullptr), _nextCommandType(0)
	{
		for (unsigned int i = 0; i < replacements_count; i++) {
			replacements.insert(
				std::pair<UnicodeOutput*, UnicodeOutput*>(replacements_from[i], replacements_to[i])
			);
		}
	}	// end of constructor
	
	
	
	// Call when next input is received, before using this dead key
	// IkeystrokeOutput* const command - pointer to the command to be executed right after this key;
	//		might be replaced if a match exists
	VOID setNextCommand(IKeystrokeOutput*const command)
	{ _setNextCommand(command, 0); }

	// Call when next input is received, before using this dead key
	// const USHORT vKey - the virtual key input by the user
	VOID setNextCommand(const USHORT vKey)
	{ _setNextCommand(nullptr, vKey); }


	// Overwrite:
	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::DeadKeyOutput;
	}


	// Overwrite:
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (_nextCommandType == 0)		// Then this key is just being called
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

