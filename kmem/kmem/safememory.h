#pragma once

#define MAX_VIRTUAL_USERMODE 0x7FFFFFFFFFFF
#define MIN_VIRTUAL_USERMODE 0x10000

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
    ULONG tag = 'ihaG';
    PVOID kernelBuffer = ExAllocatePoolWithTag(NonPagedPool, size, tag);
    SIZE_T transfered = 0;

    KAPC_STATE apcState;
    KeStackAttachProcess(sourceProcess, &apcState);

    MM_COPY_ADDRESS copyAddress;
    copyAddress.VirtualAddress = sourceAddress;
    MmCopyMemory(kernelBuffer, copyAddress, size, MM_COPY_MEMORY_VIRTUAL, &transfered);
	
    KeUnstackDetachProcess(&apcState);

    KeStackAttachProcess(targetProcess, &apcState);

    copyAddress.VirtualAddress = kernelBuffer;
    MmCopyMemory(targetAddress, copyAddress, size, MM_COPY_MEMORY_VIRTUAL, &transfered);
	
    KeUnstackDetachProcess(&apcState);

    ExFreePoolWithTag(kernelBuffer, tag);
}