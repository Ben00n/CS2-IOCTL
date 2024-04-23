#include "Memory.h"

Memory::Memory(HANDLE driverHandle, DWORD processId) : _driverHandle(driverHandle), _processId(processId)
{
    attachToProcess();
}

Memory::~Memory()
{
    CloseHandle(_driverHandle);
}

bool Memory::attachToProcess()
{
    Request req;
    req.processId = reinterpret_cast<HANDLE>(_processId);

    return DeviceIoControl(_driverHandle, ATTACH_CODE, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
}