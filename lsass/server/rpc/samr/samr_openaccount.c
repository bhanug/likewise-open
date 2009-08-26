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
 *        samr_openaccount.c
 *
 * Abstract:
 *
 *        Remote Procedure Call (RPC) Server Interface
 *
 *        SamrOpenAccount function
 *
 * Authors: Rafal Szczesniak (rafal@likewise.com)
 */

#include "includes.h"


NTSTATUS
SamrSrvOpenAccount(
    /* [in] */ handle_t hBinding,
    /* [in] */ DOMAIN_HANDLE *hDomain,
    /* [in] */ uint32 access_mask,
    /* [in] */ uint32 rid,
    /* [in] */ uint32 objectClass,
    /* [out] */ ACCOUNT_HANDLE *hAccount
    )
{
    const ULONG ulSubAuthCount = 5;
    const wchar_t wszFilterFmt[] = L"(%ws=%d AND %ws='%ws')";
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD dwError = 0;
    PDOMAIN_CONTEXT pDomCtx = NULL;
    PACCOUNT_CONTEXT pAcctCtx = NULL;
    HANDLE hDirectory = NULL;
    PWSTR pwszBaseDn = NULL;
    WCHAR wszAttrObjectClass[] = DS_ATTR_OBJECT_CLASS;
    WCHAR wszAttrObjectSid[] = DS_ATTR_OBJECT_SID;
    WCHAR wszAttrDn[] = DS_ATTR_DISTINGUISHED_NAME;
    WCHAR wszAttrSamAccountName[] = DS_ATTR_SAM_ACCOUNT_NAME;
    DWORD dwScope = 0;
    PWSTR pwszFilter = NULL;
    DWORD dwFilterLen = 0;
    PWSTR wszAttributes[4];
    PSID pDomainSid = NULL;
    PSID pAccountSid = NULL;
    ULONG ulSidLength = 0;
    PWSTR pwszAccountSid = NULL;
    PDIRECTORY_ENTRY pEntries = NULL;
    PDIRECTORY_ENTRY pEntry = NULL;
    PDIRECTORY_ATTRIBUTE pAttr = NULL;
    PATTRIBUTE_VALUE pAttrVal = NULL;
    DWORD dwEntriesNum = 0;
    DWORD i = 0;
    PWSTR pwszName = NULL;
    DWORD dwNameLen = 0;
    PWSTR pwszAccountDn = NULL;
    DWORD dwDnLen = 0;
    PSID pSid = NULL;
    DWORD dwRid = 0;

    memset(wszAttributes, 0, sizeof(wszAttributes));

    pDomCtx = (PDOMAIN_CONTEXT)hDomain;

    if (pDomCtx == NULL || pDomCtx->Type != SamrContextDomain) {
        ntStatus = STATUS_INVALID_HANDLE;
        BAIL_ON_NTSTATUS_ERROR(ntStatus);
    }

    hDirectory = pDomCtx->pConnCtx->hDirectory;
    pwszBaseDn = pDomCtx->pwszDn;
    pDomainSid = pDomCtx->pDomainSid;

    ntStatus = RTL_ALLOCATE(&pAcctCtx, ACCOUNT_CONTEXT, sizeof(*pAcctCtx));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    ulSidLength = RtlLengthRequiredSid(ulSubAuthCount);
    ntStatus = RTL_ALLOCATE(&pAccountSid, SID, ulSidLength);
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    ntStatus = RtlCopySid(ulSidLength, pAccountSid, pDomainSid);
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    ntStatus = RtlAppendRidSid(ulSidLength, pAccountSid, rid);
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    ntStatus = RtlAllocateWC16StringFromSid(&pwszAccountSid,
                                          pAccountSid);
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    dwFilterLen = ((sizeof(wszAttrObjectClass) / sizeof(WCHAR)) - 1) +
                  10 +
                  ((sizeof(wszAttrObjectSid) / sizeof(WCHAR)) - 1) +
                  wc16slen(pwszAccountSid) +
                  (sizeof(wszFilterFmt) / sizeof(wszFilterFmt[0]));

    ntStatus = SamrSrvAllocateMemory((void**)&pwszFilter,
                                   dwFilterLen * sizeof(WCHAR));
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    sw16printfw(pwszFilter, dwFilterLen, wszFilterFmt,
                wszAttrObjectClass, objectClass,
                wszAttrObjectSid, pwszAccountSid);

    wszAttributes[0] = wszAttrDn;
    wszAttributes[1] = wszAttrSamAccountName;
    wszAttributes[2] = wszAttrObjectSid;
    wszAttributes[3] = NULL;

    dwError = DirectorySearch(hDirectory,
                              pwszBaseDn,
                              dwScope,
                              pwszFilter,
                              wszAttributes,
                              FALSE,
                              &pEntries,
                              &dwEntriesNum);
    BAIL_ON_LSA_ERROR(dwError);

    if (dwEntriesNum == 1) {
        pEntry = &(pEntries[0]);

        for (i = 0; i < pEntry->ulNumAttributes; i++) {
            pAttr = &(pEntry->pAttributes[i]);

            dwError = DirectoryGetAttributeValue(pAttr,
                                                 &pAttrVal);
            BAIL_ON_LSA_ERROR(dwError);

            if (pAttrVal == NULL) {
                ntStatus = STATUS_INTERNAL_ERROR;
                BAIL_ON_NTSTATUS_ERROR(ntStatus);
            }

            if (!wc16scmp(pAttr->pwszName, wszAttrSamAccountName) &&
                pAttrVal->Type == DIRECTORY_ATTR_TYPE_UNICODE_STRING) {

                dwNameLen = wc16slen(pAttrVal->data.pwszStringValue);

                ntStatus = RTL_ALLOCATE(&pwszName, WCHAR, (dwNameLen + 1) * sizeof(WCHAR));
                BAIL_ON_NTSTATUS_ERROR(ntStatus);

                wc16sncpy(pwszName, pAttrVal->data.pwszStringValue, dwNameLen);

            } else if (!wc16scmp(pAttr->pwszName, wszAttrObjectSid) &&
                       pAttrVal->Type == DIRECTORY_ATTR_TYPE_UNICODE_STRING) {

                ntStatus = RtlAllocateSidFromWC16String(
                                            &pSid,
                                            pAttrVal->data.pwszStringValue);
                BAIL_ON_NTSTATUS_ERROR(ntStatus);

                if (!RtlEqualSid(pSid, pAccountSid)) {
                    ntStatus = STATUS_INTERNAL_ERROR;
                    BAIL_ON_NTSTATUS_ERROR(ntStatus);
                }

                dwRid = pSid->SubAuthority[pSid->SubAuthorityCount - 1];

            } else if (!wc16scmp(pAttr->pwszName, wszAttrDn) &&
                       pAttrVal->Type == DIRECTORY_ATTR_TYPE_UNICODE_STRING) {

                dwDnLen = wc16slen(pAttrVal->data.pwszStringValue);

                ntStatus = RTL_ALLOCATE(&pwszAccountDn,
                                      WCHAR,
                                      (dwDnLen + 1) * sizeof(WCHAR));
                BAIL_ON_NTSTATUS_ERROR(ntStatus);

                wc16sncpy(pwszAccountDn, pAttrVal->data.pwszStringValue, dwDnLen);
            }
        }

    } else if (dwEntriesNum == 0) {
        ntStatus = STATUS_NO_SUCH_USER;
        BAIL_ON_NTSTATUS_ERROR(ntStatus);

    } else {
        ntStatus = STATUS_INTERNAL_ERROR;
        BAIL_ON_NTSTATUS_ERROR(ntStatus);
    }

    pAcctCtx->Type          = SamrContextAccount;
    pAcctCtx->refcount      = 1;
    pAcctCtx->pwszDn        = pwszAccountDn;
    pAcctCtx->pwszName      = pwszName;
    pAcctCtx->pSid          = pSid;
    pAcctCtx->dwRid         = dwRid;
    pAcctCtx->pDomCtx       = pDomCtx;

    InterlockedIncrement(&pDomCtx->refcount);

    /* Increase ref count because DCE/RPC runtime is about to use this
       pointer as well */
    InterlockedIncrement(&pAcctCtx->refcount);

    *hAccount = (ACCOUNT_HANDLE)pAcctCtx;

cleanup:
    if (pwszFilter) {
        SamrSrvFreeMemory(pwszFilter);
    }

    if (pAccountSid) {
        RTL_FREE(&pAccountSid);
    }

    if (pwszAccountSid) {
        RTL_FREE(&pwszAccountSid);
    }

    if (pEntries) {
        DirectoryFreeEntries(pEntries, dwEntriesNum);
    }

    return ntStatus;

error:
    if (pAcctCtx) {
        InterlockedDecrement(&pAcctCtx->refcount);
        ACCOUNT_HANDLE_rundown((ACCOUNT_HANDLE)pAcctCtx);
    }

    *hAccount = NULL;
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
