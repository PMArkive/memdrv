#pragma once

#pragma once

#define INVALID_POINTER(x) ((DWORD64)x == 0 || (DWORD64)x == 0xffffffffffffffff)

#define CSTATUS_MODULE_NOT_FOUND 0xDEAD0
#define CSTATUS_SIG_FAILED 0xDEAD1
#define CSTATUS_DRIVER_NOT_FOUND 0xDEAD2

typedef struct _Command
{
	int Source;
	int Target;
	int ForceOverwrite;
	DWORD64 SourceAddress;
	DWORD64 TargetAddress;
	DWORD64 Size;
} Command;

#define IOCTL_COMMAND CTL_CODE(FILE_DEVICE_UNKNOWN, 0xFEED, METHOD_IN_DIRECT, FILE_ANY_ACCESS)