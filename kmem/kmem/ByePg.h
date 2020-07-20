#pragma once
#include <ntifs.h>

#define BPG_STATUS_FAILED_TO_HIJACK_HPDT	0xC0013001L
#define BPG_STATUS_FAILED_TO_RESOLVE_INT	0xC0013002L

typedef LONG( __stdcall* BYE_PG_EX_CALLBACK )( CONTEXT* ContextRecord, EXCEPTION_RECORD* ExceptionRecord );

extern "C" NTSTATUS ByePgInitialize( BYE_PG_EX_CALLBACK Callback, BOOLEAN Verbose );