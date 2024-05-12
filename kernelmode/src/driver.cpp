#include "driver.h"

namespace benoon_driver
{
    NTSTATUS create_handler(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return irp->IoStatus.Status;
    }

    NTSTATUS close_handler(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return irp->IoStatus.Status;
    }

    NTSTATUS control_handler(PDEVICE_OBJECT device_object, PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        NTSTATUS status = STATUS_UNSUCCESSFUL;

        PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

        auto request = reinterpret_cast<MemoryOperation*>(irp->AssociatedIrp.SystemBuffer);

        if (stack_irp == nullptr || request == nullptr)
        {
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return status;
        }

        static PEPROCESS target_process = nullptr;

        const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
        switch (control_code)
        {
        case codes::attach:
            status = PsLookupProcessByProcessId(request->process_handle, &target_process);
            break;

        case codes::read:
            if (target_process != nullptr)
                status = MmCopyVirtualMemory(target_process, request->address, PsGetCurrentProcess(), request->buffer, request->buffer_size, KernelMode, &request->bytes_transferred);
            break;

        case codes::write:
            status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer, target_process, request->address, request->buffer_size, KernelMode, &request->bytes_transferred);
            break;

        default:
            break;
        }

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = sizeof(MemoryOperation);

        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return status;
    }

    NTSTATUS driver_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
    {
        UNREFERENCED_PARAMETER(registry_path);

        UNICODE_STRING device_name = {};
        RtlInitUnicodeString(&device_name, L"\\Device\\BenoonDriver");

        PDEVICE_OBJECT device_object = nullptr;
        NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

        if (status != STATUS_SUCCESS)
            return status;


        SetFlag(device_object->Flags, DO_BUFFERED_IO);

        driver_object->MajorFunction[IRP_MJ_CREATE] = benoon_driver::create_handler;
        driver_object->MajorFunction[IRP_MJ_CLOSE] = benoon_driver::close_handler;
        driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = benoon_driver::control_handler;

        ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

        return status;
    }
}