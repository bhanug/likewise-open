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
 *        fileRenameInfo.c
 *
 * Abstract:
 *
 *        Likewise Posix File System Driver (PVFS)
 *
 *        FileRenameInformation Handler
 *
 * Authors: Gerald Carter <gcarter@likewise.com>
 */

#include "pvfs.h"

/* Forward declarations */

static NTSTATUS
PvfsSetFileRenameInfo(
    PPVFS_IRP_CONTEXT pIrpContext
    );


/* File Globals */



/* Code */


/****************************************************************
 ***************************************************************/

NTSTATUS
PvfsFileRenameInfo(
    PVFS_INFO_TYPE Type,
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;

    switch(Type)
    {
    case PVFS_SET:
        ntError = PvfsSetFileRenameInfo(pIrpContext);
        break;

    case PVFS_QUERY:
        ntError =  STATUS_NOT_SUPPORTED;
        break;

    default:
        ntError = STATUS_INVALID_PARAMETER;
        break;
    }
    BAIL_ON_NT_STATUS(ntError);

cleanup:
    return ntError;

error:
    goto cleanup;
}

/****************************************************************
 ***************************************************************/

static NTSTATUS
PvfsSetFileRenameInfo(
    PPVFS_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS ntError = STATUS_UNSUCCESSFUL;
    PIRP pIrp = pIrpContext->pIrp;
    PPVFS_CCB pCcb = NULL;
    PPVFS_CCB pNewDirCcb = NULL;
    PFILE_RENAME_INFORMATION pFileInfo = NULL;
    IRP_ARGS_QUERY_SET_INFORMATION Args = pIrpContext->pIrp->Args.QuerySetInformation;
    PSTR pszNewFilename = NULL;

    /* Sanity checks */

    ntError =  PvfsAcquireCCB(pIrp->FileHandle, &pCcb);
    BAIL_ON_NT_STATUS(ntError);

    BAIL_ON_INVALID_PTR(Args.FileInformation, ntError);

    pFileInfo = (PFILE_RENAME_INFORMATION)Args.FileInformation;

    if (pFileInfo->RootDirectory) {
#if 0
        ntError =  PvfsAcquireCCB(pFileInfo->RootDirectory, &pNewDirCcb);
        BAIL_ON_NT_STATUS(ntError);
#else
        /* Not supporting relative renames yet */
        pNewDirCcb = NULL;

        ntError= STATUS_INVALID_PARAMETER;
        BAIL_ON_NT_STATUS(ntError);
#endif
    }

    ntError = PvfsAccessCheckFileHandle(pCcb, DELETE);
    BAIL_ON_NT_STATUS(ntError);

    if (Args.Length < sizeof(*pFileInfo))
    {        ntError = STATUS_BUFFER_TOO_SMALL;
        BAIL_ON_NT_STATUS(ntError);
    }

    /* Real work starts here */

    ntError = PvfsWC16CanonicalPathName(&pszNewFilename, pFileInfo->FileName);
    BAIL_ON_NT_STATUS(ntError);

    /* Check for an existing file if not asked to overwrite */

    if (pFileInfo->ReplaceIfExists == FALSE) {
        PVFS_STAT Stat = {0};

        ntError = PvfsSysStat(pszNewFilename, &Stat);
        if (ntError == STATUS_SUCCESS) {
            ntError = STATUS_OBJECT_NAME_COLLISION;
            BAIL_ON_NT_STATUS(ntError);
        }
    }

    ntError = PvfsValidatePath(pCcb);
    BAIL_ON_NT_STATUS(ntError);

    ntError = PvfsSysRename(pCcb->pszFilename, pszNewFilename);
    BAIL_ON_NT_STATUS(ntError);

    PVFS_SAFE_FREE_MEMORY(pCcb->pszFilename);
    pCcb->pszFilename = pszNewFilename;
    pszNewFilename = NULL;

    pIrp->IoStatusBlock.BytesTransferred = sizeof(*pFileInfo);
    ntError = STATUS_SUCCESS;

cleanup:
    PVFS_SAFE_FREE_MEMORY(pszNewFilename);

    if (pCcb) {
        PvfsReleaseCCB(pCcb);
    }
    if (pNewDirCcb) {
        PvfsReleaseCCB(pNewDirCcb);
    }

    return ntError;

error:
    goto cleanup;
}

/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/

