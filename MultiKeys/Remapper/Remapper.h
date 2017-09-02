#pragma once

#include "stdafx.h"
#include "RemapperAPI.h"
#include "KeystrokeCommands.h"
#include "Keyboard.h"

// method readSettings() implemented in a separate cpp.

namespace Multikeys
{

	class Remapper : public IRemapper
	{
	private:

		mutable Scancode workScancode;
		std::vector<Keyboard*> keyboards;

	public:
		Remapper();

		// This will fill the keyboard vector with pointers to allocated keyboards.
		// Implemented in XmlParser.cpp
		bool loadSettings(const std::wstring filename) override;

		bool evaluateKey(
			RAWKEYBOARD* const keypressed,
			wchar_t* const deviceName,
			OUT PKeystrokeCommand* const out_action) const override;

		~Remapper() override;


	};


	// Implementations of factory methods from this library's API are in Remapper.cpp



}

