/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
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
 *        lsaprovider.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Authentication Provider Interface
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */

#include "adprovider.h"

DWORD
ActiveDirectoryOpenProvider(
    uid_t peerUID,
    gid_t peerGID,
    PHANDLE phProvider
    )
{
    DWORD dwError = 0;

    return dwError;
}

VOID
ActiveDirectoryCloseProvider(
    HANDLE hProvider
    )
{

    return;
}

BOOLEAN
ActiveDirectoryServicesDomain(
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryAuthenticateUser(
    HANDLE hProvider,
    PCSTR  pszLoginId,
    PCSTR  pszPassword
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryAuthenticateUserEx(
    HANDLE hProvider,
    PLSA_AUTH_USER_PARAMS pUserParams,
    PLSA_AUTH_USER_INFO *ppUserInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryValidateUser(
    HANDLE hProvider,
    PCSTR  pszLoginId,
    PCSTR  pszPassword
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryCheckUserInList(
    HANDLE hProvider,
    PCSTR  pszLoginId,
    PCSTR  pszListName
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryFindUserByName(
    HANDLE  hProvider,
    PCSTR   pszLoginId,
    DWORD   dwUserInfoLevel,
    PVOID*  ppUserInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryFindUserById(
    HANDLE  hProvider,
    uid_t   uid,
    DWORD   dwUserInfoLevel,
    PVOID*  ppUserInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryFindGroupByName(
    HANDLE  hProvider,
    PCSTR   pszGroupName,
    LSA_FIND_FLAGS FindFlags,
    DWORD   dwGroupInfoLevel,
    PVOID*  ppGroupInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryFindGroupById(
    HANDLE  hProvider,
    gid_t   gid,
    LSA_FIND_FLAGS FindFlags,
    DWORD   dwGroupInfoLevel,
    PVOID*  ppGroupInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryGetGroupsForUser(
    HANDLE  hProvider,
    uid_t   uid,
    LSA_FIND_FLAGS FindFlags,
    DWORD   dwGroupInfoLevel,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    )
{
    DWORD dwError = 0;

#if 0
    dwError = MiniProviderGetGroupsForUser(
                    hMiniProvider,
                    uid,
                    FindFlags,
                    dwGroupInfoLevel,
                    pdwGroupsFound,
                    pppGroupInfoList
                    );
#endif

    return dwError;
}

DWORD
ActiveDirectoryBeginEnumUsers(
    HANDLE  hProvider,
    DWORD   dwInfoLevel,
    LSA_FIND_FLAGS FindFlags,
    PHANDLE phResume
    )
{
    DWORD dwError = 0;

#if 0
    dwError = MiniProviderBeginEnumUsers(
                    hProvider,
                    dwConnectMode,
                    dwInfoLevel,
                    FindFlags,
                    phResume
                    );
#endif

    return (dwError);
}

DWORD
ActiveDirectoryEnumUsers(
    HANDLE  hProvider,
    HANDLE  hResume,
    DWORD   dwMaxUsers,
    PDWORD  pdwUsersFound,
    PVOID** pppUserInfoList
    )
{
    DWORD dwError = 0;

#if 0
    dwError = MiniProviderEnumUsers(
                    hProvider,
                    dwConnectMode,
                    dwInfoLevel,
                    FindFlags,
                    phResume
                    );
#endif

    return dwError;

}

VOID
ActiveDirectoryEndEnumUsers(
    HANDLE hProvider,
    HANDLE hResume
    )
{
#if 0
    DWORD dwError = 0;

    dwError = MiniProviderEndEnumUsers(
                    hMiniProvider,
                    dwConnectMode,
                    hResume
                    );
#endif

    return;
}

DWORD
ActiveDirectoryBeginEnumGroups(
    HANDLE  hProvider,
    DWORD   dwInfoLevel,
    BOOLEAN bCheckGroupMembersOnline,
    LSA_FIND_FLAGS FindFlags,
    PHANDLE phResume
    )
{
    DWORD dwError = 0;

#if 0
    dwError = MiniProviderBeginEnumGroups(
                    hMiniProvider,
                    dwConnectMode,
                    dwInfoLevel,
                    bCheckGroupMembersOnline,
                    FindFlags,
                    phResume
                    );
#endif

    return dwError;
}

DWORD
ActiveDirectoryEnumGroups(
    HANDLE  hProvider,
    HANDLE  hResume,
    DWORD   dwMaxGroups,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    )
{
    DWORD dwError = 0;

#if 0
    dwError = MiniProviderEnumGroups(
                    hMiniProvider,
                    dwConnectState,
                    hResume,
                    dwMaxGroups,
                    pdwGroupsFound,
                    pppGroupInfoList
                    );
#endif

    return (dwError);
}

VOID
ActiveDirectoryEndEnumGroups(
    HANDLE hProvider,
    HANDLE hResume
    )
{
#if 0
    DWORD dwError = 0;

    dwError = MiniProviderEndEnumGroups(
                    hMiniProvider,
                    dwConnectState,
                    hResume
                    );
#endif

    return;
}

DWORD
ActiveDirectoryChangePassword(
    HANDLE hProvider,
    PCSTR  pszLoginId,
    PCSTR  pszPassword,
    PCSTR  pszOldPassword
    )
{
    DWORD dwError = LSA_ERROR_NOT_SUPPORTED;

    return dwError;
}

DWORD
ActiveDirectoryAddUser(
    HANDLE hProvider,
    DWORD  dwUserInfoLevel,
    PVOID  pUserInfo
    )
{
    DWORD dwError = LSA_ERROR_NOT_SUPPORTED;

    return dwError;
}


DWORD
ActiveDirectoryModifyUser(
    HANDLE hProvider,
    PLSA_USER_MOD_INFO pUserModInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryDeleteUser(
    HANDLE hProvider,
    uid_t  uid
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryAddGroup(
    HANDLE hProvider,
    DWORD  dwGroupInfoLevel,
    PVOID  pGroupInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryDeleteGroup(
    HANDLE hProvider,
    gid_t  gid
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryOpenSession(
    HANDLE hProvider,
    PCSTR  pszLoginId
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryCloseSession(
    HANDLE hProvider,
    PCSTR  pszLoginId
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryGetNamesBySidList(
    HANDLE hProvider,
    size_t sCount,
    PSTR*  ppszSidList,
    PSTR**          pppszDomainNames,
    PSTR**          pppszSamAccounts,
    ADAccountType** ppTypes
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryFindNSSArtefactByKey(
    HANDLE hProvider,
    PCSTR  pszKeyName,
    PCSTR  pszMapName,
    DWORD  dwInfoLevel,
    LSA_NIS_MAP_QUERY_FLAGS dwFlags,
    PVOID* ppNSSArtefactInfo
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryBeginEnumNSSArtefacts(
    HANDLE  hProvider,
    DWORD   dwInfoLevel,
    PCSTR   pszMapName,
    LSA_NIS_MAP_QUERY_FLAGS dwFlags,
    PHANDLE phResume
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryEnumNSSArtefacts(
    HANDLE  hProvider,
    HANDLE  hResume,
    DWORD   dwMaxNSSArtefacts,
    PDWORD  pdwNSSArtefactsFound,
    PVOID** pppNSSArtefactInfoList
    )
{
    DWORD dwError = 0;

    return dwError;
}

VOID
ActiveDirectoryEndEnumNSSArtefacts(
    HANDLE hProvider,
    HANDLE hResume
    )
{

    return;
}

DWORD
ActiveDirectoryGetStatus(
    HANDLE hProvider,
    PLSA_AUTH_PROVIDER_STATUS* ppAuthProviderStatus
    )
{
    DWORD dwError = 0;

    return dwError;
}

VOID
ActiveDirectoryFreeStatus(
    PLSA_AUTH_PROVIDER_STATUS pAuthProviderStatus
    )
{

    return;
}

DWORD
ActiveDirectoryRefreshConfiguration(
    HANDLE hProvider
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
ActiveDirectoryProviderIoControl(
    HANDLE hProvider,
    uid_t  peerUid,
    gid_t  peerGID,
    DWORD  dwIoControlCode,
    DWORD  dwInputBufferSize,
    PVOID  pInputBuffer,
    DWORD* pdwOutputBufferSize,
    PVOID* ppOutputBuffer
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
LsaInitializeProvider(
    PCSTR pszConfigFilePath,
    PSTR* ppszProviderName,
    PLSA_PROVIDER_FUNCTION_TABLE* ppFnTable
    )
{
    DWORD dwError = 0;

    return dwError;
}

DWORD
LsaShutdownProvider(
    PSTR pszProviderName,
    PLSA_PROVIDER_FUNCTION_TABLE pFnTable
    )
{
    DWORD dwError = 0;

    return dwError;
}

