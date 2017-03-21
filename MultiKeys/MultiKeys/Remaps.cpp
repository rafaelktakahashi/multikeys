#include "stdafx.h"

#include "Remaps.h"
#include "Keystrokes.h"




BOOL Multikeys::Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeOutput ** out_action)
{
	
	// Look for correct device; return FALSE (= do not block) otherwise
	for (std::vector<KEYBOARD>::iterator iterator = vectorKeyboards.begin();
		iterator != vectorKeyboards.end(); 
		iterator++)
	{
		// checks if iterator->deviceName exists in deviceName (which includes port + device names)
		if (wcsstr(deviceName, iterator->deviceName.c_str()) != nullptr)
		{
			// found the keyboard
			// ask it for what to do
			return iterator->evaluateKeystroke(
				keypressed->MakeCode,
				keypressed->VKey,
				(keypressed->Flags & RI_KEY_E0) == RI_KEY_E0,
				(keypressed->Flags & RI_KEY_E1) == RI_KEY_E1,
				(keypressed->Flags & RI_KEY_BREAK) == RI_KEY_BREAK,
				out_action);
			
		}
	}
	return FALSE;
}







