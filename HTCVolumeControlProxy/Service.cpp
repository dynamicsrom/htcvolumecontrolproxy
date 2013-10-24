#include "stdafx.h"
#include "VCLoader.h"
#include "VCWindowHook.h"
#include "KeyboardHook.h"
#include "service.h"
#include "ShellRai.h"
#include "regext.h"
#include "HTCVolumeControlProxy.h"

DWORD settingCombinatedMode = 0;

DWORD VOL_Close(DWORD dwData)
{
	return 0;
}

DWORD VOL_Deinit(DWORD dwData)
{
	VCLoader_Deinitialize();
	VCWindowHook_Deinitialize();

	return 1;
}

BOOL WaitForAPI(wchar_t *apiName)
{
	// Uses an API if it's ready, waits for it if it's not ready
	ApiState state = APISTATE_UNKNOWN;
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, 0, apiName);
	if (hEvent)
	{
		// Wait for the API set to be ready.
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
		state = APISTATE_READY;
		return TRUE;
	}
	else
	{
		// The API set will never be ready.
		state = APISTATE_NEVER;
		return FALSE;
	}
}

BOOL IsVolumeControlWindow(HWND hWnd)
{
	wchar_t className[50];
	GetClassName(hWnd, className, 50);
	if (wcscmp(className, L"VOLUMECONTROL") == 0 || wcscmp(className, L"VolumeOverlayWnd") == 0 )
		return TRUE;
	return FALSE;
}

ULONG VOL_PreWarmingThread( LPVOID pParam )
{
	RETAILMSG(1, (L"VOL_PreWarmingThread++\n"));
	Sleep(6000);
	HWND foregroundWindow = GetForegroundWindow();
	HWND hWnd = FindWindow(L"VOLUMECONTROL", NULL);
	if (foregroundWindow)
		PostMessage(hWnd, 0x8005, (WPARAM)foregroundWindow, NULL);
	for (int x = 0; x < 1000; x++)
	{
		if (IsVolumeControlWindow(GetForegroundWindow()) == TRUE)
		{
			SetForegroundWindow(foregroundWindow);
		}
		Sleep(1);
	}
	RETAILMSG(1, (L"VOL_PreWarmingThread--\n"));
	return 0;
}

ULONG VOL_StartProxy( LPVOID pParam )
{
	ApiState apiState = APISTATE_READY;

	if (WaitForAPI(GWE_API_EVENT_NAME) == FALSE)
		apiState = APISTATE_NEVER;

	if (WaitForAPI(SHELL_API_EVENT_NAME) == FALSE)
		apiState = APISTATE_NEVER;

	if (apiState == APISTATE_READY)
	{
		int retries = 0;
		while (retries < 20)
		{
			if (FindWindow(L"menu_worker", NULL))
				break;
			Sleep(10000);
			retries++;
		}

		Sleep(60000);

		RegistryGetDWORD(HKEY_LOCAL_MACHINE, L"Software\\HTC\\VolumeOverlay", L"CombinatedMode", &settingCombinatedMode);

		if (settingCombinatedMode)
			ClearRaiInfo();
		HWND foregroundWindow = GetForegroundWindow();
		VCLoader_Initialize();
		if (IsVolumeControlWindow(GetForegroundWindow()) == TRUE)
		{
			if (IsWindow(foregroundWindow) == TRUE)
				SetForegroundWindow(foregroundWindow);
		}
		Sleep(3000);
		if (IsVolumeControlWindow(GetForegroundWindow()) == TRUE)
		{
			if (IsWindow(foregroundWindow) == TRUE)
				SetForegroundWindow(foregroundWindow);
		}
		
		//CloseHandle(CreateThread(NULL, 0, VOL_PreWarmingThread, NULL, 0, NULL));
		VCWindowHook_Initialize();
	}
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
	return 0;
}


DWORD VOL_Init(DWORD dwData)
{
	CloseHandle(CreateThread(0, 0, VOL_StartProxy, 0, 0, 0));
	return 1;
}

BOOL VOL_IOControl(DWORD AData, DWORD ACode, void *ABufIn, 
				   DWORD ASzIn, void *ABufOut, DWORD ASzOut, 
				   DWORD *ARealSzOut) 
{
	switch (ACode) 
	{
	case IOCTL_SERVICE_START:
		return TRUE;
	case IOCTL_SERVICE_STOP:
		return TRUE;
	case IOCTL_SERVICE_STARTED:
		return TRUE;
	case IOCTL_SERVICE_INSTALL: 
		{
			HKEY hKey;
			DWORD dwValue;

			if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"Services\\HTCVOLUME", 0, 
								NULL, 0, 0, NULL, &hKey, &dwValue)) 
				return FALSE;

			// DLL name
			WCHAR dllname[] = L"HTCVolumeControlProxy.dll";
			RegSetValueExW(hKey, L"Dll", 0, REG_SZ, 
				(const BYTE *)dllname, wcslen(dllname) << 1);

			// Setting prefix used to control our service
			RegSetValueExW(hKey, L"Prefix", 0, REG_SZ, (const BYTE *)L"VOL",6);

			// Flags, Index, Context
			dwValue = 0;
			RegSetValueExW(hKey, L"Flags", 0, REG_DWORD, (const BYTE *) &dwValue, 4);
			RegSetValueExW(hKey, L"Index", 0, REG_DWORD, (const BYTE *) &dwValue, 4);
			RegSetValueExW(hKey, L"Context", 0, REG_DWORD, (const BYTE *) &dwValue, 4);

			// Should system keep service alive after initialization?
			dwValue = 1;
			RegSetValueExW(hKey, L"Keep", 0, REG_DWORD, (const BYTE *) &dwValue, 4);

			// Setting load order
			dwValue = 9999;
			RegSetValueExW(hKey, L"Order", 0, REG_DWORD, (const BYTE *) &dwValue, 4);

			RegCloseKey(hKey);
			return TRUE;
		}
	case IOCTL_SERVICE_UNINSTALL: 
		{
			// Uninstalling service from the OS
			HKEY rk;
			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Services", 0, NULL, &rk)) 
				return FALSE;

			RegDeleteKeyW(rk, L"HTCVOLUME");
			RegCloseKey(rk);

			return TRUE;
		}

	case IOCTL_SERVICE_QUERY_CAN_DEINIT:
		{
			memset(ABufOut, 1, ASzOut);
			return TRUE;
		}
	case IOCTL_SERVICE_CONTROL:
		{
			if (ASzIn != 4)
				return FALSE;

			return TRUE;
		}
	default:
		// Unknown control code received
		return FALSE;
	}

	return TRUE;
}


DWORD VOL_Open(DWORD dwData,
			   DWORD dwAccess,
			   DWORD dwShareMode)
{
	return 0;
}

DWORD VOL_Read(DWORD dwData,
			   LPVOID pBuf,
			   DWORD dwLen)
{

	return 0;
}

DWORD VOL_Seek(DWORD dwData,
			   long pos,
			   DWORD type)
{

	return 0;
}

DWORD VOL_Write(DWORD dwData,
				LPCVOID pInBuf,
				DWORD dwInLen)
{

	return 0;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

