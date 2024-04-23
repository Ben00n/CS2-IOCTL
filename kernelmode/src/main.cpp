#include <ntifs.h>
#include "driver.h"

NTSTATUS DriverEntry()
{
    UNICODE_STRING driver_name = {};
    RtlInitUnicodeString(&driver_name, L"\\Driver\\BenoonDriver");

    return IoCreateDriver(&driver_name, &driver::driver_main);
}