#pragma once
#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

#define MAX_LOADSTRING 100

#ifndef WH_KEYBOARD_LL

// These definitions are found in pwinuser.h in Platform Builder

#define WH_KEYBOARD_LL 20

extern "C" {

typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);

typedef struct tagKBDLLHOOKSTRUCT
{
    DWORD vkCode;        // virtual key code
    DWORD scanCode;      // scan code    DWORD flags;       // flags
    DWORD flags;         // unused
    DWORD time;          // time stamp for this message
    DWORD dwExtraInfo;   // extra info from the driver or keybd_event
}
KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

HHOOK
WINAPI
SetWindowsHookExW(
        int idHook,
        HOOKPROC lpfn,
        HINSTANCE hmod,
        DWORD dwThreadId);

#define SetWindowsHookEx  SetWindowsHookExW

BOOL
WINAPI
UnhookWindowsHookEx(
        HHOOK hhk);

LRESULT
WINAPI
CallNextHookEx(
        HHOOK hhk,
        int nCode,
        WPARAM wParam,
        LPARAM lParam);

}

#endif

VOID KeyboardHook_Initialize();
VOID KeyboardHook_Deinitialize();

#endif