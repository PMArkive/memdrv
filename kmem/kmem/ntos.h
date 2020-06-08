#pragma once

extern NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS sourceProcess, PVOID sourceAddress, PEPROCESS targetProcess, PVOID targetAddress, SIZE_T bufferSize, KPROCESSOR_MODE previousMode, PSIZE_T returnSize);
extern NTKERNELAPI NTSTATUS ObReferenceObjectByName(IN PUNICODE_STRING objectName, IN ULONG attributes, IN PACCESS_STATE passedAccessState, IN ACCESS_MASK desiredAccess, IN POBJECT_TYPE objectType, IN KPROCESSOR_MODE accessMode, IN OUT PVOID parseContext, OUT PVOID* object);
extern POBJECT_TYPE* IoDriverObjectType;

extern NTSTATUS ZwQuerySystemInformation(INT systemInformationClass, PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength);

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;