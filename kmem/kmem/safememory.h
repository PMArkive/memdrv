#pragma once

#define MAX_VIRTUAL_USERMODE 0x7FFFFFFFFFFF
#define MIN_VIRTUAL_USERMODE 0x10000

static unsigned long long TotalFails = 0;

BOOLEAN CopyMemory(PVOID destination, PVOID source, SIZE_T size, BOOLEAN intersects)
{
    switch (size)
	{
        case sizeof(UCHAR) :
    	{
            *(PUCHAR)(destination) = *(PUCHAR)(source);
            break;
        }
        case sizeof(USHORT) :
    	{
            *(PUSHORT)(destination) = *(PUSHORT)(source);
            break;
        }
        case sizeof(ULONG) :
    	{
            *(PULONG)(destination) = *(PULONG)(source);
            break;
        }
#ifdef _AMD64_
        case sizeof(ULONGLONG) :
    	{
            *(PULONGLONG)(destination) = *(PULONGLONG)(source);
            break;
        }
#endif
        default:
    	{
            if (intersects) 
            {
                RtlMoveMemory(
                    (PVOID)(destination),
                    (PVOID)(source),
                    size
                );
            }
            else 
            {
                RtlCopyMemory(
                    (PVOID)(destination),
                    (PVOID)(source),
                    size
                );
            }
        }
    }
    return TRUE;
}

void CopyProcessMemory(PEPROCESS sourceProcess, PVOID sourceAddress, PEPROCESS targetProcess, PVOID targetAddress, SIZE_T size)
{
    __try 
    {
        ULONG tag = 'ihhG';
        PVOID kernelBuffer = ExAllocatePoolWithTag(NonPagedPool, size, tag);

        KAPC_STATE apcState;
        KeStackAttachProcess(sourceProcess, &apcState);

        HANDLE secureMemoryRead = MmSecureVirtualMemory(sourceAddress, size, PAGE_READONLY);
        if (!secureMemoryRead)
        {
            KeUnstackDetachProcess(&apcState);
            ExFreePoolWithTag(kernelBuffer, tag);
            return;
        }

        ProbeForRead(sourceAddress, size, 1);
        CopyMemory(kernelBuffer, sourceAddress, size, FALSE);

        MmUnsecureVirtualMemory(secureMemoryRead);
        KeUnstackDetachProcess(&apcState);

        KeStackAttachProcess(targetProcess, &apcState);
        HANDLE secureMemoryWrite = MmSecureVirtualMemory(targetAddress, size, PAGE_READWRITE);
        if (!secureMemoryWrite)
        {
            KeUnstackDetachProcess(&apcState);
            ExFreePoolWithTag(kernelBuffer, tag);
            return;
        }

        ProbeForWrite(targetAddress, size, 1);
        CopyMemory(targetAddress, kernelBuffer, size, FALSE);

        MmUnsecureVirtualMemory(secureMemoryWrite);
        KeUnstackDetachProcess(&apcState);

        ExFreePoolWithTag(kernelBuffer, tag);
    }
    __except (1)
    {
        // SEH exception handler
        // In ideal scenario should not happen
        TotalFails++; // Windows is high quality and not having anything here = not good
    }
}