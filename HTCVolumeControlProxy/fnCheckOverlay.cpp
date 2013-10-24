/* 
	fnCheckOverlay module.
*/
#include "stdafx.h"
#include "regext.h"

// The worst code ever. Ported directly from assembly.

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 100

static BOOL isInhouseAPClass(wchar_t *path, wchar_t *className)
{
	HKEY hKey = NULL;

	BOOL result = FALSE;
	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		path,
		0,
		KEY_READ,
		&hKey) == ERROR_SUCCESS
		)
	{
		TCHAR    achKey[MAX_KEY_LENGTH]; 
		DWORD    cbName;  
		TCHAR    achClass[MAX_PATH] = TEXT(""); 
		DWORD    cchClassName = MAX_PATH; 
		DWORD    cSubKeys=0; 
		DWORD    cbMaxSubKey;
		DWORD    cchMaxClass; 
		DWORD    cValues;  
		DWORD    cchMaxValue;
		DWORD    cbMaxValueData;
		DWORD    cbSecurityDescriptor;
		FILETIME ftLastWriteTime;

		DWORD i, retCode; 

		DWORD cchValue = MAX_VALUE_NAME; 

		retCode = RegQueryInfoKey(
			hKey,
			achClass,
			&cchClassName,
			NULL,
			&cSubKeys,
			&cbMaxSubKey,
			&cchMaxClass,
			&cValues,
			&cchMaxValue,
			&cbMaxValueData,
			&cbSecurityDescriptor,
			&ftLastWriteTime);

		if (cSubKeys)
		{
			for (i = 0; i < cSubKeys; ++i) 
			{ 
				cbName = MAX_KEY_LENGTH;
				retCode = RegEnumKeyEx(hKey, i,
					achKey, 
					&cbName, 
					NULL, 
					NULL, 
					NULL, 
					&ftLastWriteTime); 
				if (retCode == ERROR_SUCCESS) 
				{
					wchar_t regPath[150];
					swprintf(regPath, 
						L"Software\\HTC\\VolumeOverlay\\InhouseAPClass\\%ls", 
						achKey);

					wchar_t newClassName[50];
					RegistryGetString(HKEY_LOCAL_MACHINE, regPath, L"ClassName", 
						newClassName, 50);
					
					if (wcsstr(newClassName, className))
					{
						result = TRUE;
						goto L_ToEnd;
					}
				}
			}
		}
	}
L_ToEnd:
	RegCloseKey(hKey);
	return result;
}

static BOOL isLandscape()
{
	if (GetSystemMetrics(SM_CXSCREEN) > GetSystemMetrics(SM_CYSCREEN))
		return TRUE;
	return FALSE;
}

BOOL fnCheckOverlayAP(HWND hWnd, HWND foregroundWindow)
{
L_start:
	char mem[0x206];
	char mem2[0x206];

	memset(mem, 0, 0x206);
	memset(mem2, 0, 0x206);

	HWND ownerWindow = GetWindow(foregroundWindow, GW_OWNER);
	wchar_t foregroundWindowClass[130];
	wchar_t ownerWindowClass[130];
	if (foregroundWindow)
		GetClassName(foregroundWindow, foregroundWindowClass, 130);
	if (ownerWindow)
		GetClassName(ownerWindow, ownerWindowClass, 130);

	wchar_t registryPath[0x206];
	memset(registryPath, 0, 0x206 * sizeof(wchar_t));

	wchar_t registryPath2[0x206];
	memset(registryPath2, 0, 0x206 * sizeof(wchar_t));
	swprintf(registryPath, L"%ls\\%ls", 
		L"Software\\HTC\\VolumeOverlay\\InhouseEnable2ndUI", 
		foregroundWindowClass);
	DWORD ownerWindowEnable = 0;
	DWORD registryReadResult = 0;

#define INVERT(a) if (a == S_OK) a = 1; else a = 0;
	
	registryReadResult = RegistryGetDWORD(HKEY_LOCAL_MACHINE, 
						registryPath, L"OwnerWindowEnable", &ownerWindowEnable);
	INVERT(registryReadResult);
	if (registryReadResult == 0)
	{
		swprintf(registryPath2, 
			L"%ls\\%ls", L"Software\\HTC\\VolumeOverlay\\InhouseEnable2ndUI", 
			ownerWindowClass);
		registryReadResult = RegistryGetDWORD(HKEY_LOCAL_MACHINE, 
			registryPath2, L"OwnerWindowEnable", &ownerWindowEnable);
		INVERT(registryReadResult);
	}

	BOOL isInhouse = isInhouseAPClass(L"Software\\HTC\\VolumeOverlay\\InhouseAPClass", 
		foregroundWindowClass);
	if (registryReadResult == TRUE && ownerWindowEnable == TRUE)
	{
		isInhouse = isInhouseAPClass(L"Software\\HTC\\VolumeOverlay\\InhouseAPClass", 
			ownerWindowClass);
	}
	if (isInhouse == TRUE)
	{
		// found in InhouseAPClass reg
		DWORD enable2ndOverlay = FALSE;
		DWORD ret = RegistryGetDWORD(HKEY_LOCAL_MACHINE, registryPath, 
			L"Enable2ndOverlay", &enable2ndOverlay);
		INVERT(ret);

		if (registryReadResult == TRUE && ownerWindowEnable == TRUE)
		{
			swprintf(registryPath2, L"%ls\\%ls", 
				L"Software\\HTC\\VolumeOverlay\\InhouseEnable2ndUI", 
				ownerWindowClass);
			ret = RegistryGetDWORD(HKEY_LOCAL_MACHINE, registryPath2, 
				L"Enable2ndOverlay", &enable2ndOverlay);
			INVERT(ret);
		}
		if (ret == 0)
		{
			goto L_foreground;
		}
		else
		{
			if (enable2ndOverlay)
			{
				goto L_3dparty;
			}
			else
			{
				goto L_foreground;
			}
		}
	}
	else
	{
		BOOL is3dParty = isInhouseAPClass(L"Software\\HTC\\VolumeOverlay\\3rdPartyAPClass", 
											foregroundWindowClass);
		if (is3dParty == TRUE)
		{
L_3dparty:
			return TRUE;
			/*
			// This funny code was used during initial porting. Don't laugh.
			if (isLandscape() == TRUE)
			{
				// Show 2nd L in L
				if (foregroundWindow)
				{
					return TRUE;
				}
				else
				{
					// fnCheckOverlayAP no Parent
					return TRUE;
				}
			}
			else
			{
				// Show 2nd P in P
				if (foregroundWindow)
				{
					return TRUE;
				}
				else
				{
					// fnCheckOverlayAP no Parent
					return TRUE;
				}
			}
			*/
		}
		else if (is3dParty == FALSE)
		{
			is3dParty = isInhouseAPClass(L"Software\\HTC\\VolumeOverlay\\3rdPartyAPClassL", 
											foregroundWindowClass);
			if (is3dParty == FALSE)
			{
L_foreground:
				if (GetParent(foregroundWindow) == NULL)
				{
					return FALSE; // fnCheckOverlayAP no Parent
				}
				else
				{
					foregroundWindow = GetParent(foregroundWindow);
					goto L_start;
				}
			}
			else
			{
				return TRUE;
				/*
				// funny code
				// Show 2nd L
				if (foregroundWindow)
				{
					return TRUE;
				}
				else
				{
					// fnCheckOverlayAP no Parent
					return TRUE;
				}
				*/
			}
		}
	}
	return FALSE;
}
