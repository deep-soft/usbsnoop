// RemoveLock.cpp -- Temporary implementation for remove-lock functions
// Generated by Walt Oney's driver wizard

#include "StdDCls.h"
#include "RemoveLock.h"

///////////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE

VOID MyIoInitializeRemoveLock(PMYIO_REMOVE_LOCK lock, ULONG tag, ULONG minutes, ULONG maxcount)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(tag);
    UNREFERENCED_PARAMETER(minutes);
    UNREFERENCED_PARAMETER(maxcount);

    KeInitializeEvent(&lock->evRemove, NotificationEvent, FALSE);
    lock->usage = 1;
    lock->removing = FALSE;
}

///////////////////////////////////////////////////////////////////////////////

#pragma LOCKEDCODE

NTSTATUS MyIoAcquireRemoveLock(PMYIO_REMOVE_LOCK lock, PVOID tag)
{
    UNREFERENCED_PARAMETER(tag);

    InterlockedIncrement(&lock->usage);
    if(lock->removing)
    {
        // removal in progress
        if(0 == InterlockedDecrement(&lock->usage))
        {
            KeSetEvent(&lock->evRemove, 0, FALSE);
        }
        return STATUS_DELETE_PENDING;
    }
    return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

VOID MyIoReleaseRemoveLock(PMYIO_REMOVE_LOCK lock, PVOID tag)
{
    UNREFERENCED_PARAMETER(tag);

    if(0 == InterlockedDecrement(&lock->usage))
    {
        KeSetEvent(&lock->evRemove, 0, FALSE);
    }
}

///////////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE

VOID MyIoReleaseRemoveLockAndWait(PMYIO_REMOVE_LOCK lock, PVOID tag)
{
    PAGED_CODE();
    
    lock->removing = TRUE;
    MyIoReleaseRemoveLock(lock, tag);
    MyIoReleaseRemoveLock(lock, NULL);
    KeWaitForSingleObject(&lock->evRemove, Executive, KernelMode, FALSE, NULL);
}
