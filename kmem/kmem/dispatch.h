#pragma once

void* originalFunction = 0;
typedef NTSTATUS(__stdcall* fnOriginal)(PDEVICE_OBJECT device, PIRP irp);

void HandleCommand(Command* cmd)
{
    static PEPROCESS sourceProcess;
	if (!sourceProcess || cmd->ForceOverwrite)
	{
        NTSTATUS status = PsLookupProcessByProcessId((HANDLE)cmd->Source, &sourceProcess);
        if (!NT_SUCCESS(status))
            return;
	}

    static PEPROCESS targetProcess;
    if (!targetProcess || cmd->ForceOverwrite)
    {
        NTSTATUS status = PsLookupProcessByProcessId((HANDLE)cmd->Target, &targetProcess);
        if (!NT_SUCCESS(status))
            return;
    }

    SIZE_T dummySize = 0;
    MmCopyVirtualMemory(sourceProcess, (PVOID)cmd->SourceAddress, targetProcess, (PVOID)cmd->TargetAddress, cmd->Size, KernelMode, &dummySize);
}

NTSTATUS Hooked(PDEVICE_OBJECT device, PIRP irp)
{
    fnOriginal original = (fnOriginal)originalFunction;

    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
    ULONG code = ioc->Parameters.DeviceIoControl.IoControlCode;

    if (code == IOCTL_COMMAND)
    {
        Command* command = (Command*)irp->AssociatedIrp.SystemBuffer;

        HandleCommand(command);

        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = sizeof(command);
        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }

    return original(device, irp);
}