#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <Windows.h>

namespace Multikeys
{
	class Remapper
	{
	public:
		Remapper(std::string filename);
		BOOL Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName);
		BOOL Remapper::ReloadSettings(std::string filename);
	};
}
