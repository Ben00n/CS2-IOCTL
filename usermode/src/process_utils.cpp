#include "process_utils.h"
#include <TlHelp32.h>

namespace ProcessUtils {
	DWORD get_process_id(const wchar_t* process_name) {
		DWORD process_id = 0;

		HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snap_shot == INVALID_HANDLE_VALUE) return process_id;

		PROCESSENTRY32W entry = {};
		entry.dwSize = sizeof(decltype(entry));

		if (Process32FirstW(snap_shot, &entry) == TRUE)
		{
			// Check if the first handle is the one we want.
			if (_wcsicmp(process_name, entry.szExeFile) == 0) process_id = entry.th32ProcessID;
			else
			{
				while (Process32NextW(snap_shot, &entry) == TRUE)
				{
					if (_wcsicmp(process_name, entry.szExeFile) == 0)
					{
						process_id = entry.th32ProcessID;
						break;
					}
				}
			}
		}

		CloseHandle(snap_shot);

		return process_id;
	}

	uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
	{
		uintptr_t module_base = 0;

		// Snap-shot of process' modules (dlls).
		HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (snap_shot == INVALID_HANDLE_VALUE) return module_base;

		MODULEENTRY32W entry = {};
		entry.dwSize = sizeof(decltype(entry));

		if (Module32FirstW(snap_shot, &entry) == TRUE)
		{
			if (wcsstr(module_name, entry.szModule) != nullptr) module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
			else
			{
				while (Module32NextW(snap_shot, &entry) == TRUE)
				{
					if (wcsstr(module_name, entry.szModule) != nullptr)
					{
						module_base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
						break;
					}
				}
			}
		}

		CloseHandle(snap_shot);

		return module_base;
	}
}