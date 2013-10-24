#include "stdafx.h"
#include "regext.h"
#include "HTCVolumeControlProxy.h"

void ClearRaiInfo()
{
	if (settingCombinatedMode == 0)
		return;
	wchar_t str[2];
	str[0] = L'\0';
	RegistrySetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Shell\\Rai\\:TASKBAR_VOLUME", L"1", str);
}

void FlushRaiInfo()
{
	if (settingCombinatedMode != 0)
		RegistrySetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Shell\\Rai\\:TASKBAR_VOLUME", L"1", L"\\Windows\\htcvolumecontrol.exe");
}
