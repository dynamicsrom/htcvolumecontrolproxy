#pragma once
#ifndef HTCVOLUMECONTROLPROXY_H
#define HTCVOLUMECONTROLPROXY_H

// structures
typedef enum
{
	APISTATE_UNKNOWN,
	APISTATE_NEVER,
	APISTATE_NOT_READY,
	APISTATE_READY
}ApiState;

// definitions
#define GWE_API_EVENT_NAME TEXT("SYSTEM/GweApiSetReady")
#define BOOTPHASE_EVENT_NAME TEXT("SYSTEM/BootPhase2")
#define SHELL_API_EVENT_NAME TEXT("SYSTEM/ShellAPIReady")

extern DWORD settingCombinatedMode;

typedef VOID (*INITIALIZER)(int);

#endif