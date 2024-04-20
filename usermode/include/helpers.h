#pragma once
#include <Windows.h>
#include <TlHelp32.h>

DWORD get_process_id(const wchar_t* process_name);
uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name);