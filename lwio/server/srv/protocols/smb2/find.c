/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

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
 *        find.c
 *
 * Abstract:
 *
 *        Likewise IO (LWIO) - SRV
 *
 *        Protocols API - SMBV2
 *
 *        Find/Search
 *
 * Authors: Krishna Ganugapati (kganugapati@likewise.com)
 *          Sriram Nambakam (snambakam@likewise.com)
 *
 *
 */

#include "includes.h"

static
NTSTATUS
SrvFindBothDirInformation(
    IN     PLWIO_SRV_CONNECTION      pConnection,
    IN     PSMB2_MESSAGE             pSmbRequest,
    IN OUT PLWIO_SRV_FILE_2          pFile,
    IN     PSMB2_FIND_REQUEST_HEADER pRequestHeader,
    IN OUT PBYTE                     pDataBuffer,
    IN     ULONG                     ulDataOffset,
    IN     ULONG                     ulMaxDataLength,
    IN OUT PULONG                    pulDataLength
    );

static
NTSTATUS
SrvMarshalBothDirInfoSearchResults(
    PLWIO_SRV_SEARCH_SPACE_2 pSearchSpace,
    PBYTE                    pBuffer,
    ULONG                    ulBytesAvailable,
    ULONG                    ulOffset,
    PULONG                   pulBytesUsed,
    PULONG                   pulSearchCount
    );

NTSTATUS
SrvProcessFind_SMB_V2(
    IN     PSMB2_CONTEXT pContext,
    IN     PSMB2_MESSAGE pSmbRequest,
    IN OUT PSMB_PACKET   pSmbResponse
    )
{
    NTSTATUS ntStatus                = STATUS_SUCCESS;
    PLWIO_SRV_CONNECTION pConnection = pContext->pConnection;
    PLWIO_SRV_SESSION_2 pSession     = NULL;
    PLWIO_SRV_TREE_2    pTree        = NULL;
    PLWIO_SRV_FILE_2    pFile        = NULL;
    PSMB2_FIND_REQUEST_HEADER  pRequestHeader  = NULL; // Do not free
    PSMB2_FIND_RESPONSE_HEADER pResponseHeader = NULL; // Do not free
    UNICODE_STRING             wszFilename     = {0};
    PWSTR    pwszFilePath       = NULL;
    PWSTR    pwszFilesystemPath = NULL;
    PWSTR    pwszSearchPattern = NULL;
    BOOLEAN  bInLock           = FALSE;
    BOOLEAN  bTreeInLock       = FALSE;
    PBYTE    pData             = NULL; // Do not free
    ULONG    ulMaxDataLength   = 0;
    ULONG    ulDataLength      = 0;
    PBYTE pOutBufferRef   = pSmbResponse->pRawBuffer + pSmbResponse->bufferUsed;
    PBYTE pOutBuffer       = pOutBufferRef;
    ULONG ulBytesAvailable = pSmbResponse->bufferLen - pSmbResponse->bufferUsed;
    ULONG ulOffset         = 0;
    ULONG ulDataOffset     = 0;
    ULONG ulBytesUsed      = 0;
    ULONG ulTotalBytesUsed = 0;

    ntStatus = SrvConnection2FindSession_SMB_V2(
                    pContext,
                    pConnection,
                    pSmbRequest->pHeader->ullSessionId,
                    &pSession);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvSession2FindTree_SMB_V2(
                    pContext,
                    pSession,
                    pSmbRequest->pHeader->ulTid,
                    &pTree);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SMB2UnmarshalFindRequest(
                    pSmbRequest,
                    &pRequestHeader,
                    &wszFilename);
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = SrvTree2FindFile_SMB_V2(
                    pContext,
                    pTree,
                    &pRequestHeader->fid,
                    &pFile);
    BAIL_ON_NT_STATUS(ntStatus);

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pFile->mutex);

    if (!pFile->pSearchSpace)
    {
        pFile->searchSpace.ucInfoClass = pRequestHeader->ucInfoClass;
        pFile->searchSpace.ucSearchFlags = pRequestHeader->ucSearchFlags;
        pFile->searchSpace.ulFileIndex = pRequestHeader->ulFileIndex;
        pFile->searchSpace.bUseLongFilenames = TRUE;

        pFile->pSearchSpace = &pFile->searchSpace;

        if (wszFilename.Length)
        {
            wchar16_t wszBackSlash[] = {'\\', 0};

            ntStatus = SrvAllocateMemory(
                            wszFilename.Length + sizeof(wchar16_t),
                            (PVOID*)&pwszSearchPattern);
            BAIL_ON_NT_STATUS(ntStatus);

            memcpy((PBYTE)pwszSearchPattern,
                   (PBYTE)wszFilename.Buffer,
                   wszFilename.Length);

            if (pwszSearchPattern && *pwszSearchPattern == wszBackSlash[0])
            {
                pwszSearchPattern++;
            }
        }

        LWIO_LOCK_RWMUTEX_SHARED(bTreeInLock, &pTree->mutex);

        ntStatus = SrvBuildFilePath(
                        pTree->pShareInfo->pwszPath,
                        pFile->pwszFilename,
                        &pwszFilePath);
        BAIL_ON_NT_STATUS(ntStatus);

        LWIO_UNLOCK_RWMUTEX(bTreeInLock, &pTree->mutex);

        ntStatus = SrvFinderBuildSearchPath(
                        pwszFilePath,
                        pwszSearchPattern,
                        &pwszFilesystemPath,
                        &pFile->searchSpace.pwszSearchPattern);
        BAIL_ON_NT_STATUS(ntStatus);
    }
    else if (pFile->searchSpace.ucInfoClass != pRequestHeader->ucInfoClass)
    {
        ntStatus = STATUS_INVALID_NETWORK_RESPONSE;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    LWIO_UNLOCK_RWMUTEX(bInLock, &pFile->mutex);

    ntStatus = SMB2MarshalHeader(
                    pOutBuffer,
                    ulOffset,
                    ulBytesAvailable,
                    COM2_FIND,
                    0,
                    1,
                    pSmbRequest->pHeader->ulPid,
                    pSmbRequest->pHeader->ullCommandSequence,
                    pTree->ulTid,
                    pSession->ullUid,
                    STATUS_SUCCESS,
                    TRUE,
                    NULL,
                    &ulBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    ulTotalBytesUsed += ulBytesUsed;
    pOutBuffer += ulBytesUsed;
    ulOffset += ulBytesUsed;
    ulBytesAvailable -= ulBytesUsed;

    ntStatus = SMB2MarshalFindResponse(
                    pOutBuffer,
                    ulOffset,
                    ulBytesAvailable,
                    NULL, /* No data */
                    0,
                    &ulDataOffset,
                    &pResponseHeader,
                    &ulBytesUsed);
    BAIL_ON_NT_STATUS(ntStatus);

    ulTotalBytesUsed += ulBytesUsed;
    pOutBuffer += ulBytesUsed;
    ulOffset += ulBytesUsed;
    ulBytesAvailable -= ulBytesUsed;

    pData = pOutBufferRef + ulDataOffset;
    ulMaxDataLength = SMB_MIN(pRequestHeader->ulOutBufferLength,
                              ulBytesAvailable);

    switch (pRequestHeader->ucInfoClass)
    {
        case SMB2_FILE_INFO_CLASS_BOTH_DIR:

            ntStatus = SrvFindBothDirInformation(
                            pConnection,
                            pSmbRequest,
                            pFile,
                            pRequestHeader,
                            pData,
                            ulDataOffset,
                            ulMaxDataLength,
                            &ulDataLength);

            break;

        default:

            ntStatus = STATUS_INVALID_PARAMETER;

            break;
    }
    BAIL_ON_NT_STATUS(ntStatus);

    pResponseHeader->usOutBufferOffset = ulDataOffset;
    pResponseHeader->ulOutBufferLength = ulDataLength;
    if (pResponseHeader->ulOutBufferLength)
    {
        pResponseHeader->usLength++;
    }

    ulTotalBytesUsed += ulDataLength;
    pOutBuffer += ulDataLength;
    ulOffset += ulDataLength;
    ulBytesAvailable -= ulDataLength;

    pSmbResponse->bufferUsed += ulTotalBytesUsed;

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pFile->mutex);

    if (pwszSearchPattern)
    {
        SrvFreeMemory(pwszSearchPattern);
    }

    if (pwszFilesystemPath)
    {
        SrvFreeMemory(pwszFilesystemPath);
    }
    if (pwszFilePath)
    {
        SrvFreeMemory(pwszFilePath);
    }

    if (pFile)
    {
        SrvFile2Release(pFile);
    }

    if (pTree)
    {
        LWIO_UNLOCK_RWMUTEX(bTreeInLock, &pTree->mutex);

        SrvTree2Release(pTree);
    }

    if (pSession)
    {
        SrvSession2Release(pSession);
    }

    return ntStatus;

error:

    if (ulTotalBytesUsed)
    {
        memset(pOutBufferRef, 0, ulTotalBytesUsed);
    }

    goto cleanup;
}

static
NTSTATUS
SrvFindBothDirInformation(
    IN     PLWIO_SRV_CONNECTION      pConnection,
    IN     PSMB2_MESSAGE             pSmbRequest,
    IN OUT PLWIO_SRV_FILE_2          pFile,
    IN     PSMB2_FIND_REQUEST_HEADER pRequestHeader,
    IN OUT PBYTE                     pDataBuffer,
    IN     ULONG                     ulDataOffset,
    IN     ULONG                     ulMaxDataLength,
    IN OUT PULONG                    pulDataLength
    )
{
    NTSTATUS ntStatus = 0;
    BOOLEAN  bInLock = FALSE;
    PBYTE    pData = pDataBuffer;
    ULONG    ulDataLength = 0;
    ULONG    ulSearchResultCount = 0;
    ULONG    ulBytesUsed = 0;
    ULONG    ulOffset = ulDataOffset;
    ULONG    ulBytesAvailable = ulMaxDataLength;
    BOOLEAN  bEndOfSearch = FALSE;
    BOOLEAN  bRestartScan = (pRequestHeader->ucSearchFlags & SMB2_SEARCH_FLAGS_RESTART_SCAN);
    BOOLEAN  bReturnSingleEntry = (pRequestHeader->ucSearchFlags & SMB2_SEARCH_FLAGS_RETURN_SINGLE_ENTRY);
    PFILE_BOTH_DIR_INFORMATION pFileInfoCursor = NULL;
    PLWIO_SRV_SEARCH_SPACE_2 pSearchSpace = NULL;

    LWIO_LOCK_RWMUTEX_EXCLUSIVE(bInLock, &pFile->mutex);

    pSearchSpace = pFile->pSearchSpace;

    if (!pSearchSpace->pFileInfo)
    {
        // Keep space for 10 items
        USHORT usBytesAllocated = (sizeof(FILE_BOTH_DIR_INFORMATION) +
                                   256 * sizeof(wchar16_t)) * 10;

        ntStatus = SrvAllocateMemory(
                        usBytesAllocated,
                        (PVOID*)&pSearchSpace->pFileInfo);
        BAIL_ON_NT_STATUS(ntStatus);

        pSearchSpace->usFileInfoLen = usBytesAllocated;
    }

    while (!bEndOfSearch && ulBytesAvailable)
    {
        ULONG ulIterSearchCount = 0;

        pFileInfoCursor = (PFILE_BOTH_DIR_INFORMATION)pSearchSpace->pFileInfoCursor;
        if (!pFileInfoCursor || !pFileInfoCursor->NextEntryOffset)
        {
            IO_MATCH_FILE_SPEC ioFileSpec;
            IO_STATUS_BLOCK ioStatusBlock = {0};

            ioFileSpec.Type = IO_MATCH_FILE_SPEC_TYPE_UNKNOWN;
            // ioFileSpec.Options = IO_NAME_OPTION_CASE_SENSITIVE;
            RtlUnicodeStringInit(
                            &ioFileSpec.Pattern,
                            pSearchSpace->pwszSearchPattern);

            pSearchSpace->pFileInfoCursor = NULL;

            do
            {
                memset(pSearchSpace->pFileInfo, 0x0, pSearchSpace->usFileInfoLen);

                ntStatus = IoQueryDirectoryFile(
                                pFile->hFile,
                                NULL,
                                &ioStatusBlock,
                                pSearchSpace->pFileInfo,
                                pSearchSpace->usFileInfoLen,
                                FileBothDirectoryInformation,
                                bReturnSingleEntry,
                                &ioFileSpec,
                                bRestartScan);
                if (ntStatus == STATUS_SUCCESS)
                {
                    pSearchSpace->pFileInfoCursor = pSearchSpace->pFileInfo;

                    break;
                }
                else if (ntStatus == STATUS_NO_MORE_MATCHES)
                {
                    bEndOfSearch = TRUE;
                    ntStatus = STATUS_SUCCESS;

                    break;
                }
                else if (ntStatus == STATUS_BUFFER_TOO_SMALL)
                {
                    USHORT usNewSize = pSearchSpace->usFileInfoLen + 256 * sizeof(wchar16_t);

                    ntStatus = SMBReallocMemory(
                                    pSearchSpace->pFileInfo,
                                    (PVOID*)&pSearchSpace->pFileInfo,
                                    usNewSize);
                    BAIL_ON_NT_STATUS(ntStatus);

                    memset(pSearchSpace->pFileInfo + pSearchSpace->usFileInfoLen,
                           0,
                           usNewSize - pSearchSpace->usFileInfoLen);

                    pSearchSpace->usFileInfoLen = usNewSize;

                    continue;
                }
                BAIL_ON_NT_STATUS(ntStatus);

            } while (TRUE);
        }

        if (!bEndOfSearch)
        {
            ntStatus = SrvMarshalBothDirInfoSearchResults(
                            pSearchSpace,
                            pDataBuffer + ulDataLength,
                            ulBytesAvailable,
                            ulOffset,
                            &ulBytesUsed,
                            &ulIterSearchCount);
            BAIL_ON_NT_STATUS(ntStatus);

            if (!ulIterSearchCount)
            {
                break;
            }
            else
            {
                pData += ulBytesUsed;
                ulDataLength += ulBytesUsed;
                ulOffset += ulBytesUsed;
                ulBytesAvailable -= ulBytesUsed;
            }

            ulSearchResultCount += ulIterSearchCount;
        }
    }

    if (!ulSearchResultCount)
    {
        ntStatus = STATUS_NO_MORE_MATCHES;
        BAIL_ON_NT_STATUS(ntStatus);
    }

    *pulDataLength = ulDataLength;

cleanup:

    LWIO_UNLOCK_RWMUTEX(bInLock, &pFile->mutex);

    return ntStatus;

error:

    *pulDataLength = 0;

    goto cleanup;
}

static
NTSTATUS
SrvMarshalBothDirInfoSearchResults(
    PLWIO_SRV_SEARCH_SPACE_2 pSearchSpace,
    PBYTE                    pBuffer,
    ULONG                    ulBytesAvailable,
    ULONG                    ulOffset,
    PULONG                   pulBytesUsed,
    PULONG                   pulSearchCount
    )
{
    NTSTATUS ntStatus = 0;
    PSMB2_FILE_BOTH_DIR_INFO_HEADER pInfoHeader = NULL;
    PFILE_BOTH_DIR_INFORMATION pFileInfoCursor = NULL;
    PBYTE pDataCursor = pBuffer;
    ULONG ulBytesUsed = 0;
    ULONG ulSearchCount = 0;
    ULONG iSearchCount = 0;
    ULONG ulDataOffset = ulOffset;

    pFileInfoCursor = (PFILE_BOTH_DIR_INFORMATION)pSearchSpace->pFileInfoCursor;

    while (pFileInfoCursor && (ulBytesAvailable > 0))
    {
        ULONG ulInfoBytesRequired = 0;

        if (ulDataOffset % 8)
        {
            USHORT usAlignment = (8 - (ulDataOffset % 8));

            ulInfoBytesRequired += usAlignment;
            ulDataOffset += usAlignment;
        }

        ulInfoBytesRequired = sizeof(SMB2_FILE_BOTH_DIR_INFO_HEADER);
        ulDataOffset += sizeof(SMB2_FILE_BOTH_DIR_INFO_HEADER);

        if (pSearchSpace->bUseLongFilenames)
        {
            ulInfoBytesRequired += pFileInfoCursor->FileNameLength;
            ulDataOffset += pFileInfoCursor->FileNameLength;
        }

        if (!pFileInfoCursor->FileNameLength)
        {
            ulInfoBytesRequired += sizeof(wchar16_t);
            ulDataOffset += sizeof(wchar16_t);
        }

        if (ulBytesAvailable < ulInfoBytesRequired)
        {
            break;
        }

        ulSearchCount++;
        ulBytesAvailable -= ulInfoBytesRequired;
        ulBytesUsed += ulInfoBytesRequired;

        if (pFileInfoCursor->NextEntryOffset)
        {
            pFileInfoCursor = (PFILE_BOTH_DIR_INFORMATION)(((PBYTE)pFileInfoCursor) + pFileInfoCursor->NextEntryOffset);
        }
        else
        {
            pFileInfoCursor = NULL;
        }
    }

    for (; iSearchCount < ulSearchCount; iSearchCount++)
    {
        pFileInfoCursor = (PFILE_BOTH_DIR_INFORMATION)pSearchSpace->pFileInfoCursor;

        if (pInfoHeader)
        {
            pInfoHeader->ulNextEntryOffset = ulDataOffset;
        }

        pInfoHeader = (PSMB2_FILE_BOTH_DIR_INFO_HEADER)pDataCursor;

        ulDataOffset = 0;

        if (ulOffset % 8)
        {
            USHORT usAlignment = (8 - (ulOffset % 8));

            pDataCursor += usAlignment;
            ulOffset += usAlignment;
            ulDataOffset += usAlignment;
        }

        pInfoHeader->ulFileIndex = pFileInfoCursor->FileIndex;
        pInfoHeader->ulEaSize = pFileInfoCursor->EaSize;
        pInfoHeader->ullCreationTime = pFileInfoCursor->CreationTime;
        pInfoHeader->ullLastAccessTime = pFileInfoCursor->LastAccessTime;
        pInfoHeader->ullLastWriteTime = pFileInfoCursor->LastWriteTime;
        pInfoHeader->ullChangeTime = pFileInfoCursor->ChangeTime;
        pInfoHeader->ullEndOfFile = pFileInfoCursor->EndOfFile;
        pInfoHeader->ullAllocationSize = pFileInfoCursor->AllocationSize;
        pInfoHeader->ulFileAttributes = pFileInfoCursor->FileAttributes;
        pInfoHeader->ulFileNameLength = pFileInfoCursor->FileNameLength;

        if (!pSearchSpace->bUseLongFilenames)
        {
            memcpy((PBYTE)&pInfoHeader->wszShortName[0],
                   (PBYTE)pFileInfoCursor->ShortName,
                   sizeof(pInfoHeader->wszShortName));

            pInfoHeader->usShortNameLength = pFileInfoCursor->ShortNameLength;
        }
        else
        {
            memset((PBYTE)&pInfoHeader->wszShortName[0],
                   0x0,
                   sizeof(pInfoHeader->wszShortName));

            pInfoHeader->usShortNameLength = 0;
        }

        pDataCursor += sizeof(SMB2_FILE_BOTH_DIR_INFO_HEADER);
        ulDataOffset += sizeof(SMB2_FILE_BOTH_DIR_INFO_HEADER);
        ulOffset += sizeof(SMB2_FILE_BOTH_DIR_INFO_HEADER);

        if (pSearchSpace->bUseLongFilenames && pFileInfoCursor->FileNameLength)
        {
            memcpy(pDataCursor, (PBYTE)pFileInfoCursor->FileName, pFileInfoCursor->FileNameLength);

            pDataCursor += pFileInfoCursor->FileNameLength;
            ulOffset += pFileInfoCursor->FileNameLength;
            ulDataOffset += pFileInfoCursor->FileNameLength;
        }

        if (!pFileInfoCursor->FileNameLength)
        {
            pDataCursor += sizeof(wchar16_t);
            ulOffset += sizeof(wchar16_t);
            ulDataOffset += sizeof(wchar16_t);
        }

        if (pFileInfoCursor->NextEntryOffset)
        {
            pSearchSpace->pFileInfoCursor = (((PBYTE)pFileInfoCursor) + pFileInfoCursor->NextEntryOffset);
        }
        else
        {
            pSearchSpace->pFileInfoCursor = NULL;
        }
    }

    if (pInfoHeader)
    {
        pInfoHeader->ulNextEntryOffset = 0;
    }

    *pulSearchCount = ulSearchCount;
    *pulBytesUsed = ulBytesUsed;

    return ntStatus;
}
