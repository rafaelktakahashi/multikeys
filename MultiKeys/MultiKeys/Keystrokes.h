#pragma once



// enum classes are strongly typed
enum class KeystrokeOutputType
{
	UnicodeOutput,
	VirtualOutput,
	MacroOutput,
	StringOutput,
	ScriptOutput,
	NoOutput
};

struct IKeystrokeOutput		/*Interface*/
{
	virtual KeystrokeOutputType getType() = 0;

	virtual BOOL simulate(BOOL keyup, BOOL repeated = FALSE) = 0;
};

struct UnicodeOutput : IKeystrokeOutput
{
	INPUT * keystrokesDown;
	INPUT * keystrokesUp;
	USHORT inputCount;
	UINT codepoint;			// Non-surrogate codepoint is for checking dead keys

	UnicodeOutput() : IKeystrokeOutput() {}

	KeystrokeOutputType getType()
	{
		return KeystrokeOutputType::UnicodeOutput;
	}

	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (keyup)
			return (SendInput(inputCount, keystrokesUp, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else
			return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

};

struct VirtualKeyOutput : IKeystrokeOutput
{
	INPUT * keystrokesDown;
	INPUT * keystrokesUp;
	USHORT inputCount;

	VirtualKeyOutput() : IKeystrokeOutput() {}

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
	INPUT * keystrokes;
	USHORT inputCount;

	MacroOutput() : IKeystrokeOutput() {}

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
	INPUT * keystrokes;
	USHORT inputCount;

	StringOutput() : IKeystrokeOutput() {}

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
		
		ShellExecute(NULL, L"open", filename.c_str(), L"", NULL, SW_SHOWNORMAL);

		// TODO: handle errors

		return TRUE;
	}


};

// Dummy output that performs no action when executed (good for dead keys)
struct NoOutput : IKeystrokeOutput
{
public:

	NoOutput() : IKeystrokeOutput() {}
	KeystrokeOutputType getType() { return KeystrokeOutputType::NoOutput; }
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE) { return TRUE; }
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
