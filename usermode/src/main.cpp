#include <iostream>
#include "process_utils.h"
#include "driver_utils.h"

int main() {
    std::cout << "Benoon!\n";

    const DWORD pid = ProcessUtils::get_process_id(L"notepad.exe");

    if (pid == 0) {
        std::cout << "Failed to find notepad\n";
        std::cin.get();
        return 1;
    }

    const HANDLE driverHandle = CreateFile(L"\\\\.\\BenoonDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driverHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create our driver handle.\n";
        std::cin.get();
        return 1;
    }

    if (DriverUtils::attach_to_process(driverHandle, pid)) {
        std::cout << "Attachment successful.\n";
    }

    CloseHandle(driverHandle);

    std::cin.get();

    return 0;
}