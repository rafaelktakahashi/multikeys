#pragma once

// Copy of dll's header

#pragma once

// Windows types
#include <windef.h>

// did not include (or create) resource.h


#define HOOKINGRAWINPUTDLL_API __declspec(dllexport)

// Exporting prototypes:
HOOKINGRAWINPUTDLL_API BOOL InstallHook(HWND hwndParent);

HOOKINGRAWINPUTDLL_API BOOL UninstallHook();