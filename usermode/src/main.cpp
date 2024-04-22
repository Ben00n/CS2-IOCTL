#include "driver.h"
#include "helpers.h"
#include <iostream>

int main() {
    std::cout << "Benoon!\n";

    const DWORD pid = get_process_id(L"RumbleFighter.exe");

    if (pid == 0) {
        std::cout << "Failed to find Rumble Fighter\n";
        std::cin.get();
        return 1;
    }

    const HANDLE driverHandle = CreateFileW(L"\\\\.\\BenoonDriver", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driverHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create our driver handle.\n";
        std::cin.get();
        return 1;
    }

    Driver driver(driverHandle, pid);

    int value = driver.read_memory<int>(0x02F42380);
    std::cout << value;
    std::cin.get();

    CloseHandle(driverHandle);

    std::cin.get();

    return 0;
}