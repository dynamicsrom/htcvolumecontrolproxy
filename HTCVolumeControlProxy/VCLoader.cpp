#include "stdafx.h"
#include "HTCVolumeControlProxy.h"

HMODULE htcVolumeControl = NULL;

HMODULE VCLoader_GetModule()
{
	return htcVolumeControl;
}

ULONG topWindowThread(LPVOID pParam)
{
	HWND hWnd = (HWND)pParam;
	for (int x = 0; x < 10; x++)
	{
		if (IsWindow(hWnd))
			SetForegroundWindow(hWnd);
		Sleep(500);
	}
	return 1;
}

VOID VCLoader_Initialize()
{
	if (htcVolumeControl == NULL)
	{
		htcVolumeControl = LoadLibrary(L"HTCVolumeControl.dll");

		HWND foregroundWindow = GetForegroundWindow();
		if (htcVolumeControl)
		{
			INITIALIZER VolInit = (INITIALIZER)GetProcAddress(htcVolumeControl, L"VOL_Init");
			CloseHandle(CreateThread(NULL, 0, topWindowThread, foregroundWindow, 0, NULL));
			VolInit(0);
		}
	}
}

VOID VCLoader_Deinitialize()
{
	if (htcVolumeControl)
	{
		INITIALIZER VolDeinit = (INITIALIZER)GetProcAddress(htcVolumeControl, L"VOL_Deinit");
		VolDeinit(0);
		FreeLibrary(htcVolumeControl);
	}
	htcVolumeControl = NULL;
}
