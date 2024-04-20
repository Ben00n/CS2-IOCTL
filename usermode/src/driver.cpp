#include "driver.h"

Driver::Driver(HANDLE driverHandle, DWORD processId) : _driverHandle(driverHandle), _processId(processId)
{
    attachToProcess();
}

Driver::~Driver()
{
    CloseHandle(_driverHandle);
}

bool Driver::attachToProcess()
{
    Request req;
    req.processId = reinterpret_cast<HANDLE>(_processId);

    return DeviceIoControl(_driverHandle, ATTACH_CODE, &req, sizeof(req), &req, sizeof(req), nullptr, nullptr);
}