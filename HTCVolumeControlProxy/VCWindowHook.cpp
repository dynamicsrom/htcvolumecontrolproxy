#include "stdafx.h"
#include "VCWindowHook.h"
#include "regext.h"
#include "ShellRai.h"

extern DWORD settingCombinatedMode;

static int firstRun = 1;

static LRESULT VCWindowHook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == 0x8015)
	{
		if (firstRun == 1)
		{
			FlushRaiInfo();
			firstRun = 0;
		}

		KeyboardHook_Initialize();
		uMsg = 0x8005;

		PostMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	}
	else if (uMsg == 0x8005)
	{
		if (firstRun == 1)
		{
			FlushRaiInfo();
			firstRun = 0;
		}
		KeyboardHook_Initialize();
	}
	else if (uMsg == WM_TIMER)
	{
		if (wParam == 3)
		{
			KeyboardHook_Deinitialize();
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		KeyboardHook_Deinitialize();
	}
	else if (uMsg == WM_KILLFOCUS)
	{
		KeyboardHook_Deinitialize();
	}
	else if (uMsg == WM_ACTIVATE)
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			KeyboardHook_Deinitialize();
		}
	}
	LONG old = (LONG)GetProp(hWnd, prop_OriginalProc);
	return CallWindowProc((WNDPROC)old, hWnd, uMsg, wParam, lParam);
}

VOID VCWindowHook_Initialize()
{
	HWND hWnd = FindWindow(L"VOLUMECONTROL", NULL);

	if (hWnd)
	{
		if (GetProp(hWnd, prop_OriginalProc) == NULL)
			SetProp(hWnd, prop_OriginalProc, (HANDLE)GetWindowLong(hWnd, GWL_WNDPROC));

		SetWindowLong(hWnd, GWL_WNDPROC, (LONG) VCWindowHook_WndProc);
	}
}

VOID VCWindowHook_Deinitialize()
{
	HWND hWnd = FindWindow(L"VOLUMECONTROL", NULL);

	if (hWnd)
	{
		LONG oldWndProc = (LONG) GetProp(hWnd, prop_OriginalProc);

		if (oldWndProc)
		{
			SetWindowLong(hWnd, GWL_WNDPROC, (LONG) oldWndProc);
			RemoveProp(hWnd, prop_OriginalProc);
		}
	}
}
