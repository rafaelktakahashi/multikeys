// MultiKeys.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MultiKeys.h"
#include "..\HookRawInput\KeyboardHook.h"
#include "..\Remaps\Remaps.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


// Handle to the main executable window
HWND mainHwnd;
// Windows message for communication between main executable and DLL module
UINT const WM_HOOK = WM_APP + 1;
// How long should hook processing wait for the matching Raw Input message (in ms)
DWORD maxWaitingTime = 100;		// Hopefully no legitimate keystroke should have a delay as long as 0.1s

// Buffer for keyboard Raw Input struct
UINT rawKeyboardBufferSize = 32;
LPBYTE rawKeyboardBuffer = new BYTE[rawKeyboardBufferSize];		// These buffers should be enough,	
// Buffer for keyboard name										// but do allocate more space if needed.
UINT keyboardNameBufferSize = 128;
WCHAR * keyboardNameBuffer = new WCHAR[keyboardNameBufferSize];

// Structure to contain a Raw Input pointer
RAWINPUT * raw;

// Remapper
std::string myString = std::string("F:\\MultiKeys\\configuration");
Multikeys::Remapper remapper = Multikeys::Remapper(myString);

// text to display on screen for debugging. Won't break as long as we're using safe memcpy.
WCHAR* debugText = new WCHAR[DEBUG_TEXT_SIZE];
WCHAR* debugTextKeyboardName = new WCHAR[DEBUG_TEXT_SIZE];
WCHAR* debugTextBeingBlocked = new WCHAR[DEBUG_TEXT_SIZE];


// Buffer for the decisions whether to block the input with Hook
std::deque<DecisionRecord> decisionBuffer;
// Deque because we'll need to iterate through it.



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

	// Setting up everything...
	// (there's nothing yet)


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTIKEYS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTIKEYS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)		// receives a handle to an application instance
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTIKEYS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTIKEYS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,		// The moment when a window is created
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }



   // Store the handle globally
   mainHwnd = hWnd;



   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   
   // Register for receiving Raw Input for keyboards
   RAWINPUTDEVICE rawInputDevice[1];
   rawInputDevice[0].usUsagePage = 1;		// usage page = 1 and usage = 6 is for keyboards
   rawInputDevice[0].usUsage = 6;				// (2 is mouse, 4 is joystick, 6 is keyboard, there are others)
   rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;		// Receive input even if the registered window is in the foreground
   rawInputDevice[0].hwndTarget = hWnd;				// Handle to the target window (NULL would make it follow kb focus)
   RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));

   // Setup the keyboard hook (from the dll)
   InstallHook(hWnd);
   // This call to intall the hook is accesible from here because we included the header from the DLL project
   // the actual DLL is also included as a reference in this project.
   // The hook needs to be executed from a separate dll because it's a global hook.



   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	// Raw Input Message:
	case WM_INPUT:			// <- is a UINT (unsigned int)
	{	// brackets for locality

		UINT bufferSize;		// work variable

		// Get data from the raw input structure
		// Parameters:
		// 1. HRAWINPUT - a handle to the raw input structure (in this case stored in the long param of the message)
		// 2. UINT - a flag containing what to return, either RID_HEADER (header) or RID_INPUT (raw data)
		// 3. LPVOID, out param - a pointer to the data that comes from the RAW INPUT structure
		//		If this is null, the next parameter will contain the required size of the buffer
		// 4. PUINT, in or out param - size of the data, or required size if previous param is NULL
		// 5. UINT - size in bytes of the header

		// get required buffer size, check if it's larger than what we have
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
		if (bufferSize > rawKeyboardBufferSize)
		{
			// Oh, no! Needs more space than we have! Let's replace our buffer with a better one.
			rawKeyboardBufferSize = bufferSize;
			delete[] rawKeyboardBuffer;
			rawKeyboardBuffer = new BYTE[rawKeyboardBufferSize];
			if (DEBUG) OutputDebugString(L"Needed more space for keyboard buffer");
		}

		// load data into buffer
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawKeyboardBuffer, &rawKeyboardBufferSize, sizeof(RAWINPUTHEADER));

		// cast the contents of the buffer into our rawinput pointer
		raw = (RAWINPUT*)rawKeyboardBuffer;

		if (DEBUG)		// Report relevant data, if needed
		{
			WCHAR text[128];		// <- unnecessary allocation, but this is just for debug
			swprintf_s(text, 128, L"Raw Input: virtual key %X scancode %X (%s)\n",
				raw->data.keyboard.VKey,		// virtual keycode
				raw->data.keyboard.MakeCode,	// scancode
				raw->data.keyboard.Flags & RI_KEY_BREAK ? L"up" : L"down");		// keydown or keyup (make/break)
			OutputDebugString(text);
			memcpy_s(debugText, DEBUG_TEXT_SIZE, text, 128);	// will redraw later
		}
		

		// We'll get the device name
		// Check that our buffer is enough for the task:
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, NULL, &bufferSize);
		if (bufferSize > keyboardNameBufferSize)		// It needs more space than we have!
		{
			keyboardNameBufferSize = bufferSize;
			delete[] keyboardNameBuffer;
			keyboardNameBuffer = new WCHAR[keyboardNameBufferSize];
			if (DEBUG) OutputDebugString(L"Needed more space for device name buffer");
		}

		// Load the device name into the buffer
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, keyboardNameBuffer, &keyboardNameBufferSize);
		// Now the buffer contains the name of the device that sent the signal

		if (DEBUG)
		{
			memcpy_s(debugTextKeyboardName, DEBUG_TEXT_SIZE, keyboardNameBuffer, keyboardNameBufferSize);
			// OutputDebugString(keyboardNameBuffer);
		}	// will redraw later

		// Call the function that decides whether to block or allow this keystroke
		// Store that decision in the decisionBuffer; look for it when the hook asks.

		// Check whether to block this key, and store the decision for when the hook asks for it
		BOOL DoBlock = Remaps::EvaluateKey(&(raw->data.keyboard), keyboardNameBuffer);		// ask
		decisionBuffer.push_back(DecisionRecord(raw->data.keyboard, DoBlock));	// remember the answer

		if (DEBUG) RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

		return 0;	// exit Wndproc

	}	// end of case WM_INPUT


	// Message from Hooking DLL
	// It means we need to look for a corresponding Raw Input message that should have arrived before
	// That message is the one that can tell us whether or not to block the key,
	// and this message is the one that can block the key.
	// (or maybe it didn't arrive yet. Checking is necessary.)
	case WM_HOOK:
	{
		// In this message, word parameter contains the virtual key code (not scancode),
		// and long parameter contains whether or not the keypress is a down key.
		USHORT virtualKeyCode = (USHORT)wParam;
		USHORT keyPressed = lParam & 0x80000000 ? 0 : 1;

		WCHAR text[128];
		swprintf_s(text, 128, L"Hook: %X (%d)\n", virtualKeyCode, keyPressed);
		if (DEBUG) OutputDebugString(text);	// <- sends to debug window

		// Check the Raw Input buffer to see if this Hook message is supposed to be blocked; WdnProc returns 1 if it is
		BOOL blockThisHook = FALSE;
		BOOL recordFound = FALSE;
		if (!decisionBuffer.empty())
		{
			int index = 0;
			// Search the buffer for the matching record (it should exist)
			for (std::deque<DecisionRecord>::iterator iterator = decisionBuffer.begin();
				iterator != decisionBuffer.end();
				iterator++, index++)
			{
				if (iterator->keyboardInput.VKey == virtualKeyCode)		// match!
				{
					// Actually, this doesn't guarantee a match:
					// 1. Keys in two different keyboards corresponding to the same virtual key may be pressed in rapid succession
					// 2. Two physical keys may correspond to one virtual key if other remaps have been done. This shouldn't happen.
					// The problem that the hook can't see low level information about the keypress is exactly why this whole thing exists.
					// Making them work together is not easy.
					blockThisHook = iterator->decision;	// this was decided somewhere else
					recordFound = TRUE;		// set the flag
					// Then, remove this and all preceding messages from the buffer
					for (int i = 0; i <= index; i++)		// <- up until and including this one
						decisionBuffer.pop_front();

					// We found the item, so we break the for loop to stop looking:
					break;

				} // end if match

			} // end for search

		} // end if buffer.empty

		// At this point, recordFound is either TRUE or FALSE, and if TRUE, blockThisHook has the correct choice.

		// Wait for the matching Raw Input message if the decision buffer was empty or the matching record wasn't there
		DWORD currentTime, startTime;
		startTime = GetTickCount();			// <- record start time
		// This happens every time and didn't need to.

		// Will only fall into this if we didn't find the correct record.
		// It's a lot of work, but hopefully won't happen frequently.
		while (!recordFound)
		{
			MSG rawMessage;		// We're peeking the message; that is, we're trusting that the very next raw input message
								// will be the one we're waiting for.
								// Also, we can't wait for the other case in this switch to be called, because we'd need
								// to interrupt this case.
			while (!PeekMessage(&rawMessage, mainHwnd, WM_INPUT, WM_INPUT, PM_REMOVE))		// this does not remove the message
			{
				// Test for the maxWaitingTime
				currentTime = GetTickCount();
				// If current time is less than start, the time rolled over to 0
				if ((currentTime < startTime ? ULONG_MAX - startTime + currentTime : currentTime - startTime) > maxWaitingTime)
				{
					// Ignore the Hook message if it exceeded the limit
					if (DEBUG)
					{
						WCHAR text[128];
						swprintf_s(text, 128, L"Hook timed out: %X (%d)\n", virtualKeyCode, keyPressed);
						OutputDebugString(text);
					}
					return 0;
				}

				// Take a nap until checking again.
				Sleep(20);			// Take this out if it causes problems.

			}

			// The Raw Input message has arrived; decide whether to block the input
			// We're still inside that possibility that the message took long to arrive.
			UINT bufferSize;

			// See if we'll need more space
			GetRawInputData((HRAWINPUT)rawMessage.lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
			if (bufferSize > rawKeyboardBufferSize)
			{
				// Need more space!
				rawKeyboardBufferSize = bufferSize;
				delete[] rawKeyboardBuffer;
				rawKeyboardBuffer = new BYTE[rawKeyboardBufferSize];
				OutputDebugString(L"Needed more space for the delayed raw input message.");
			}
			// Load data into the buffer
			GetRawInputData((HRAWINPUT)rawMessage.lParam, RID_INPUT, rawKeyboardBuffer, &rawKeyboardBufferSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)rawKeyboardBuffer;

			// A lot of code here is similar to that in the WM_INPUT case.


			if (DEBUG)		// Report relevant data, if needed
			{
				WCHAR text[128];		// <- unnecessary allocation, but this is just for debug
				swprintf_s(text, 128, L"(delayed) Raw Input: virtual key %X scancode %X (%s)\n",
					raw->data.keyboard.VKey,		// virtual keycode
					raw->data.keyboard.MakeCode,	// scancode
					raw->data.keyboard.Flags & RI_KEY_BREAK ? L"up" : L"down");		// keydown or keyup (make/break)
				OutputDebugString(text);
				memcpy_s(debugText, DEBUG_TEXT_SIZE, text, 128);	// will redraw later
			}

			// Prepare string buffer for the device name
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, NULL, &bufferSize);
			if (bufferSize > keyboardNameBufferSize)
			{
				// Need more space than we have!
				keyboardNameBufferSize = bufferSize;
				delete[] keyboardNameBuffer;
				keyboardNameBuffer = new WCHAR[keyboardNameBufferSize];
				OutputDebugString(L"Needed more space for keyboard name in the delayed raw input message");
			}
			// Load the device name into the buffer
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, keyboardNameBuffer, &keyboardNameBufferSize);

			// Evaluate and take action depending on whether the message is the one we were expecting:

			// If the raw input message doesn't match the hook, push it into the buffer and continue waiting
			if (virtualKeyCode != raw->data.keyboard.VKey)
			{
				// Turns out this raw input message wasn't the one we were looking for.
				// Put it in the queue just like we did in the WM_INPUT case, and keep waiting.
				BOOL doBlock = Remaps::EvaluateKey(&(raw->data.keyboard), keyboardNameBuffer);
				decisionBuffer.push_back(DecisionRecord(raw->data.keyboard, doBlock));
			}
			else
			{
				// This is truly the message we were looking for.
				recordFound = TRUE;		// This will get us out of the loop.
										// (the other way to exit the loop is by timing out)
				// But we still didn't evaluate the raw message (it just arrived!)
				blockThisHook = Remaps::EvaluateKey(&(raw->data.keyboard), keyboardNameBuffer);
			}

		}

		if (DEBUG && blockThisHook)
		{
			swprintf_s(text, 128, L"Keyboard event: %X (%d) is being blocked!\n", virtualKeyCode, keyPressed);
			memcpy_s(debugTextBeingBlocked, DEBUG_TEXT_SIZE, text, 128);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
			OutputDebugString(text);
		}
		else if (DEBUG)
		{
			swprintf_s(text, 128, L"Keyboard event: %X (%d) passed through.\n", virtualKeyCode, keyPressed);
			memcpy_s(debugTextBeingBlocked, DEBUG_TEXT_SIZE, text, 128);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		}

		return blockThisHook;	// exit WndProc, the message caller receives 1 or 0
		// Message caller is the hook dll. It sends a message to this window (this message) upon receival of a hook signal,
		// and blocks it if this message returns 1.

	}	// end of case WM_HOOK


    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			if (DEBUG) TextOut(hdc, 10, 10, debugText, DEBUG_TEXT_SIZE);
			if (DEBUG) TextOut(hdc, 10, 40, debugTextBeingBlocked, DEBUG_TEXT_SIZE);
			if (DEBUG) TextOut(hdc, 10, 80, debugTextKeyboardName, DEBUG_TEXT_SIZE);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		UninstallHook();		// Done using it.
        PostQuitMessage(0);
        break;
    default:		// Give it to someone else.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
