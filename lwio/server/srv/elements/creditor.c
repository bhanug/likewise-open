/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*-
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * Editor Settings: expandtabs and use 4 spaces for indentation */

/*
 * Copyright Likewise Software
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */



/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        creditor.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Elements
 *
 *        Credit Management
 *
 * Authors: Sriram Nambakam (snambakam@likewise.com)
 *
 */

#include "includes.h"

static
NTSTATUS
SrvCreditorAdjustAsyncCredits(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence,
    ULONG64       ullAsyncId,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    );

static
NTSTATUS
SrvCreditorAdjustNormalCredits(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    );

static
NTSTATUS
SrvCreditorInitialize_inlock(
    PSRV_CREDITOR pCreditor
    );

static
VOID
SrvCreditorMoveDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppFromListHead,
    PSRV_DEBITOR* ppFromListTail,
    PSRV_DEBITOR* ppToListHead,
    PSRV_DEBITOR* ppToListTail
    );

static
VOID
SrvCreditorAttachDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppListHead,
    PSRV_DEBITOR* ppListTail
    );

static
VOID
SrvCreditorDetachDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppListHead,
    PSRV_DEBITOR* ppListTail
    );

static
NTSTATUS
SrvCreditorGrantNewCredits_inlock(
    PSRV_CREDITOR pCreditor,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    );

static
NTSTATUS
SrvCreditorFindDebitor_inlock(
    PSRV_DEBITOR  pDebitorList,
    ULONG64       ullSequence,
    PSRV_DEBITOR* ppDebitor
    );

static
VOID
SrvCreditorFreeDebitorList(
    PSRV_DEBITOR pDebitorList
    );

static
NTSTATUS
SrvCreditorAcquireGlobalCredits(
    USHORT  usCreditsRequested,
    PUSHORT pusCreditsGranted
    );

static
VOID
SrvCreditorReturnGlobalCredits(
    USHORT usCredits
    );

NTSTATUS
SrvCreditorCreate(
    PSRV_CREDITOR* ppCreditor
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_CREDITOR pCreditor = NULL;

    ntStatus = SrvAllocateMemory(sizeof(SRV_CREDITOR), (PVOID*)&pCreditor);
    BAIL_ON_NT_STATUS(ntStatus);

    pthread_mutex_init(&pCreditor->mutex, NULL);
    pCreditor->pMutex = &pCreditor->mutex;

    *ppCreditor = pCreditor;

cleanup:

    return ntStatus;

error:

    *ppCreditor = NULL;

    if (pCreditor)
    {
        SrvCreditorFree(pCreditor);
    }

    goto cleanup;
}

NTSTATUS
SrvCreditorAcquireCredit(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN  bInLock  = FALSE;
    PSRV_DEBITOR pDebitor = NULL;

    if (ullSequence == UINT64_MAX)
    {
        goto cleanup;
    }

    LWIO_LOCK_MUTEX(bInLock, &pCreditor->mutex);

    ntStatus = SrvCreditorInitialize_inlock(pCreditor);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvCreditorFindDebitor_inlock(
                    pCreditor->pAvbl_head,
                    ullSequence,
                    &pDebitor);
    BAIL_ON_NT_STATUS(ntStatus);

    // Move from the available to the in-use list
    SrvCreditorMoveDebitor(
            pDebitor,
            &pCreditor->pAvbl_head,
            &pCreditor->pAvbl_tail,
            &pCreditor->pInUse_head,
            &pCreditor->pInUse_tail);

cleanup:

    LWIO_UNLOCK_MUTEX(bInLock, &pCreditor->mutex);

    return ntStatus;

error:

    switch (ntStatus)
    {
        case STATUS_NOT_FOUND:

                ntStatus = STATUS_INVALID_PARAMETER;

                break;

        default:

            break;
    }

    goto cleanup;
}

NTSTATUS
SrvCreditorAdjustCredits(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence,
    ULONG64       ullAsyncId,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN  bInLock  = FALSE;
    USHORT   usCreditsGranted = 0;

    if (pCreditor && (ullSequence != UINT64_MAX))
    {
        if (ullAsyncId)
        {
            ntStatus = SrvCreditorAdjustAsyncCredits(
                            pCreditor,
                            ullSequence,
                            ullAsyncId,
                            usCreditsRequested,
                            &usCreditsGranted);
        }
        else
        {
            ntStatus = SrvCreditorAdjustNormalCredits(
                            pCreditor,
                            ullSequence,
                            usCreditsRequested,
                            &usCreditsGranted);
        }
        BAIL_ON_NT_STATUS(ntStatus);
    }

    *pusCreditsGranted = usCreditsGranted;

cleanup:

    if (pCreditor)
    {
       LWIO_LOCK_MUTEX(bInLock, &pCreditor->mutex);
       pCreditor->usNumCreditsLastGranted = usCreditsGranted;
       LWIO_UNLOCK_MUTEX(bInLock, &pCreditor->mutex);
    }

    return ntStatus;

error:

    *pusCreditsGranted = 0;

    goto cleanup;
}

static
NTSTATUS
SrvCreditorAdjustAsyncCredits(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence,
    ULONG64       ullAsyncId,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT   usCreditsGranted = 0;
    USHORT   usNewCreditsGranted = 0;
    PSRV_DEBITOR pDebitor = NULL;
    PSRV_DEBITOR pNewDebitor = NULL;
    BOOLEAN  bInLock = FALSE;

    LWIO_LOCK_MUTEX(bInLock, &pCreditor->mutex);

    ntStatus = SrvCreditorInitialize_inlock(pCreditor);
    BAIL_ON_NT_STATUS(ntStatus);

    LWIO_ASSERT(pCreditor->usTotalCredits >= pCreditor->CreditFloor);

    ntStatus = SrvCreditorFindDebitor_inlock(
                    pCreditor->pInUse_head,
                    ullSequence,
                    &pDebitor);
    switch (ntStatus)
    {
        case STATUS_SUCCESS:

            // sending interim response. need to assign credits here.

            // Move from in-use list to the interim list
            SrvCreditorMoveDebitor(
                    pDebitor,
                    &pCreditor->pInUse_head,
                    &pCreditor->pInUse_tail,
                    &pCreditor->pInterim_head,
                    &pCreditor->pInterim_tail);

            // Replenish the current credit

            if (usCreditsRequested || pCreditor->usTotalCredits == pCreditor->CreditFloor)
            {
                ntStatus = SrvAllocateMemory(
                                sizeof(SRV_DEBITOR),
                                (PVOID*)&pNewDebitor);
                BAIL_ON_NT_STATUS(ntStatus);

                pNewDebitor->ullSequence = pCreditor->ullNextAvblId++;

                // Attach to the end of the available list
                SrvCreditorAttachDebitor(
                        pNewDebitor,
                        &pCreditor->pAvbl_head,
                        &pCreditor->pAvbl_tail);

                usCreditsGranted++;
            }
            else
            {
                --pCreditor->usTotalCredits;

                SrvCreditorReturnGlobalCredits(1);
            }

            if (usCreditsRequested > usCreditsGranted) // allocate more credits?
            {
                ntStatus = SrvCreditorGrantNewCredits_inlock(
                                pCreditor,
                                usCreditsRequested - usCreditsGranted,
                                &usNewCreditsGranted);
                BAIL_ON_NT_STATUS(ntStatus);

                usCreditsGranted += usNewCreditsGranted;
            }

            break;

        case STATUS_NOT_FOUND:

            // sending actual response (after interim response)
            // just cleanup and don't assign any credits

            ntStatus = SrvCreditorFindDebitor_inlock(
                                pCreditor->pInterim_head,
                                ullSequence,
                                &pDebitor);
            BAIL_ON_NT_STATUS(ntStatus);

            SrvCreditorDetachDebitor(
                    pDebitor,
                    &pCreditor->pInterim_head,
                    &pCreditor->pInterim_tail);

            SrvCreditorFreeDebitorList(pDebitor);

            break;

        default: // some other issue

            break;
    }
    BAIL_ON_NT_STATUS(ntStatus);

    *pusCreditsGranted = usCreditsGranted;

cleanup:

    LWIO_UNLOCK_MUTEX(bInLock, &pCreditor->mutex);

    return ntStatus;

error:

    switch (ntStatus)
    {
        case STATUS_NOT_FOUND:

            ntStatus = STATUS_INTERNAL_ERROR;

            break;

        default:

            break;
    }

    *pusCreditsGranted = usCreditsGranted;

    goto cleanup;
}

static
NTSTATUS
SrvCreditorAdjustNormalCredits(
    PSRV_CREDITOR pCreditor,
    ULONG64       ullSequence,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT   usCreditsGranted = 0;
    USHORT   usNewCreditsGranted = 0;
    PSRV_DEBITOR pDebitor = NULL;
    BOOLEAN  bInLock = FALSE;

    LWIO_LOCK_MUTEX(bInLock, &pCreditor->mutex);

    ntStatus = SrvCreditorInitialize_inlock(pCreditor);
    BAIL_ON_NT_STATUS(ntStatus);

    LWIO_ASSERT(pCreditor->usTotalCredits >= pCreditor->CreditFloor);

    ntStatus = SrvCreditorFindDebitor_inlock(
                    pCreditor->pInUse_head,
                    ullSequence,
                    &pDebitor);
    BAIL_ON_NT_STATUS(ntStatus);

    if (usCreditsRequested || pCreditor->usTotalCredits == pCreditor->CreditFloor)
    {
        pDebitor->ullSequence = pCreditor->ullNextAvblId++;

        // Move from in-use list to the available list
        SrvCreditorMoveDebitor(
                pDebitor,
                &pCreditor->pInUse_head,
                &pCreditor->pInUse_tail,
                &pCreditor->pAvbl_head,
                &pCreditor->pAvbl_tail);

        usCreditsGranted++; // we replenished the current credit
    }
    else
    {
        SrvCreditorDetachDebitor(
                    pDebitor,
                    &pCreditor->pInUse_head,
                    &pCreditor->pInUse_tail);

        SrvCreditorFreeDebitorList(pDebitor);

        --pCreditor->usTotalCredits;

        SrvCreditorReturnGlobalCredits(1);
    }

    if (usCreditsRequested > usCreditsGranted) // allocate more credits?
    {
        ntStatus = SrvCreditorGrantNewCredits_inlock(
                        pCreditor,
                        usCreditsRequested - usCreditsGranted,
                        &usNewCreditsGranted);
        BAIL_ON_NT_STATUS(ntStatus);

        usCreditsGranted += usNewCreditsGranted;
    }

    *pusCreditsGranted = usCreditsGranted;

cleanup:

    LWIO_UNLOCK_MUTEX(bInLock, &pCreditor->mutex);

    return ntStatus;

error:

    switch (ntStatus)
    {
        case STATUS_NOT_FOUND:

            ntStatus = STATUS_INTERNAL_ERROR;

            break;

        default:

            break;
    }

    *pusCreditsGranted = usCreditsGranted;

    goto cleanup;
}

static
NTSTATUS
SrvCreditorInitialize_inlock(
    PSRV_CREDITOR pCreditor
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT   usCreditsGranted = 0;
    PSRV_DEBITOR* ppDebitor = NULL;
    ULONG count = 0;

    if (!pCreditor->bInitialized)
    {
        pCreditor->usCreditLimit = SrvElementsConfigGetClientCreditLimit();
        pCreditor->usCreditStart = SrvElementsConfigGetClientCreditStart();
        pCreditor->CreditFloor = SrvElementsConfigGetClientCreditFloor();

        ntStatus = SrvCreditorAcquireGlobalCredits(
                       pCreditor->CreditFloor,
                       &usCreditsGranted);
        BAIL_ON_NT_STATUS(ntStatus);

        if (usCreditsGranted < pCreditor->CreditFloor)
        {
            ntStatus = LW_STATUS_INSUFFICIENT_RESOURCES;
            BAIL_ON_NT_STATUS(ntStatus);
        }

        ntStatus = SrvAllocateMemory(
                       sizeof(*ppDebitor) * pCreditor->CreditFloor,
                       OUT_PPVOID(&ppDebitor));
        BAIL_ON_NT_STATUS(ntStatus);

        for (count=0; count < pCreditor->CreditFloor; count++)
        {
            ntStatus = SrvAllocateMemory(
                           sizeof(SRV_DEBITOR),
                           OUT_PPVOID(&ppDebitor[count]));
            BAIL_ON_NT_STATUS(ntStatus);

            ppDebitor[count]->ullSequence = pCreditor->ullNextAvblId++;
        }


        for (count=0; count < pCreditor->CreditFloor; count++)
        {
            SrvCreditorAttachDebitor(
                ppDebitor[count],
                &pCreditor->pAvbl_head,
                &pCreditor->pAvbl_tail);
        }

        pCreditor->usTotalCredits += usCreditsGranted;
        pCreditor->usNumCreditsLastGranted = usCreditsGranted;

        pCreditor->bInitialized = TRUE;
    }

cleanup:

    if (ppDebitor)
    {
        SrvFreeMemory(ppDebitor);
    }

    return ntStatus;

error:

    for (count=0;
         ppDebitor && ppDebitor[count] && count < pCreditor->CreditFloor;
         count++)
    {
        SrvCreditorFreeDebitorList(ppDebitor[count]);
    }

    if (usCreditsGranted)
    {
        SrvCreditorReturnGlobalCredits(usCreditsGranted);
    }

    goto cleanup;
}

static
VOID
SrvCreditorMoveDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppFromListHead,
    PSRV_DEBITOR* ppFromListTail,
    PSRV_DEBITOR* ppToListHead,
    PSRV_DEBITOR* ppToListTail
    )
{
    SrvCreditorDetachDebitor(
            pDebitor,
            ppFromListHead,
            ppFromListTail);

    SrvCreditorAttachDebitor(
            pDebitor,
            ppToListHead,
            ppToListTail);
}

static
VOID
SrvCreditorAttachDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppListHead,
    PSRV_DEBITOR* ppListTail
    )
{
    if (!*ppListHead)
    {
        *ppListHead = *ppListTail = pDebitor;
        pDebitor->pPrev = NULL;
    }
    else
    {
        (*ppListTail)->pNext = pDebitor;
        pDebitor->pPrev = *ppListTail;
        *ppListTail = pDebitor;
    }
    pDebitor->pNext = NULL;
}

static
VOID
SrvCreditorDetachDebitor(
    PSRV_DEBITOR  pDebitor,
    PSRV_DEBITOR* ppListHead,
    PSRV_DEBITOR* ppListTail
    )
{
    if (!pDebitor->pPrev && !pDebitor->pNext) // at head of list and only item
    {
        *ppListHead = *ppListTail = NULL;
    }
    else if (!pDebitor->pPrev) // at head of list
    {
        (*ppListHead) = pDebitor->pNext;
        (*ppListHead)->pPrev = NULL;
    }
    else if (!pDebitor->pNext) // at tail of list
    {
        *ppListTail = pDebitor->pPrev;
        (*ppListTail)->pNext = NULL;
    }
    else
    {
        pDebitor->pPrev->pNext = pDebitor->pNext;
        pDebitor->pNext->pPrev = pDebitor->pPrev;
    }

    pDebitor->pPrev = pDebitor->pNext = NULL;
}

static
NTSTATUS
SrvCreditorGrantNewCredits_inlock(
    PSRV_CREDITOR pCreditor,
    USHORT        usCreditsRequested,
    PUSHORT       pusCreditsGranted
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT   usCreditsGranted = 0;
    PSRV_DEBITOR pDebitorList = NULL;

    if (pCreditor->usCreditLimit > pCreditor->usTotalCredits)
    {
        USHORT usCreditsStart = pCreditor->usCreditStart;
        USHORT usCreditsLastGranted = pCreditor->usNumCreditsLastGranted;
        USHORT usCreditsToAcquire = usCreditsStart;

        /* Try to acquire usCreditsStart credits (initialized above).
           If the number of credits requested by the client is less than
           usCreditsStart, try to acquire whatever the client requested.
           Else, slowly ramp up the credits by granting
           LWIO_SRV_CREDIT_INCREMENT more credits than the number of credits
           granted in the last round. If this number exceeds twice the
           usCreditsStart, then start from 2*usCreditsStart again */

        if (usCreditsRequested < usCreditsStart)
        {
            usCreditsToAcquire = usCreditsRequested;
        }
        else if (usCreditsLastGranted >= usCreditsStart)
        {
             usCreditsToAcquire = usCreditsLastGranted +
                  LWIO_SRV_CREDIT_INCREMENT;
             if (usCreditsToAcquire > (2 * usCreditsStart))
             {
                 usCreditsToAcquire = 2 * usCreditsStart;
             }
        }

        if ((usCreditsToAcquire + pCreditor->usTotalCredits) >
            pCreditor->usCreditLimit)
        {
            usCreditsToAcquire = pCreditor->usCreditLimit -
                                 pCreditor->usTotalCredits;
        }

        ntStatus = SrvCreditorAcquireGlobalCredits(
                        usCreditsToAcquire,
                        &usCreditsGranted);
        if (ntStatus == STATUS_SUCCESS)
        {
            PSRV_DEBITOR pDebitor = NULL;
            USHORT       iDebitor = 0;

            for (;iDebitor < usCreditsGranted; iDebitor++)
            {
                ntStatus = SrvAllocateMemory(
                                    sizeof(SRV_DEBITOR),
                                    (PVOID*)&pDebitor);
                BAIL_ON_NT_STATUS(ntStatus);

                pDebitor->pNext = pDebitorList;
                if (pDebitorList)
                {
                    pDebitorList->pPrev = pDebitor;
                }
                pDebitorList = pDebitor;
            }

            // no more errors
            for (pDebitor = pDebitorList; pDebitor; pDebitor = pDebitor->pNext)
            {
                pDebitor->ullSequence = pCreditor->ullNextAvblId++;
            }

            if (!pCreditor->pAvbl_head)
            {
                pCreditor->pAvbl_head = pCreditor->pAvbl_tail = pDebitorList;
            }
            else
            {
                pCreditor->pAvbl_tail->pNext = pDebitorList;
                pDebitorList->pPrev = pCreditor->pAvbl_tail;

                while (pDebitorList->pNext)
                {
                    pDebitorList = pDebitorList->pNext;
                }

                pCreditor->pAvbl_tail = pDebitorList;
            }

            pCreditor->usTotalCredits += usCreditsGranted;
        }
    }

    *pusCreditsGranted = usCreditsGranted;

cleanup:

    return ntStatus;

error:

    *pusCreditsGranted = 0;

    if (usCreditsGranted)
    {
        SrvCreditorReturnGlobalCredits(usCreditsGranted);
    }

    if (pDebitorList)
    {
        SrvCreditorFreeDebitorList(pDebitorList);
    }

    goto cleanup;
}

static
NTSTATUS
SrvCreditorFindDebitor_inlock(
    PSRV_DEBITOR  pDebitorList,
    ULONG64       ullSequence,
    PSRV_DEBITOR* ppDebitor
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PSRV_DEBITOR pCandidate = NULL;

    for (; !pCandidate && pDebitorList; pDebitorList = pDebitorList->pNext)
    {
        if (pDebitorList->ullSequence == ullSequence)
        {
            pCandidate = pDebitorList;
        }
    }

    if (!pCandidate)
    {
        ntStatus = STATUS_NOT_FOUND;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    *ppDebitor = pCandidate;

cleanup:

    return ntStatus;

error:

    *ppDebitor = NULL;

    goto cleanup;
}

VOID
SrvCreditorFree(
    PSRV_CREDITOR pCreditor
    )
{
    if (pCreditor->usTotalCredits)
    {
        SrvCreditorReturnGlobalCredits(pCreditor->usTotalCredits);
    }

    if (pCreditor->pAvbl_head)
    {
        SrvCreditorFreeDebitorList(pCreditor->pAvbl_head);
    }

    if (pCreditor->pInterim_head)
    {
        SrvCreditorFreeDebitorList(pCreditor->pInterim_head);
    }

    if (pCreditor->pInUse_head)
    {
        SrvCreditorFreeDebitorList(pCreditor->pInUse_head);
    }

    if (pCreditor->pMutex)
    {
        pthread_mutex_destroy(&pCreditor->mutex);
        pCreditor->pMutex = NULL;
    }

    SrvFreeMemory(pCreditor);
}

static
VOID
SrvCreditorFreeDebitorList(
    PSRV_DEBITOR pDebitorList
    )
{
    while (pDebitorList)
    {
        PSRV_DEBITOR pDebitor = pDebitorList;

        pDebitorList = pDebitorList->pNext;

        SrvFreeMemory(pDebitor);
    }
}

static
NTSTATUS
SrvCreditorAcquireGlobalCredits(
    USHORT  usCreditsRequested,
    PUSHORT pusCreditsGranted
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN  bInLock = FALSE;
    USHORT   usCreditsGranted = usCreditsRequested;

    LWIO_LOCK_MUTEX(bInLock, gSrvElements.pMutex);

    if (!gSrvElements.ulGlobalCreditLimit)
    {
        ntStatus = STATUS_INSUFF_SERVER_RESOURCES;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    if (gSrvElements.ulGlobalCreditLimit >= usCreditsRequested)
    {
        gSrvElements.ulGlobalCreditLimit -= usCreditsRequested;
    }
    else
    {
        usCreditsGranted = gSrvElements.ulGlobalCreditLimit;
        gSrvElements.ulGlobalCreditLimit = 0;
    }

    *pusCreditsGranted = usCreditsGranted;

cleanup:

    LWIO_UNLOCK_MUTEX(bInLock, gSrvElements.pMutex);

    return ntStatus;

error:

    *pusCreditsGranted = 0;

    goto cleanup;
}

static
VOID
SrvCreditorReturnGlobalCredits(
    USHORT usCredits
    )
{
    BOOLEAN bInLock = FALSE;

    LWIO_LOCK_MUTEX(bInLock, gSrvElements.pMutex);

    gSrvElements.ulGlobalCreditLimit += usCredits;

    LWIO_UNLOCK_MUTEX(bInLock, gSrvElements.pMutex);
}
