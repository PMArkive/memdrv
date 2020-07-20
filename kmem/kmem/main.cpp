/*
 * Copyright (c) 2020 Samuel Tulach - All rights reserved
 * Unauthorized copying of this project, via any medium is strictly prohibited
 */

#include <ntifs.h>
#include <ntimage.h>
#include <minwindef.h>
#include <intrin.h>

#include "shared.h"
#include "utils.h"
#include "ntos.h"
#include "scan.h"
#include "dispatch.h"
#include "ByePg.h"
#include "SEH.h"

#define HANDLE_SEH 1

PVOID GetShellCode(DWORD64 hooked)
{
	/*
		var_18= qword ptr -18h
		arg_0= qword ptr  8
		arg_8= qword ptr  10h

		mov     [rsp+arg_8], rdx
		mov     [rsp+arg_0], rcx
		sub     rsp, 38h
		mov     rax, 7FFFFFFFFFFFFFFFh
		mov     [rsp+38h+var_18], rax
		mov     rdx, [rsp+38h+arg_8]
		mov     rcx, [rsp+38h+arg_0]
		call    [rsp+38h+var_18]
		add     rsp, 38h
		retn
	*/
	const char* code = "\x48\x89\x54\x24\x10\x48\x89\x4C\x24\x08\x48\x83\xEC\x38\x48"
					   "\xB8\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F\x48\x89\x44\x24\x20\x48"
					   "\x8B\x54\x24\x48\x48\x8B\x4C\x24\x40\xFF\x54\x24\x20\x48\x83"
					   "\xC4\x38\xC3";

    PVOID codeBuffer = ExAllocatePool(NonPagedPool, 49);
    if (!codeBuffer)
        return 0;

    memcpy(codeBuffer, code, 49);

    *(DWORD64*)((DWORD64)codeBuffer + 16) = hooked;

    return codeBuffer;
}

extern "C" NTSTATUS CustomEntry(void* dummy1, void* dummy2)
{
    UNREFERENCED_PARAMETER(dummy1);
    UNREFERENCED_PARAMETER(dummy2);

	if (HANDLE_SEH) 
	{
		NTSTATUS ByePgStatus = ByePgInitialize(SEH::HandleException, TRUE);
		if (!NT_SUCCESS(ByePgStatus))
			return CSTATUS_SEH_HANDLER_FAILED;
	}

	DWORD64 diskDriverBase = FindTargetModule("disk.sys");
	if (!diskDriverBase)
		return CSTATUS_MODULE_NOT_FOUND;

	PVOID targetFunction = FindPatternImage((PCHAR)diskDriverBase, "\x40\x55\x48\x8B\xEC\x48\x83\xEC\x40\x83\x65\x10\x00\x48\x8D\x15", "xxxxxxxxxxxxxxxx");
	if (!targetFunction)
		return CSTATUS_SIG_FAILED;

	UNICODE_STRING driverName = RTL_CONSTANT_STRING(L"\\Driver\\Disk");

	PDRIVER_OBJECT object = 0;
	NTSTATUS status = ObReferenceObjectByName(&driverName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (PVOID*)&object);
	if (!NT_SUCCESS(status) || !object)
	{
		return STATUS_NOT_FOUND;
	}

	originalFunction = (void*)object->MajorFunction[IRP_MJ_DEVICE_CONTROL];
	if (INVALID_POINTER(originalFunction))
		return CSTATUS_DRIVER_NOT_FOUND;

	PVOID shellcode = GetShellCode((DWORD64)Hooked);

	_disable();

	unsigned long long cr0 = __readcr0();
	unsigned long long originalCr0 = cr0;
	cr0 &= ~(1UL << 16);
	__writecr0(cr0);

	memcpy(targetFunction, shellcode, 49);

	__writecr0(originalCr0);
	_enable();

	object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)targetFunction;

	ExFreePool(shellcode);
	return STATUS_SUCCESS;
}