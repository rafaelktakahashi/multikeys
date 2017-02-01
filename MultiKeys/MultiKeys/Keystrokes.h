#pragma once

/*

// Struct for a simulated shortcut or unicode keypress
struct IKeystrokeOutput
{
	BYTE modifiers;		// Extra keys to be pressed with the virtual key. No effect if using unicode
	USHORT vKey;		// Virtual key code to be sent; must be 0 if KEYEVENTF_UNICODE is set
	UINT32 codepoint;	// If KEYEVENTF_UNICODE, send this unicode character instead, ignoring modifiers
	DWORD flags;		// Extra information: we only really use the KEYEVENTF_UNICODE flag. There's KEYUP too.

						// Constructor for a virtual key with modifiers
	IKeystrokeOutput(BYTE _modifiers, USHORT _vKey) : modifiers(_modifiers), vKey(_vKey), flags(0)
	{}

	// Constructor for a unicode character (vKey will be 0)
	IKeystrokeOutput(UINT32 _codepoint) : codepoint(_codepoint), vKey(0), modifiers(0), flags(KEYEVENTF_UNICODE)
	{}


	IKeystrokeOutput() : vKey(0), modifiers(0), flags(0)
	{}
};

*/


// enum classes are strongly typed
enum class KeystrokeOutputType
{
	UnicodeOutput,
	VirtualOutput,
	MacroOutput,
	StringOutput,
	ScriptOutput
};



struct IKeystrokeOutput		/*Interface*/
{

	// Don't get your hopes up, this isn't Java.
	// These are the public attributes of this interface:
	INPUT * keystrokesDown;
	INPUT * keystrokesUp;
	USHORT inputCount;

	

	IKeystrokeOutput()
		: keystrokesDown(nullptr), keystrokesUp(nullptr), inputCount(0)
	{}

	IKeystrokeOutput(INPUT * _keystrokesDown, USHORT _inputCount)
		: keystrokesDown(_keystrokesDown), keystrokesUp(nullptr), inputCount(_inputCount)
	{}

	IKeystrokeOutput(INPUT * _keystrokesDown, INPUT * _keystrokesUp, USHORT _inputCount)
		: keystrokesDown(_keystrokesDown), keystrokesUp(_keystrokesUp), inputCount(inputCount)
	{}


	// Pure virtual functions
	virtual KeystrokeOutputType getType() = 0;
	virtual BOOL simulateKeyDown() = 0;
	virtual BOOL simulateKeyUp() = 0;
};

struct UnicodeOutput : IKeystrokeOutput
{

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::UnicodeOutput;
	}

	BOOL simulateKeyDown()
	{
		return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

	BOOL simulateKeyUp()
	{
		return (SendInput(inputCount, keystrokesUp, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

};

struct VirtualKeyOutput : IKeystrokeOutput
{

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::VirtualOutput;
	}

	BOOL simulateKeyDown()
	{
		return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

	BOOL simulateKeyUp()
	{
		return (SendInput(inputCount, keystrokesUp, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}
};

struct MacroOutput : IKeystrokeOutput
{

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::MacroOutput;
	}

	BOOL simulateKeyDown()
	{
		return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

	BOOL simulateKeyUp()
	{
		return FALSE;
	}
};

struct StringOutput : IKeystrokeOutput
{

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::StringOutput;
	}

	BOOL simulateKeyDown()
	{
		return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

	BOOL simulateKeyUp()
	{
		return FALSE;
	}
};

struct ScriptOutput : IKeystrokeOutput
{

protected:
	std::wstring filename;

public:

	ScriptOutput(std::wstring filename) : IKeystrokeOutput()
	{
		this->filename = filename;
	}

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::ScriptOutput;
	}

	BOOL simulateKeyDown()
	{
		// start process at filename
		
		ShellExecute(NULL, L"open", filename.c_str(), L"", NULL, SW_SHOWNORMAL);

		// TODO: handle errors
	}

	BOOL simulateKeyDown()
	{
		return FALSE;
	}


};


// This struct represents a user keystroke (identified by scancode, not virtual key)
// together with modifiers (which have to be virtual keys)
// and RI_KEY flags (keydown, keyup, and e0/e1 prefixes)
// that may or may not be mapped to a IKeystrokeOutput
struct KEYSTROKE_INPUT
{
	BYTE modifiers;		// Necessary modifiers to activate this trigger (Ctrl, Alt, Win and Shift, left and right)
	USHORT scancode;	// Physical key that activates this trigger
	USHORT flags;		// 0 - RI_KEY_MAKE; 1 - RI_KEY_BREAK; 2 - RI_KEY_E0; 4 - RI_KEY_E1

	KEYSTROKE_INPUT(BYTE _modifiers, USHORT _scancode, USHORT _flags)
		: modifiers(_modifiers), scancode(_scancode), flags(_flags)
	{}

	KEYSTROKE_INPUT(BYTE _modifiers, USHORT _scancode) : modifiers(_modifiers), scancode(_scancode), flags(0)
	{}

	KEYSTROKE_INPUT(USHORT _scancode, USHORT _flags) : modifiers(0), scancode(_scancode), flags(_flags)
	{}

	KEYSTROKE_INPUT(USHORT _scancode) : modifiers(0), scancode(_scancode), flags(0)
	{}

	KEYSTROKE_INPUT() : modifiers(0), scancode(0xff), flags(0)
	{}		// caution - scancode ff represents keyboard error
};


// Since we map from KEYSTROKE_INPUT to IKeystrokeOutput using unordered maps, we need a way to implement a hash
namespace std
{
	template <>
	struct hash<KEYSTROKE_INPUT> : public unary_function<KEYSTROKE_INPUT, size_t>
	{
		size_t operator()(const KEYSTROKE_INPUT& value) const
		{
			return value.scancode % 0x10;
		}
	};

	template <>
	struct equal_to<KEYSTROKE_INPUT> : public unary_function<KEYSTROKE_INPUT, bool>
	{
		bool operator()(const KEYSTROKE_INPUT& x, const KEYSTROKE_INPUT& y) const
		{
			return (x.scancode == y.scancode && x.flags == y.flags);
		}
	};
}
