#pragma once
#include "driver.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

DWORD get_process_id(const wchar_t* process_name);
uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name);
uintptr_t find_final_address(Driver& driver, uintptr_t base_address, std::vector<uintptr_t>& offsets);