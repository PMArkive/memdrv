#pragma once

#define MAX_VIRTUAL_USERMODE 0x7FFFFFFFFFFF
#define MIN_VIRTUAL_USERMODE 0x10000

static unsigned long long TotalFails = 0;

__forceinline BOOLEAN CopyMemory(PVOID destination, PVOID source, SIZE_T size, BOOLEAN intersects)
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

    // Weird except handlers that seem unnecessarry, because the same code that is in except is under it
    // but it's windows and it's exception handling
    __try 
    {
        ProbeForRead(sourceAddress, size, 1);
        CopyMemory(kernelBuffer, sourceAddress, size, FALSE);
    }
    __except (1) 
    {
        MmUnsecureVirtualMemory(secureMemoryRead);
        KeUnstackDetachProcess(&apcState);
        ExFreePoolWithTag(kernelBuffer, tag);
        return;
    }

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

    __try
    {
        ProbeForWrite(targetAddress, size, 1);
        CopyMemory(targetAddress, kernelBuffer, size, FALSE);
    }
    __except (1)
    {
        MmUnsecureVirtualMemory(secureMemoryWrite);
        KeUnstackDetachProcess(&apcState);
        ExFreePoolWithTag(kernelBuffer, tag);
        return;
    }

    MmUnsecureVirtualMemory(secureMemoryWrite);
    KeUnstackDetachProcess(&apcState);

    ExFreePoolWithTag(kernelBuffer, tag);
}