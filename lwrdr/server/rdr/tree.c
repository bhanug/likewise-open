/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

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
 *        tree.c
 *
 * Abstract:
 *
 *        Likewise SMB Subsystem (LSMB)
 *
 *        Common Tree Code
 *
 * Author: Kaya Bekiroglu (kaya@likewisesoftware.com)
 *
 * @todo: add error logging code
 * @todo: switch to NT error codes where appropriate
 */

#include "includes.h"

static
int
SMBTreeHashResponseCompare(
    PCVOID vp1,
    PCVOID vp2
    );

static
size_t
SMBTreeHashResponse(
    PCVOID vp
    );

static
VOID
SMBTreeFree(
    PSMB_TREE pTree
    );

static
DWORD
SMBTreeDestroyContents(
    PSMB_TREE pTree
    );

DWORD
SMBTreeCreate(
    PSMB_TREE* ppTree
    )
{
    DWORD dwError = 0;
    PSMB_TREE pTree = NULL;
    BOOLEAN bDestroyCondition = FALSE;
    BOOLEAN bDestroyMutex = FALSE;
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_t* pMutexAttr = NULL;

    dwError = SMBAllocateMemory(
                sizeof(SMB_TREE),
                (PVOID*)&pTree);
    BAIL_ON_SMB_ERROR(dwError);

    dwError = pthread_mutexattr_init(&mutexAttr);
    BAIL_ON_SMB_ERROR(dwError);

    pMutexAttr = &mutexAttr;

    dwError = pthread_mutexattr_settype(pMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    BAIL_ON_SMB_ERROR(dwError);

    pthread_mutex_init(&pTree->mutex, pMutexAttr);
    bDestroyMutex = TRUE;

    pTree->state = SMB_RESOURCE_STATE_INITIALIZING;
    pTree->error.type = ERROR_SMB;
    pTree->error.smb = SMB_ERROR_SUCCESS;

    dwError = pthread_cond_init(&pTree->event, NULL);
    BAIL_ON_SMB_ERROR(dwError);

    bDestroyCondition = TRUE;

    pTree->refCount = 1;

    /* @todo: find a portable time call which is immune to host date and time
       changes, such as made by ntpd */
    if (time(&pTree->lastActiveTime) == (time_t)-1)
    {
        dwError = errno;
        BAIL_ON_SMB_ERROR(dwError);
    }

    pTree->pSession = NULL;
    pTree->tid = 0;
    pTree->pszPath = NULL;

    dwError = SMBHashCreate(
                19,
                &SMBTreeHashResponseCompare,
                &SMBTreeHashResponse,
                NULL,
                &pTree->pResponseHash);
    BAIL_ON_SMB_ERROR(dwError);

    *ppTree = pTree;

cleanup:

    if (pMutexAttr)
    {
        pthread_mutexattr_destroy(pMutexAttr);
    }

    return dwError;

error:

    if (bDestroyCondition)
    {
        pthread_cond_destroy(&pTree->event);
    }

    if (bDestroyMutex)
    {
        pthread_mutex_destroy(&pTree->mutex);
    }

    if (pTree)
    {
        SMBTreeDestroyContents(pTree);
    }
    SMB_SAFE_FREE_MEMORY(pTree);

    *ppTree = NULL;

    goto cleanup;
}

static
int
SMBTreeHashResponseCompare(
    PCVOID vp1,
    PCVOID vp2)
{
    uint16_t mid1 = *((uint16_t *) vp1);
    uint16_t mid2 = *((uint16_t *) vp2);

    if (mid1 == mid2)
    {
        return 0;
    }
    else if (mid1 > mid2)
    {
        return 1;
    }

    return -1;
}

static
size_t
SMBTreeHashResponse(
    PCVOID vp)
{
    return *((uint16_t *) vp);
}

VOID
SMBTreeAddReference(
    PSMB_TREE pTree
    )
{
    BOOLEAN bInLock = FALSE;

    SMB_LOCK_MUTEX(bInLock, &pTree->pSession->mutex);

    pTree->refCount++;

    SMB_UNLOCK_MUTEX(bInLock, &pTree->pSession->mutex);
}

DWORD
SMBTreeAcquireMid(
    PSMB_TREE pTree,
    uint16_t* pwMid
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    WORD wMid = 0;

    SMB_LOCK_MUTEX(bInLock, &pTree->mutex);

    wMid = pTree->mid++;

    SMB_LOG_DEBUG("Acquired mid [%d] from Tree [0x%x]", wMid, pTree);

    *pwMid = wMid;

    SMB_UNLOCK_MUTEX(bInLock, &pTree->mutex);

    return dwError;
}

DWORD
SMBTreeSetState(
    PSMB_TREE pTree,
    SMB_RESOURCE_STATE state
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;

    SMB_LOCK_MUTEX(bInLock, &pTree->mutex);

    pTree->state = state;

    pthread_cond_broadcast(&pTree->event);

    SMB_UNLOCK_MUTEX(bInLock, &pTree->mutex);

    return dwError;
}

DWORD
SMBTreeInvalidate(
    PSMB_TREE      pTree,
    SMB_ERROR_TYPE errorType,
    uint32_t       errorValue
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bInSessionLock = FALSE;

    SMB_LOCK_MUTEX(bInLock, &pTree->mutex);

    pTree->error.type = errorType;
    pTree->error.smb = errorValue;
    pTree->state = SMB_RESOURCE_STATE_INVALID;

    if (pTree->reverseRef)
    {
        SMB_LOCK_MUTEX(bInSessionLock, &pTree->pSession->mutex);
        SMBHashRemoveKey(pTree->pSession->pTreeHashByPath,
                         &pTree->pszPath);

        SMBHashRemoveKey(pTree->pSession->pTreeHashByTID,
                         &pTree->tid);
        pTree->reverseRef = FALSE;
        SMB_UNLOCK_MUTEX(bInSessionLock, &pTree->pSession->mutex);
    }

    pthread_cond_broadcast(&pTree->event);

    SMB_UNLOCK_MUTEX(bInLock, &pTree->mutex);

    return dwError;
}

VOID
SMBTreeRelease(
    SMB_TREE *pTree
    )
{
    BOOLEAN bInLock = FALSE;

    SMB_LOCK_MUTEX(bInLock, &pTree->pSession->mutex);

    assert(pTree->refCount > 0);

    pTree->refCount--;

    if (!pTree->refCount && !pTree->bShutdown)
    {
        /* The TreeDisconnect() call can cause the tree
           to gain another reference, so mark that we have
           already started shutting down so we don't attempt
           to do so again when the count returns to 0 */
        pTree->bShutdown = TRUE;

        if (pTree->reverseRef)
        {
            SMBHashRemoveKey(pTree->pSession->pTreeHashByPath,
                             &pTree->pszPath);
        }

        /* If the tree is still valid, we need to send
           a tree disconnect request before continuing.  This
           must be done *before* removing the tree from the TID hash */
        if (pTree->state == SMB_RESOURCE_STATE_VALID)
        {
            /* We need to call TreeDisconnect() outside of the lock */
            SMB_UNLOCK_MUTEX(bInLock, &pTree->pSession->mutex);

            TreeDisconnect(pTree);

            SMB_LOCK_MUTEX(bInLock, &pTree->pSession->mutex);
        }

        if (pTree->reverseRef)
        {
            SMBHashRemoveKey(pTree->pSession->pTreeHashByTID,
                             &pTree->tid);
            pTree->reverseRef = FALSE;
        }

        SMB_UNLOCK_MUTEX(bInLock, &pTree->pSession->mutex);

        SMBSessionRelease(pTree->pSession);

        SMBTreeFree(pTree);
    }

    SMB_UNLOCK_MUTEX(bInLock, &pTree->pSession->mutex);
}

static
VOID
SMBTreeFree(
    PSMB_TREE pTree
    )
{
    assert(!pTree->refCount);

    pthread_cond_destroy(&pTree->event);
    pthread_mutex_destroy(&pTree->mutex);

    SMBTreeDestroyContents(pTree);

    SMBFreeMemory(pTree);
}

static
DWORD
SMBTreeDestroyContents(
    PSMB_TREE pTree
    )
{
    SMB_SAFE_FREE_MEMORY(pTree->pszPath);

    /* @todo: assert that the session hash is empty */
    SMBHashSafeFree(&pTree->pResponseHash);

    return 0;
}

DWORD
SMBTreeReceiveResponse(
    IN PSMB_TREE pTree,
    IN BOOLEAN bVerifySignature,
    IN DWORD dwExpectedSequence,
    IN PSMB_RESPONSE pResponse,
    OUT PSMB_PACKET* ppResponsePacket
    )
{
    DWORD dwError = 0;
    BOOLEAN bResponseInLock = FALSE;
    BOOLEAN bTreeInLock = FALSE;
    struct timespec ts = {0, 0};

    // TODO-This function should really just get use pSocket instead ofpTree and
    // use MID allocation from the socket...  so it should become
    // SMBSocketReceiveResponse.

    SMB_LOCK_MUTEX(bResponseInLock, &pResponse->mutex);

    while (!pResponse->state == SMB_RESOURCE_STATE_VALID)
    {
        ts.tv_sec = time(NULL) + 30;
        ts.tv_nsec = 0;

        SMB_LOG_DEBUG("Waiting for response for [mid: %d] and Tree [0x%x]", pResponse->mid, pTree);

retry_wait:

        /* @todo: always verify non-error state after acquiring mutex */
        dwError = pthread_cond_timedwait(
                        &pResponse->event,
                        &pResponse->mutex,
                        &ts);
        if (dwError == ETIMEDOUT)
        {
            if (time(NULL) < ts.tv_sec)
            {
                dwError = 0;
                goto retry_wait;
            }

            /* As long as the socket is active, continue to wait.
             * otherwise, mark the socket as bad and return
             */
            if (SMBSocketTimedOut(pTree->pSession->pSocket))
            {
                SMBSocketInvalidate(
                            pTree->pSession->pSocket,
                            ERROR_SMB,
                            ETIMEDOUT);

                SMBResponseInvalidate(pResponse, ERROR_SMB, ETIMEDOUT);
            }
            else
            {
                // continue waiting
                dwError = SMB_ERROR_SUCCESS;
            }
        }
        BAIL_ON_SMB_ERROR(dwError);
    }

    SMB_UNLOCK_MUTEX(bResponseInLock, &pResponse->mutex);

    SMB_LOCK_MUTEX(bTreeInLock, &pTree->mutex);

    SMB_LOG_DEBUG("Removing response [mid: %d] from Tree [0x%x]", pResponse->mid, pTree);

    dwError = SMBHashRemoveKey(
                    pTree->pResponseHash,
                    &pResponse->mid);
    BAIL_ON_SMB_ERROR(dwError);

    /* @todo: this need be set only when the hash is empty */
    pTree->lastActiveTime = time(NULL);

    dwError = SMBPacketDecodeHeader(
                    pResponse->pPacket,
                    bVerifySignature,
                    dwExpectedSequence,
                    pTree->pSession->pSocket->pSessionKey,
                    pTree->pSession->pSocket->dwSessionKeyLength);
    BAIL_ON_SMB_ERROR(dwError);

    /* Could be NULL on error */
    *ppResponsePacket = pResponse->pPacket;
    pResponse->pPacket = NULL;

cleanup:

    SMB_UNLOCK_MUTEX(bTreeInLock, &pTree->mutex);

    SMB_UNLOCK_MUTEX(bResponseInLock, &pResponse->mutex);

    return dwError;

error:

    *ppResponsePacket = NULL;

    goto cleanup;
}

DWORD
SMBTreeFindLockedResponseByMID(
    PSMB_TREE      pTree,
    uint16_t       wMid,
    PSMB_RESPONSE* ppResponse
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bResponseInLock = FALSE;
    PSMB_RESPONSE pResponse = NULL;

    SMB_LOCK_MUTEX(bInLock, &pTree->mutex);

    SMB_LOG_DEBUG("Trying to find response [mid: %d] in Tree [0x%x]", wMid, pTree);

    dwError = SMBHashGetValue(
                    pTree->pResponseHash,
                    &wMid,
                    (PVOID *) &pResponse);
    BAIL_ON_SMB_ERROR(dwError);

    SMB_UNLOCK_MUTEX(bInLock, &pTree->mutex);

    SMB_LOG_DEBUG("Locking response [mid: %d] in Tree [0x%x]", wMid, pTree);

    SMB_LOCK_MUTEX(bResponseInLock, &pResponse->mutex);

    SMB_LOG_DEBUG("Locked response [mid: %d] in Tree [0x%x]", wMid, pTree);

    *ppResponse = pResponse;

cleanup:

    SMB_UNLOCK_MUTEX(bInLock, &pTree->mutex);

    return dwError;

error:

    *ppResponse = NULL;

    goto cleanup;
}
