/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 * Copyright Likewise Software    2004-2009
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        samr_lookupnames.c
 *
 * Abstract:
 *
 *        Remote Procedure Call (RPC) Server Interface
 *
 *        SamrLookupNames function
 *
 * Authors: Rafal Szczesniak (rafal@likewise.com)
 */

#include "includes.h"


NTSTATUS
SamrSrvLookupNames(
    /* [in] */ handle_t hBinding,
    /* [in] */ DOMAIN_HANDLE hDomain,
    /* [in] */ uint32 num_names,
    /* [in] */ UnicodeString *names,
    /* [out] */ Ids *ids,
    /* [out] */ Ids *types
    )
{
    const wchar_t wszFilterFmt[] = L"(%ws=%d AND %ws='%ws' AND %ws='%ws') OR "
                                   L"(%ws=%d AND %ws='%ws' AND %ws='%ws')";
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD dwError = 0;
    PDOMAIN_CONTEXT pDomCtx = NULL;
    HANDLE hDirectory = NULL;
    Ids *pIds = NULL;
    Ids *pTypes = NULL;
    PWSTR pwszDn = NULL;
    PWSTR pwszDomainName = NULL;
    WCHAR wszAttrObjectClass[] = DS_ATTR_OBJECT_CLASS;
    WCHAR wszAttrSamAccountName[] = DS_ATTR_SAM_ACCOUNT_NAME;
    WCHAR wszAttrObjectSid[] = DS_ATTR_OBJECT_SID;
    WCHAR wszAttrDomainName[] = DS_ATTR_DOMAIN;
    DWORD dwObjectClassUser = DS_OBJECT_CLASS_USER;
    DWORD dwObjectClassGroup = DS_OBJECT_CLASS_LOCAL_GROUP;
    DWORD dwScope = 0;
    PWSTR pwszFilter = NULL;
    DWORD dwFilterLen = 0;
    PWSTR wszAttributes[3];
    DWORD i = 0;
    PWSTR pwszName = NULL;
    PDIRECTORY_ENTRY pEntries = NULL;
    PDIRECTORY_ENTRY pEntry = NULL;
    PDIRECTORY_ATTRIBUTE pAttr = NULL;
    PATTRIBUTE_VALUE pAttrVal = NULL;
    DWORD dwEntriesNum = 0;
    PWSTR pwszSid = NULL;
    PSID pSid = NULL;
    DWORD dwObjectClass = 0;
    DWORD dwRid = 0;
    DWORD dwType = 0;
    BOOLEAN bNamesFound = FALSE;
    BOOLEAN bNamesNotFound = FALSE;

    pDomCtx = (PDOMAIN_CONTEXT)hDomain;

    if (pDomCtx == NULL || pDomCtx->Type != SamrContextDomain) {
        ntStatus = STATUS_INVALID_HANDLE;
        BAIL_ON_NTSTATUS_ERROR(ntStatus);
    }

    hDirectory     = pDomCtx->pConnCtx->hDirectory;
    pwszDn         = pDomCtx->pwszDn;
    pwszDomainName = pDomCtx->pwszDomainName;

    ntStatus = SamrSrvAllocateMemory((void**)&pIds,
                                   sizeof(*pIds));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    pIds->count = num_names;
    ntStatus = SamrSrvAllocateMemory((void**)&(pIds->ids),
                                   pIds->count * sizeof(uint32));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    ntStatus = SamrSrvAllocateMemory((void**)&pTypes,
                                   sizeof(*pTypes));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    pTypes->count = num_names;
    ntStatus = SamrSrvAllocateMemory((void**)&(pTypes->ids),
                                   pTypes->count * sizeof(uint32));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    for (i = 0; i < num_names; i++) {
        UnicodeString *name = &(names[i]);

        ntStatus = SamrSrvGetFromUnicodeString(&pwszName,
                                             name);
        BAIL_ON_NTSTATUS_ERROR(ntStatus);

        dwFilterLen = ((sizeof(wszAttrObjectClass) / sizeof(WCHAR)) - 1) +
                      10 +
                      ((sizeof(wszAttrSamAccountName) / sizeof(WCHAR)) - 1) +
                      wc16slen(pwszName) +
                      ((sizeof(wszAttrDomainName) / sizeof(WCHAR)) - 1) +
                      wc16slen(pwszDomainName) +
                      ((sizeof(wszAttrObjectClass) / sizeof(WCHAR)) - 1) +
                      10 +
                      ((sizeof(wszAttrSamAccountName) / sizeof(WCHAR)) - 1) +
                      wc16slen(pwszName) +
                      ((sizeof(wszAttrDomainName) / sizeof(WCHAR)) - 1) +
                      wc16slen(pwszDomainName) +
                      (sizeof(wszFilterFmt) / sizeof(wszFilterFmt[0]));

        ntStatus = SamrSrvAllocateMemory((void**)&pwszFilter,
                                       dwFilterLen * sizeof(WCHAR));
        BAIL_ON_NTSTATUS_ERROR(ntStatus);

        sw16printfw(pwszFilter, dwFilterLen, wszFilterFmt,
                    wszAttrObjectClass, dwObjectClassUser,
                    wszAttrSamAccountName, pwszName,
                    wszAttrDomainName, pwszDomainName,
                    wszAttrObjectClass, dwObjectClassGroup,
                    wszAttrSamAccountName, pwszName,
                    wszAttrDomainName, pwszDomainName);

        wszAttributes[0] = wszAttrObjectSid;
        wszAttributes[1] = wszAttrObjectClass;
        wszAttributes[2] = NULL;

        pEntry   = NULL;
        pAttr    = NULL;
        pAttrVal = NULL;

        dwError = DirectorySearch(hDirectory,
                                  pwszDn,
                                  dwScope,
                                  pwszFilter,
                                  wszAttributes,
                                  FALSE,
                                  &pEntries,
                                  &dwEntriesNum);
        BAIL_ON_LSA_ERROR(dwError);

        if (dwEntriesNum == 1) {
            pEntry  = &(pEntries[0]);

            pwszSid       = NULL;
            dwRid         = 0;
            dwType        = 0;
            dwObjectClass = 0;

            dwError = DirectoryGetEntryAttrValueByName(pEntry,
                                                       wszAttrObjectSid,
                                                       DIRECTORY_ATTR_TYPE_UNICODE_STRING,
                                                       &pwszSid);
            if (pwszSid && dwError == 0) {
                ntStatus = RtlAllocateSidFromWC16String(&pSid, pwszSid);
                BAIL_ON_NTSTATUS_ERROR(ntStatus);

                dwRid = pSid->SubAuthority[pSid->SubAuthorityCount - 1];
            }

            dwError = DirectoryGetEntryAttrValueByName(pEntry,
                                                       wszAttrObjectClass,
                                                       DIRECTORY_ATTR_TYPE_INTEGER,
                                                       &dwObjectClass);
            if (dwError == 0) {
                switch (dwObjectClass) {
                case DS_OBJECT_CLASS_USER:
                    dwType = SID_TYPE_USER;
                    break;

                case DS_OBJECT_CLASS_LOCAL_GROUP:
                    dwType = SID_TYPE_ALIAS;
                    break;

                default:
                    dwType = SID_TYPE_INVALID;
                }

                BAIL_ON_NTSTATUS_ERROR(ntStatus);
            }


            pIds->ids[i]   = dwRid;
            pTypes->ids[i] = dwType;

        } else if (dwEntriesNum == 0) {
            pIds->ids[i]   = 0;
            pTypes->ids[i] = SID_TYPE_UNKNOWN;

            bNamesNotFound = TRUE;

        } else {
            ntStatus = STATUS_INTERNAL_ERROR;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }

        if (pEntries) {
            DirectoryFreeEntries(pEntries, dwEntriesNum);
            pEntries = NULL;
        }

        if (pwszFilter) {
            SamrSrvFreeMemory(pwszFilter);
            pwszFilter = NULL;
        }

        if (pwszName) {
            SamrSrvFreeMemory(pwszName);
            pwszName = NULL;
        }

        if (pSid) {
            RTL_FREE(&pSid);
        }
    }

    ids->count   = pIds->count;
    ids->ids     = pIds->ids;
    types->count = pTypes->count;
    types->ids   = pTypes->ids;

    if (ntStatus == STATUS_SUCCESS) {
        if (bNamesFound && bNamesNotFound) {
            ntStatus = LW_STATUS_SOME_NOT_MAPPED;

        } else if (!bNamesFound && bNamesNotFound) {
            ntStatus = STATUS_NONE_MAPPED;
        }
    }

cleanup:
    if (pwszName) {
        SamrSrvFreeMemory(pwszName);
    }

    if (pSid) {
        RTL_FREE(&pSid);
    }

    if (pwszFilter) {
        SamrSrvFreeMemory(pwszFilter);
    }

    if (pEntries) {
        DirectoryFreeEntries(pEntries, dwEntriesNum);
    }

    if (pIds) {
        SamrSrvFreeMemory(pIds);
    }

    if (pTypes) {
        SamrSrvFreeMemory(pTypes);
    }

    return ntStatus;

error:
    if (pIds->ids) {
        SamrSrvFreeMemory(pIds->ids);
    }

    if (pTypes->ids) {
        SamrSrvFreeMemory(pTypes->ids);
    }

    ids->count   = 0;
    ids->ids     = NULL;
    types->count = 0;
    types->ids   = NULL;

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
