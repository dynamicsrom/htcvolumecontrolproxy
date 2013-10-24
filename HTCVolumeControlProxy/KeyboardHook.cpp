#include "stdafx.h"
#include "KeyboardHook.h"
#include "fnCheckOverlay.h"
#include "regext.h"
#include "pm.h"

extern "C" 
{
	BOOL SetEventData(HANDLE hEvent, DWORD dwData); 
}

HHOOK keyboardHook = NULL;

BOOL isOK = FALSE;
BOOL downAfterRWinKey = FALSE;

int badKeyTimeStamp = 0;
int downKeyTimeStamp = 0;

CRITICAL_SECTION csKbInit;
BOOL isKbReady = FALSE;


// dirty code inspired by HTC keypad driver implementation.
LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
	{
		KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
		
		CEDEVICE_POWER_STATE powerstate;
		DWORD res = GetDevicePower(L"BKL1:", POWER_NAME, &powerstate);
		if (powerstate == 4)
			badKeyTimeStamp = GetTickCount();
	

		if (wParam == WM_KEYDOWN && (kbd->vkCode == 0x75 || kbd->vkCode == 0x76) && isOK == TRUE)
			downAfterRWinKey = TRUE;

		HWND volumeControl = FindWindow(L"VOLUMECONTROL", NULL);
		HWND foregroundWindow = GetForegroundWindow();

		BOOL isOverlay = fnCheckOverlayAP(volumeControl, foregroundWindow);

		if ((foregroundWindow != volumeControl || isOverlay == TRUE) && (kbd->vkCode == VK_UP || kbd->vkCode == VK_DOWN))
			return 1;

		BOOL shouldReturn = FALSE;

		if (downAfterRWinKey == FALSE && (kbd->vkCode == 0x75 || kbd->vkCode == 0x76))
		{
			int ticks = GetTickCount();

			if (wParam == WM_KEYDOWN)
				downKeyTimeStamp = ticks;
			
			if (volumeControl)
			{
				if ((ticks - badKeyTimeStamp) < 2000)
					return 1;

				if ((ticks - downKeyTimeStamp) > 2000)
					return 1;

				HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"GetFromKeyEvent_11");
				DWORD data = (kbd->vkCode << 4) | (wParam == WM_KEYUP ? 1 : 0);
				SetEventData(hEvent, data);
				SetEvent(hEvent);
				CloseHandle(hEvent);
				shouldReturn = TRUE;
			}
		}

		if (wParam == WM_KEYUP && (kbd->vkCode == 0x75 || kbd->vkCode == 0x76) && downAfterRWinKey == TRUE)
			downAfterRWinKey = FALSE;

		if (wParam == WM_KEYUP && kbd->vkCode == 0x5C)
			isOK = FALSE;
		else if (wParam == WM_KEYDOWN && kbd->vkCode == 0x5C)
			isOK = TRUE;

		if (shouldReturn == TRUE)
			return 1;

	}
	return 0;
}

VOID KeyboardHook_InitCriticalSection()
{
	if (isKbReady == FALSE)
		InitializeCriticalSection(&csKbInit);
	isKbReady = TRUE;
}

VOID KeyboardHook_Initialize()
{
	KeyboardHook_InitCriticalSection();
	EnterCriticalSection(&csKbInit);
	if (keyboardHook == NULL)
	{
		keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL,
			(HOOKPROC)HookProc,
			(HINSTANCE)GetModuleHandle(NULL), 
			NULL);
	}
	LeaveCriticalSection(&csKbInit);
}

VOID KeyboardHook_Deinitialize()
{
	KeyboardHook_InitCriticalSection();
	EnterCriticalSection(&csKbInit);
	if (keyboardHook)
	{
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = NULL;
	}
	LeaveCriticalSection(&csKbInit);
}

