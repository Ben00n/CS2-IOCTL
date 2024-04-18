#include "driver_utils.h"

namespace DriverUtils {
    bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
        Request req;
        req.process_id = reinterpret_cast<HANDLE>(pid);

        return DeviceIoControl(driver_handle, Codes::Attach, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
    }

    template <typename T>
    T read_memory(HANDLE driver_handle, const uintptr_t addr) {
        T temp = {};

        Request req;
        req.target = reinterpret_cast<PVOID>(addr);
        req.buffer = &temp;
        req.size = sizeof(T);

        DeviceIoControl(driver_handle, Codes::Read, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);

        return temp;
    }

    template <typename T>
    void write_memory(HANDLE driver_handle, const uintptr_t addr, const T& value) {
        Request req;
        req.target = reinterpret_cast<PVOID>(addr);
        req.buffer = (PVOID)&value;
        req.size = sizeof(T);

        DeviceIoControl(driver_handle, Codes::Write, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
    }
}