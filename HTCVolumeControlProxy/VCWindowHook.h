#pragma once
#ifndef VCWINDOWHOOK_H
#define VCWINDOWHOOK_H

VOID VCWindowHook_Initialize();
VOID VCWindowHook_Deinitialize();

#define prop_OriginalProc L"{edfb0c31-59ca-4cce-a0ea-a4548e8ca0b7}"

VOID KeyboardHook_Initialize();
VOID KeyboardHook_Deinitialize();

#endif