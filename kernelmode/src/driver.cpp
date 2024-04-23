#include "driver.h"

namespace driver
{
	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}

	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}

	NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp)
	{
		UNREFERENCED_PARAMETER(device_object);

		NTSTATUS status = STATUS_UNSUCCESSFUL;

		// Determine which code was passed through.
		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		// Access the request object sent from user mode.
		auto request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr)
		{
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		// the target process we want access to.
		static PEPROCESS target_process = nullptr;

		const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
		switch (control_code)
		{
		case codes::attach:
			status = PsLookupProcessByProcessId(request->process_id, &target_process);
			break;

		case codes::read:
			if (target_process != nullptr)
				status = MmCopyVirtualMemory(target_process, request->target, PsGetCurrentProcess(), request->buffer, request->size, KernelMode, &request->return_size);
			break;

		case codes::write:
			status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer, target_process, request->target, request->size, KernelMode, &request->return_size);
			break;

		default:
			break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(Request);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}

	NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
	{
		UNREFERENCED_PARAMETER(registry_path);

		UNICODE_STRING device_name = {};
		RtlInitUnicodeString(&device_name, L"\\Device\\BenoonDriver");

		// Create Driver Device obj.
		PDEVICE_OBJECT device_object = nullptr;
		NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

		if (status != STATUS_SUCCESS)
		{
			return status;
		}


		UNICODE_STRING symbolic_link = {};
		RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\BenoonDriver");

		status = IoCreateSymbolicLink(&symbolic_link, &device_name);
		if (status != STATUS_SUCCESS)
		{
			return status;
		}

		// Allow us to send small amounts of data between user and kernel modes.
		SetFlag(device_object->Flags, DO_BUFFERED_IO);

		// Set the driver handlers to our functions with our logic.
		driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
		driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
		driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

		// We have initialized our device.
		ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

		return status;
	}
}