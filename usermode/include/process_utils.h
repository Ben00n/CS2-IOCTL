#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <Windows.h>

namespace ProcessUtils {
    DWORD get_process_id(const wchar_t* process_name);
    uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name);
}

#endif