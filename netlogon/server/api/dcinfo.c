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
 *        dcinfo.c
 *
 * Abstract:
 *
 *        Likewise Site Manager
 * 
 *        Domain Controller Info API (Server)
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Kyle Stemen (kstemen@likewisesoftware.com)
 *          Brian Dunstan (bdunstan@likewisesoftware.com)
 *
 */
#include "includes.h"

DWORD
LWNetSrvGetDCName(
    IN PCSTR pszServerName,
    IN PCSTR pszDnsDomainName,
    IN OPTIONAL PCSTR pszSiteName,
    IN DWORD dwDsFlags,
    IN DWORD dwBlackListCount,
    IN PSTR* ppszAddressBlackList,
    OUT PLWNET_DC_INFO* ppDcInfo
    )
//
// TODO: Remove server name param.  Technically, the server-size should not
//       take a server name.  That's for the client to use to figure out
//       where to dispatch the request (as an RPC, in that case).
//
// TODO: Add parameter for GUID-based lookups.
//
{
    DWORD dwError = 0;
    LWNET_UNIX_TIME_T now = 0;
    LWNET_UNIX_TIME_T lastPinged = 0;
    LWNET_UNIX_TIME_T lastDiscovered = 0;
    PLWNET_DC_INFO pDcInfo = NULL;
    PLWNET_DC_INFO pNewDcInfo = NULL;
    PDNS_SERVER_INFO pServerArray = NULL;
    DWORD dwServerCount = 0;
    DWORD dwIndex = 0;

    LWNET_LOG_INFO("Looking for a DC in domain '%s', site '%s' with flags %X",
            LWNET_SAFE_LOG_STRING(pszDnsDomainName),
            LWNET_SAFE_LOG_STRING(pszSiteName),
            dwDsFlags);

    if (!IsNullOrEmptyString(pszServerName))
    {
        LWNET_LOG_ERROR("Cannot specify computer name: '%s'", pszServerName);
        dwError = LWNET_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWNET_ERROR(dwError);
    }

    for (dwIndex = 0; dwIndex < dwBlackListCount; dwIndex++)
    {
        if (!ppszAddressBlackList[dwIndex])
        {
            dwError = LWNET_ERROR_INVALID_PARAMETER;
            BAIL_ON_LWNET_ERROR(dwError);
        }
    }

    // Look up in cache
    if (!(dwDsFlags & DS_FORCE_REDISCOVERY))
    {
        dwError = LWNetCacheQuery(pszDnsDomainName, pszSiteName, dwDsFlags,
                                  &pDcInfo, &lastDiscovered, &lastPinged);
        BAIL_ON_LWNET_ERROR(dwError);

        // If in background mode, we do not care about any expiration
        if (dwDsFlags & DS_BACKGROUND_ONLY)
        {
            dwError = pDcInfo ? 0 : LWNET_ERROR_DOMAIN_NOT_FOUND;
            BAIL_ON_LWNET_ERROR(dwError);
            goto error;
        }

        dwError = LWNetGetSystemTime(&now);
        BAIL_ON_LWNET_ERROR(dwError);

        // Check whether a negative cache applies
        if (!pDcInfo && (lastDiscovered > 0))
        {
            if ((now - lastDiscovered) <= LWNET_NEGATIVE_CACHE_EXPIRATION_SECONDS)
            {
                dwError = LWNET_ERROR_DOMAIN_NOT_FOUND;
                BAIL_ON_LWNET_ERROR(dwError);
            }
        }

        // ISSUE-2008/07/03-Perhaps cache on pszClientSiteName too...
        // Actually, it may be worthwhile to have that take place
        // under the hood inside the caching code itself.

        // If we found something in the cache, we may need to ping it.
        if (pDcInfo)
        {
            if ((now - lastPinged) > LWNET_PING_EXPIRATION_SECONDS)
            {
                DNS_SERVER_INFO serverInfo;

                serverInfo.pszName = pDcInfo->pszDomainControllerName;
                serverInfo.pszAddress = pDcInfo->pszDomainControllerAddress;

                while (serverInfo.pszName && serverInfo.pszName[0] == '\\')
                    serverInfo.pszName++;
                while (serverInfo.pszAddress && serverInfo.pszAddress[0] == '\\')
                    serverInfo.pszAddress++;

                dwError = LWNetSrvPingCLdapArray(pszDnsDomainName,
                                                 dwDsFlags,
                                                 &serverInfo, 1,
                                                 1, 0, &pNewDcInfo);
                if (!dwError)
                {
                    dwError = LWNetCacheUpdatePing(pszDnsDomainName,
                                                   pszSiteName,
                                                   dwDsFlags,
                                                   lastDiscovered,
                                                   pNewDcInfo);
                    BAIL_ON_LWNET_ERROR(dwError);

                    dwError = LWNetCacheUpdatePing(pszDnsDomainName,
                                                   pNewDcInfo->pszDCSiteName,
                                                   dwDsFlags,
                                                   lastDiscovered,
                                                   pNewDcInfo);
                    BAIL_ON_LWNET_ERROR(dwError);
                }
                else
                {
                    // now we need to bottom out and find us new info.
                    LWNET_SAFE_FREE_DC_INFO(pDcInfo);
                }
            }
            else
            {
                // cached data is fine
                dwError = 0;
            }
        }
    }

    for (dwIndex = 0; pDcInfo && dwIndex < dwBlackListCount; dwIndex++)
    {
        if (!strcmp(pDcInfo->pszDomainControllerAddress,
                    ppszAddressBlackList[dwIndex]))
        {
            LWNET_SAFE_FREE_DC_INFO(pDcInfo);
        }
    }

    if (!pDcInfo)
    {
        dwError = LWNetSrvGetDCNameDiscover(pszDnsDomainName,
                                            pszSiteName,
                                            dwDsFlags,
                                            dwBlackListCount,
                                            ppszAddressBlackList,
                                            &pDcInfo,
                                            &pServerArray,
                                            &dwServerCount);
        BAIL_ON_LWNET_ERROR(dwError);

        // Do not update the cache if a black list is passed in. Otherwise,
        // callers would have too much control over which DC is affinitized.
        if (dwBlackListCount == 0)
        {
            dwError = LWNetCacheUpdateDiscover(
                            pszDnsDomainName,
                            pszSiteName,
                            dwDsFlags,
                            pDcInfo);
            BAIL_ON_LWNET_ERROR(dwError);

            dwError = LWNetCacheUpdateDiscover(
                            pszDnsDomainName,
                            pDcInfo->pszDCSiteName,
                            dwDsFlags,
                            pDcInfo);
            BAIL_ON_LWNET_ERROR(dwError);

            // Will only affinitize for KDC/LDAP (i.e., not PDC/GC) and if the site is the same.
            if (!(dwDsFlags & (DS_PDC_REQUIRED | DS_GC_SERVER_REQUIRED)) &&
                (IsNullOrEmptyString(pszSiteName) ||
                 !strcasecmp(pDcInfo->pszDCSiteName, pszSiteName)))
            {
                dwError = LWNetKrb5UpdateAffinity(
                                pszDnsDomainName,
                                pDcInfo,
                                pServerArray,
                                dwServerCount);
                BAIL_ON_LWNET_ERROR(dwError);
            }
        }
    }

error:
    LWNET_SAFE_FREE_MEMORY(pServerArray);
    LWNET_SAFE_FREE_DC_INFO(pNewDcInfo);
    if (dwError)
    {
        LWNET_SAFE_FREE_DC_INFO(pDcInfo);
    }
    *ppDcInfo = pDcInfo;
    return dwError;
}


DWORD
LWNetSrvGetDomainController(
    IN PCSTR pszDomainFQDN,
    OUT PSTR* ppszDomainControllerFQDN
    )
{
    DWORD dwError = 0;
    PSTR pszDomainControllerFQDN = NULL;
    PLWNET_DC_INFO pDcInfo = NULL;
    
    dwError = LWNetSrvGetDCName(NULL,
                                pszDomainFQDN,
                                NULL,
                                DS_DIRECTORY_SERVICE_REQUIRED,
                                0,
                                NULL,
                                &pDcInfo);
    BAIL_ON_LWNET_ERROR(dwError);

    dwError = LWNetAllocateString(pDcInfo->pszDomainControllerName,
                                  &pszDomainControllerFQDN);
    BAIL_ON_LWNET_ERROR(dwError);
    
error:    
    LWNET_SAFE_FREE_DC_INFO(pDcInfo);
    if (dwError)
    {
        LWNET_SAFE_FREE_STRING(pszDomainControllerFQDN);
    }
    *ppszDomainControllerFQDN = pszDomainControllerFQDN;
    return dwError;
}

DWORD
LWNetSrvGetDCTime(
    IN PCSTR pszDomainFQDN,
    OUT PLWNET_UNIX_TIME_T pDCTime
    )
{
    DWORD dwError   = 0;
    PSTR  pszDC     = NULL;
    PSTR  pszDCTime = NULL;
    struct tm dcTime = {0};
    time_t ttDcTimeUTC = 0;
    LDAPMessage* pMessage = NULL;
    HANDLE hDirectory = (HANDLE)NULL;
    PSTR  ppszAttributeList[] = 
        {
             "currentTime",
             NULL
        };
    LWNET_UNIX_TIME_T result = 0;
#ifndef HAVE_TIMEGM
    struct tm epochTime = {0};
#endif

    BAIL_ON_INVALID_POINTER(pDCTime);

    LWNET_LOG_INFO("Determining the current time for domain '%s'",
            LWNET_SAFE_LOG_STRING(pszDomainFQDN));

    dwError = LWNetSrvGetDomainController(pszDomainFQDN, &pszDC);
    BAIL_ON_LWNET_ERROR(dwError);
    
    dwError = LWNetCLdapOpenDirectory(pszDC, &hDirectory);
    BAIL_ON_LWNET_ERROR(dwError);
    
    dwError = LWNetLdapBindDirectoryAnonymous(hDirectory);
    BAIL_ON_LWNET_ERROR(dwError);
    
    dwError = LWNetLdapDirectorySearchEx(hDirectory, "", LDAP_SCOPE_BASE,
                                         "(objectclass=*)", ppszAttributeList,
                                         0, &pMessage);
    BAIL_ON_LWNET_ERROR(dwError);
    
    dwError = LWNetLdapGetString(hDirectory, pMessage, "currentTime",
                                 &pszDCTime);
    BAIL_ON_LWNET_ERROR(dwError);
    
    dwError = LWNetCrackLdapTime(pszDCTime, &dcTime);
    if (dwError)
    {
        if (dwError == EINVAL)
        {
            dwError = LWNET_ERROR_FAILED_TIME_CONVERSION;
        }
        BAIL_ON_LWNET_ERROR(dwError);
    }
     
#ifdef HAVE_TIMEGM
    ttDcTimeUTC = timegm(&dcTime);
#else
    epochTime.tm_mday = 2;
    epochTime.tm_mon = 0;
    epochTime.tm_year = 70;
    /* AIX does not honor value 0 in tm_isdst (which should mean that daylight
     * savings is not in effect). Instead AIX treats 0 like value -1 (check
     * whether daylight savings is in effect based on time of year).
     *
     * AIX does however honor value 1 (which means that daylight savings is in
     * effect). By setting tm_isdst to 1 in both epochTime and dcTime, they
     * will cancel each other out.
     */
    epochTime.tm_isdst = 1;
    dcTime.tm_isdst = 1;

    /* 00:00 Jan 1, 1970 should be 0(time_t). Converting the epoch time
     * and subtracting it adjusts for the local time zone.
     *
     * mktime(&epochTime) should be the same as the timezone global variable,
     * but for some reason that variable is set to 0 on AIX, even after calling
     * tzset.
     *
     * Note that Jan 2 is used instead of Jan 1 because a Jan 1 1970
     * local time is not representable as an epoch time for timezones
     * ahead of GMT.  Then the value is corrected by adding back
     * a day (24 hours).
     */
    ttDcTimeUTC = mktime(&dcTime) - mktime(&epochTime) + (24 * 60 * 60);
#endif

    result = ttDcTimeUTC;

error:
    LWNET_SAFE_FREE_STRING(pszDC);
    LWNET_SAFE_FREE_STRING(pszDCTime);

    if (hDirectory)
    {
        LWNetLdapCloseDirectory(hDirectory);
    }

    if (pMessage)
    {
        ldap_msgfree(pMessage);
    }

    if (dwError)
    {
        memset(&result, 0, sizeof(result));
    }

    *pDCTime = result;

    return dwError;
}
