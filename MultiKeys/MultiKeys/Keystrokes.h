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

// Dummy output that performs no action when executed (good for dead keys)
struct NoOutput : IKeystrokeOutput
{
public:

	NoOutput() : IKeystrokeOutput() {}
	KeystrokeOutputType getType() { return KeystrokeOutputType::NoOutput; }
	BOOL simulate(BOOL keyup, BOOL repeated = FALSE) { return TRUE; }
};

