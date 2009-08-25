/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 * Copyright Likewise Software    2004-2008
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
 * Abstract: Lsa rpc server functions (rpc server library)
 *
 * Authors: Rafal Szczesniak (rafal@likewise.com)
 */

#ifndef _LSASRV_H_
#define _LSASRV_H_


NTSTATUS
LsaSrvClose(
    handle_t b,
    POLICY_HANDLE hIn,
    POLICY_HANDLE *hOut
    );


NTSTATUS
LsaSrvOpenPolicy2(
    handle_t b,
    wchar16_t *system_name,
    ObjectAttribute *attrib,
    uint32 access_mask,
    POLICY_HANDLE *phPolicy
    );


NTSTATUS
LsaSrvLookupNames(
    handle_t b,
    POLICY_HANDLE hPolicy,
    uint32 num_names,
    UnicodeString *names,
    RefDomainList **domains,
    TranslatedSidArray *sids,
    uint16 level,
    uint32 *count
    );


NTSTATUS
LsaSrvLookupSids(
    handle_t b,
    POLICY_HANDLE hPolicy,
    SidArray *sids,
    RefDomainList **domains,
    TranslatedNameArray *names,
    uint16 level,
    uint32 *count
    );


NTSTATUS
LsaSrvLookupNames2(
    handle_t b,
    POLICY_HANDLE hPolicy,
    uint32 num_names,
    UnicodeStringEx *names,
    RefDomainList **domains,
    TranslatedSidArray2 *sids,
    uint16 level,
    uint32 *count,
    uint32 unknown1,
    uint32 unknown2
    );


NTSTATUS
LsaSrvLookupSids2(
    handle_t hBinding,
    POLICY_HANDLE hPolicy,
    SidArray *sids,
    RefDomainList **domains,
    TranslatedNameArray2 *names,
    uint16 level,
    uint32 *count,
    uint32 unknown1,
    uint32 unknown2
    );


NTSTATUS
LsaSrvQueryInfoPolicy(
    handle_t b,
    POLICY_HANDLE hPolicy,
    uint16 level,
    LsaPolicyInformation **info
    );


NTSTATUS
LsaSrvQueryInfoPolicy2(
    handle_t b,
    POLICY_HANDLE hPolicy,
    uint16 level,
    LsaPolicyInformation **info
    );


NTSTATUS
LsaSrvLookupNames3(
    handle_t b,
    POLICY_HANDLE hPolicy,
    uint32 num_names,
    UnicodeStringEx *names,
    RefDomainList **domains,
    TranslatedSidArray3 *sids,
    uint16 level,
    uint32 *count,
    uint32 unknown1,
    uint32 unknown2
    );


NTSTATUS
LsaSrvCreateSamDomainsTable(
    PLSA_HASH_TABLE *ppDomains
    );


NTSTATUS
LsaSrvGetSamDomainByName(
    PPOLICY_CONTEXT pPolCtx,
    PCWSTR pwszDomainName,
    PSAM_DOMAIN_ENTRY *ppDomain
    );


NTSTATUS
LsaSrvGetSamDomainBySid(
    PPOLICY_CONTEXT pPolCtx,
    const PSID pSid,
    PSAM_DOMAIN_ENTRY *ppDomain
    );


NTSTATUS
LsaSrvSetSamDomain(
    PPOLICY_CONTEXT pPolCtx,
    const PSAM_DOMAIN_ENTRY pDomain
    );


NTSTATUS
LsaSrvGetLocalSamDomain(
    PPOLICY_CONTEXT pPolCtx,
    BOOLEAN bBuiltin,
    PSAM_DOMAIN_ENTRY *ppDomain
    );


VOID
LsaSrvSamDomainEntryFree(
    PSAM_DOMAIN_ENTRY *ppEntry
    );


NTSTATUS
LsaSrvParseAccountName(
    PWSTR pwszName,
    PWSTR *ppwszDomainName,
    PWSTR *ppwszAcctName
    );


NTSTATUS
LsaSrvSelectAccountsByDomainName(
    PPOLICY_CONTEXT pPolCtx,
    UnicodeStringEx *pNames,
    DWORD dwNumNames,
    PACCOUNT_NAMES pDomainAccounts,
    PACCOUNT_NAMES pLocalAccounts,
    PACCOUNT_NAMES pBuiltinAccounts,
    PACCOUNT_NAMES pOtherAccounts
    );


NTSTATUS
LsaSrvSelectAccountsByDomainSid(
    PPOLICY_CONTEXT pPolCtx,
    SidArray *pSids,
    DWORD dwNumSids,
    PACCOUNT_SIDS pDomainAccounts,
    PACCOUNT_SIDS pLocalAccounts,
    PACCOUNT_SIDS pBuiltinAccounts
    );


VOID
LsaSrvFreeAccountNames(
    PACCOUNT_NAMES pAccountNames
    );


VOID
LsaSrvFreeAccountSids(
    PACCOUNT_SIDS pAccountSids
    );


NTSTATUS
LsaSrvGetSystemAccessToken(
    LW_PIO_ACCESS_TOKEN *ppAccessToken
    );


#endif /* _LSASRV_H_ */
