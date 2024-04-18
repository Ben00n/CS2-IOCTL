#pragma once
#ifndef DRIVER_UTILS_H
#define DRIVER_UTILS_H

#include <Windows.h>

namespace DriverUtils {
    namespace Codes {
        constexpr ULONG Attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG Read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG Write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }

    struct Request {
        HANDLE process_id;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
        SIZE_T return_size;
    };

    bool attach_to_process(HANDLE driver_handle, const DWORD pid);

    template <typename T>
    T read_memory(HANDLE driver_handle, const uintptr_t addr);

    template <typename T>
    void write_memory(HANDLE driver_handle, const uintptr_t addr, const T& value);
}

#endif