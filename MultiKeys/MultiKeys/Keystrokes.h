#pragma once



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

	// It does not seem to be necessary to send keyups when sending unicode characters
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE)
	{
		if (keyup)
			 return (SendInput(inputCount, keystrokesUp, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else
			return (SendInput(inputCount, keystrokesDown, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
	}

};

// The difference between this and MacroOutput is that this one will keep being sent while user
// holds down the mapped key. Macros only activate once.
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

	// Constructor:
	DeadKeyOutput() : UnicodeOutput(), nextCommand(nullptr), nextCommandType(0)
	{}


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
			// Both keydown and keyup
			if (!keyup)
			{
				SendInput(inputCount, keystrokesDown, sizeof(INPUT));
				SendInput(inputCount, keystrokesUp, sizeof(INPUT));
			}
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

