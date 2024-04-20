#pragma once
#include <Windows.h>
#include <cstdint>

class Driver {
public:
    Driver(HANDLE driverHandle, DWORD processId);
    ~Driver();

    template <typename T>
    T read_memory(const std::uintptr_t addr)
    {
        T temp = {};

        Request req;
        req.target = reinterpret_cast<PVOID>(addr);
        req.buffer = &temp;
        req.size = sizeof(T);

        DeviceIoControl(_driverHandle, READ_CODE, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);

        return temp;
    }

    template <typename T>
    void write_memory(const std::uintptr_t addr, const T& value)
    {
        Request req;
        req.target = reinterpret_cast<PVOID>(addr);
        req.buffer = (PVOID)&value;
        req.size = sizeof(T);

        DeviceIoControl(_driverHandle, WRITE_CODE, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
    }

private:
    bool attachToProcess();

    HANDLE _driverHandle;
    DWORD _processId;

    static constexpr ULONG ATTACH_CODE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG READ_CODE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    static constexpr ULONG WRITE_CODE = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

    struct Request {
        HANDLE processId;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
        SIZE_T returnSize;
    };
};