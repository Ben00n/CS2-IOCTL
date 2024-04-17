#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

static DWORD get_process_id(const wchar_t* process_name) {
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

static std::uintptr_t get_module_base(const DWORD pid, const wchar_t* module_name)
{
	std::uintptr_t module_base = 0;

	// Snap-shot of process' modules (dlls).
	HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snap_shot == INVALID_HANDLE_VALUE) return module_base;

	MODULEENTRY32W entry = {};
	entry.dwSize = sizeof(decltype(entry));

	if (Module32FirstW(snap_shot, &entry) == TRUE)
	{
		if (wcsstr(module_name, entry.szModule) != nullptr) module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		else
		{
			while (Module32NextW(snap_shot, &entry) == TRUE)
			{
				if (wcsstr(module_name, entry.szModule) != nullptr)
				{
					module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}

	CloseHandle(snap_shot);

	return module_base;
}

namespace driver
{
	namespace codes
	{
		// Setup the driver.
		constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// RPM.
		constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

		// WPM.
		constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}

	// Shared between user mode & kernel mode.
	struct Request
	{
		HANDLE process_id;

		PVOID target;
		PVOID buffer;

		SIZE_T size;
		SIZE_T return_size;
	};

	bool attach_to_process(HANDLE driver_handle, const DWORD pid)
	{
		Request req;
		req.process_id = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(driver_handle, codes::attach, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
	}

	template <class T>
	T read_memory(HANDLE driver_handle, const std::uintptr_t addr)
	{
		T temp = {};

		Request req;
		req.target = reinterpret_cast<PVOID>(addr);
		req.buffer = &temp;
		req.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::read, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);

		return temp;
	}

	template <class T>
	void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value)
	{
		Request req;
		req.target = reinterpret_cast<PVOID>(addr);
		req.buffer = (PVOID)&value;
		req.size = sizeof(T);

		DeviceIoControl(driver_handle, codes::write, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
	}
} // namespace driver

int main()
{
	std::cout << "Benoon!\n";

	const DWORD pid = get_process_id(L"notepad.exe");

	if (pid == 0)
	{
		std::cout << "Failed to find notepad\n";
		std::cin.get();
		return 1;
	}

	const HANDLE driverHandle = CreateFile(L"\\\\.\\BenoonDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (driverHandle == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create our driver handle.\n";
		std::cin.get();
		return 1;
	}

	if (driver::attach_to_process(driverHandle, pid) == true)
	{
		std::cout << "Attachment successful.\n";
	}

	CloseHandle(driverHandle);

	std::cin.get();

	return 0;
}