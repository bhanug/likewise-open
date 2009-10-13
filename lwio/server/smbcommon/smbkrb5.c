/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software
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

#include "includes.h"

static
DWORD
SMBGetServerCanonicalName(
    PCSTR pszServerName,
    PSTR* ppszNormal
    );

static
DWORD
SMBGetServerDomain(
    PCSTR pszServerName,
    PSTR* ppszDomain
    );

static
void
smb_display_status(
    PCSTR     pszId,
    OM_uint32 maj_stat,
    OM_uint32 min_stat
    );

static
void
smb_display_status_1(
    PCSTR     pszId,
    OM_uint32 code,
    int       type
    );

DWORD
SMBKrb5SetDefaultCachePath(
    PCSTR pszCachePath,
    PSTR* ppszOrigCachePath
    )
{
    DWORD dwError       = 0;
    DWORD dwMajorStatus = 0;
    DWORD dwMinorStatus = 0;
    PSTR  pszOrigCachePath = NULL;

    // Set the default for gss
    dwMajorStatus = gss_krb5_ccache_name(
                            (OM_uint32 *)&dwMinorStatus,
                            pszCachePath,
                            (ppszOrigCachePath) ? (const char**)&pszOrigCachePath : NULL);
    BAIL_ON_SEC_ERROR(dwMajorStatus);

    if (ppszOrigCachePath) {
        if (!IsNullOrEmptyString(pszOrigCachePath)) {
            dwError = SMBAllocateString(pszOrigCachePath, ppszOrigCachePath);
            BAIL_ON_LWIO_ERROR(dwError);
        } else {
            *ppszOrigCachePath = NULL;
        }
    }

    LWIO_LOG_DEBUG("Cache path set to [%s]", SMB_SAFE_LOG_STRING(pszCachePath));

cleanup:

    return dwError;

sec_error:
error:

    if (ppszOrigCachePath) {
        *ppszOrigCachePath = NULL;
    }

    goto cleanup;
}

DWORD
SMBKrb5DestroyCache(
    PCSTR pszCachePath
    )
{
    DWORD dwError = 0;
    krb5_error_code ret = 0;
    krb5_context ctx = NULL;
    krb5_ccache cc = NULL;

    ret = krb5_init_context(&ctx);
    BAIL_ON_LWIO_KRB_ERROR(ctx, ret);

    /* use krb5_cc_resolve to get an alternate cache */
    ret = krb5_cc_resolve(ctx, pszCachePath, &cc);
    BAIL_ON_LWIO_KRB_ERROR(ctx, ret);

    ret = krb5_cc_destroy(ctx, cc);
    if (ret != 0) {
        if (ret != KRB5_FCC_NOFILE) {
            BAIL_ON_LWIO_KRB_ERROR(ctx, ret);
        } else {
            ret = 0;
        }
    }

error:

    if (ctx)
    {
       krb5_free_context(ctx);
    }

    return(dwError);
}

DWORD
SMBGSSContextBuild(
    PCSTR     pszServerName,
    PHANDLE   phSMBGSSContext
    )
{
    DWORD dwError = 0;
    DWORD dwMajorStatus = 0;
    DWORD dwMinorStatus = 0;
    PSMB_GSS_SEC_CONTEXT pContext = NULL;
    PSTR pszDomainName = NULL;

    gss_buffer_desc input_name = {0};

    LWIO_LOG_DEBUG("Build GSS Context for server [%s]", SMB_SAFE_LOG_STRING(pszServerName));

    dwError = SMBGetServerDomain(pszServerName, &pszDomainName);
    BAIL_ON_LWIO_ERROR(dwError);

    SMBStrToUpper(pszDomainName);

    dwError = SMBAllocateMemory(
                    sizeof(SMB_GSS_SEC_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_LWIO_ERROR(dwError);

    dwError = SMBAllocateStringPrintf(
                    &pContext->pszTargetName,
                    "cifs/%s@%s",
                    pszServerName,
                    pszDomainName);
    BAIL_ON_LWIO_ERROR(dwError);

    pContext->state = SMB_GSS_SEC_CONTEXT_STATE_INITIAL;

    input_name.value = pContext->pszTargetName;
    input_name.length = strlen(pContext->pszTargetName) + 1;

    dwMajorStatus = gss_import_name(
                        (OM_uint32 *)&dwMinorStatus,
                        &input_name,
                        (gss_OID) gss_nt_krb5_name,
                        &pContext->target_name);

    smb_display_status("gss_import_name", dwMajorStatus, dwMinorStatus);

    BAIL_ON_SEC_ERROR(dwMajorStatus);

    dwError = SMBAllocateMemory(
                    sizeof(CtxtHandle),
                    (PVOID*)&pContext->pGSSContext);
    BAIL_ON_LWIO_ERROR(dwError);

    *pContext->pGSSContext = GSS_C_NO_CONTEXT;

    *phSMBGSSContext = (HANDLE)pContext;

cleanup:

    LWIO_SAFE_FREE_STRING(pszDomainName);

    return dwError;

sec_error:

    dwError = LWIO_ERROR_GSS;

error:

    *phSMBGSSContext = NULL;

    if (pContext)
    {
        SMBGSSContextFree(pContext);
    }

    goto cleanup;
}

BOOLEAN
SMBGSSContextNegotiateComplete(
    HANDLE hSMBGSSContext
    )
{
    PSMB_GSS_SEC_CONTEXT pContext = (PSMB_GSS_SEC_CONTEXT)hSMBGSSContext;

    return (pContext->state == SMB_GSS_SEC_CONTEXT_STATE_COMPLETE);
}

DWORD
SMBGSSContextNegotiate(
    HANDLE hSMBGSSContext,
    PBYTE  pSecurityInputBlob,
    DWORD  dwSecurityInputBlobLen,
    PBYTE* ppSecurityBlob,
    PDWORD pdwSecurityBlobLength
    )
{
    DWORD dwError = 0;
    DWORD dwMajorStatus = 0;
    DWORD dwMinorStatus = 0;
    PSMB_GSS_SEC_CONTEXT pContext = (PSMB_GSS_SEC_CONTEXT)hSMBGSSContext;
    gss_buffer_desc input_desc = {0};
    gss_buffer_desc output_desc = {0};
    DWORD ret_flags = 0;
    PBYTE pSecurityBlob = NULL;
    DWORD dwSecurityBlobLength = 0;

    static gss_OID_desc gss_spnego_mech_oid_desc =
      {6, (void *)"\x2b\x06\x01\x05\x05\x02"};

    LWIO_LOG_DEBUG("Negotiate GSS Context for target [%s]", pContext->pszTargetName);

    if (pContext->state == SMB_GSS_SEC_CONTEXT_STATE_COMPLETE)
    {
        goto cleanup;
    }

    input_desc.value = pSecurityInputBlob;
    input_desc.length = dwSecurityInputBlobLen;

    dwMajorStatus = gss_init_sec_context(
                        (OM_uint32 *)&dwMinorStatus,
                        NULL,
                        pContext->pGSSContext,
                        pContext->target_name,
                        &gss_spnego_mech_oid_desc,
                        GSS_C_REPLAY_FLAG | GSS_C_MUTUAL_FLAG |
                        GSS_C_SEQUENCE_FLAG | GSS_C_CONF_FLAG |
                        GSS_C_INTEG_FLAG,
                        0,
                        NULL,
                        &input_desc,
                        NULL,
                        &output_desc,
                        &ret_flags,
                        NULL);

    smb_display_status("gss_init_sec_context", dwMajorStatus, dwMinorStatus);

    switch (dwMajorStatus)
    {
        case GSS_S_CONTINUE_NEEDED:

            pContext->state = SMB_GSS_SEC_CONTEXT_STATE_NEGOTIATE;

            break;

        case GSS_S_COMPLETE:

            pContext->state = SMB_GSS_SEC_CONTEXT_STATE_COMPLETE;

            break;

        case GSS_S_FAILURE:
            if (dwMinorStatus == (DWORD) KRB5KRB_AP_ERR_SKEW)
            {
                dwError = LWIO_ERROR_CLOCK_SKEW;
            }
            else
            {
                dwError = LWIO_ERROR_GSS;
            }
            BAIL_ON_LWIO_ERROR(dwError);
            break;

        default:

            dwError = LWIO_ERROR_GSS;
            BAIL_ON_LWIO_ERROR(dwError);

            break;
    }

    if (output_desc.length)
    {
        dwError = SMBAllocateMemory(
                        output_desc.length,
                        (PVOID*)&pSecurityBlob);
        BAIL_ON_LWIO_ERROR(dwError);

        memcpy(pSecurityBlob, output_desc.value, output_desc.length);

        dwSecurityBlobLength = output_desc.length;
    }

    *ppSecurityBlob = pSecurityBlob;
    *pdwSecurityBlobLength = dwSecurityBlobLength;

cleanup:

    gss_release_buffer(&dwMinorStatus, &output_desc);

    return dwError;

error:

    *ppSecurityBlob = NULL;
    *pdwSecurityBlobLength = 0;

    LWIO_SAFE_FREE_MEMORY(pSecurityBlob);

    goto cleanup;
}

static
NTSTATUS
SMBGssGetSessionKey(
    gss_ctx_id_t Context,
    PBYTE* ppSessionKey,
    PDWORD pdwSessionKeyLength
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PBYTE pSessionKey = NULL;
    DWORD dwSessionKeyLength = 0;
    OM_uint32 gssMajor = GSS_S_COMPLETE;
    OM_uint32 gssMinor = 0;
    gss_buffer_set_t sessionKey = NULL;

    gssMajor = gss_inquire_sec_context_by_oid(
                    &gssMinor,
                    Context,
                    GSS_C_INQ_SSPI_SESSION_KEY,
                    &sessionKey);
    if (gssMajor != GSS_S_COMPLETE)
    {
        smb_display_status("gss_inquire_sec_context_by_oid", gssMajor, gssMinor);
        // TODO - error code conversion
        status = gssMajor;
        BAIL_ON_LWIO_ERROR(status);
    }

    // The key is in element 0 and the key type OID is in element 1
    if (!sessionKey ||
        (sessionKey->count < 1) ||
        !sessionKey->elements[0].value ||
        (0 == sessionKey->elements[0].length))
    {
        LWIO_ASSERT_MSG(FALSE, "Invalid session key");
        status = STATUS_ASSERTION_FAILURE;
        BAIL_ON_LWIO_ERROR(status);
    }

    status = LW_RTL_ALLOCATE(&pSessionKey, BYTE, sessionKey->elements[0].length);
    BAIL_ON_LWIO_ERROR(status);

    memcpy(pSessionKey, sessionKey->elements[0].value, sessionKey->elements[0].length);
    dwSessionKeyLength = sessionKey->elements[0].length;

cleanup:
    gss_release_buffer_set(&gssMinor, &sessionKey);

    *ppSessionKey = pSessionKey;
    *pdwSessionKeyLength = dwSessionKeyLength;

    return status;

error:
    LWIO_SAFE_FREE_MEMORY(pSessionKey);
    dwSessionKeyLength = 0;

    goto cleanup;
}

NTSTATUS
SMBGSSContextGetSessionKey(
    HANDLE hSMBGSSContext,
    PBYTE* ppSessionKey,
    PDWORD pdwSessionKeyLength
    )
{
    PSMB_GSS_SEC_CONTEXT pContext = (PSMB_GSS_SEC_CONTEXT)hSMBGSSContext;
    return SMBGssGetSessionKey(
                *pContext->pGSSContext,
                ppSessionKey,
                pdwSessionKeyLength);
}

VOID
SMBGSSContextFree(
    HANDLE hSMBGSSContext
    )
{
    DWORD dwMinorStatus = 0;
    PSMB_GSS_SEC_CONTEXT pContext = (PSMB_GSS_SEC_CONTEXT)hSMBGSSContext;

    if (pContext)
    {
        gss_release_name(&dwMinorStatus, &pContext->target_name);

        if (pContext->pGSSContext &&
            (*pContext->pGSSContext != GSS_C_NO_CONTEXT))
        {
            gss_delete_sec_context(
                            &dwMinorStatus,
                            pContext->pGSSContext,
                            GSS_C_NO_BUFFER);

            SMBFreeMemory(pContext->pGSSContext);
        }

        LWIO_SAFE_FREE_STRING(pContext->pszTargetName);

        SMBFreeMemory(pContext);
    }
}

static
DWORD
SMBGetServerCanonicalName(
    PCSTR pszServerName,
    PSTR* ppszNormal
    )
{
    DWORD dwError = 0;
    struct addrinfo hints;
    struct addrinfo* pAddrInfo = NULL;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_CANONNAME;

    if (getaddrinfo(pszServerName, NULL, &hints, &pAddrInfo))
    {
        dwError = LWIO_ERROR_HOST_NOT_FOUND;
        BAIL_ON_LWIO_ERROR(dwError);
    }

    dwError = SMBAllocateString(pAddrInfo->ai_canonname, ppszNormal);
    BAIL_ON_LWIO_ERROR(dwError);

error:

    if (pAddrInfo)
    {
        freeaddrinfo(pAddrInfo);
    }

    return dwError;
}

static
DWORD
SMBGetServerDomain(
    PCSTR pszServerName,
    PSTR* ppszDomain
    )
{
    DWORD dwError = 0;
    char* pDot = NULL;
    PSTR pszDomain = NULL;
    PSTR pszNormal = NULL;

    pDot = strchr(pszServerName, '.');

    if (pDot)
    {
        dwError = SMBAllocateString(pszServerName, &pszNormal);
        BAIL_ON_LWIO_ERROR(dwError);
    }
    else
    {
        dwError = SMBGetServerCanonicalName(pszServerName, &pszNormal);
        BAIL_ON_LWIO_ERROR(dwError);
    }

    dwError = SMBAllocateString(strchr(pszNormal, '.') + 1, &pszDomain);
    BAIL_ON_LWIO_ERROR(dwError);

    *ppszDomain = pszDomain;

cleanup:

    LWIO_SAFE_FREE_MEMORY(pszNormal);

    return dwError;

error:

    *ppszDomain = NULL;
    LWIO_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}

NTSTATUS
SMBCredTokenToKrb5CredCache(
    PIO_CREDS pCredToken,
    PSTR* ppszCachePath
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    krb5_context pContext = NULL;
    krb5_error_code krb5Error = 0;
    krb5_ccache pCache = NULL;
    PSTR pszClientPrincipalName = NULL;
    PSTR pszServerPrincipalName = NULL;
    PSTR pszCachePath = NULL;
    krb5_creds creds;

    memset(&creds, 0, sizeof(creds));

     /* Set up an in-memory cache to receive the credentials */
    Status = SMBAllocateStringPrintf(
        &pszCachePath,
        "MEMORY:%lu",
        (unsigned long) (size_t) pCredToken);
    BAIL_ON_NT_STATUS(Status);

    krb5Error = krb5_init_context(&pContext);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    krb5Error = krb5_cc_resolve(pContext, pszCachePath, &pCache);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    /* Convert cred token back into krb5 structure */

    /* Convert client and server principal names */
    Status = LwRtlCStringAllocateFromWC16String(
        &pszClientPrincipalName,
        pCredToken->payload.krb5Tgt.pwszClientPrincipal);
    BAIL_ON_NT_STATUS(Status);

    krb5Error = krb5_parse_name(pContext, pszClientPrincipalName, &creds.client);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    Status = LwRtlCStringAllocateFromWC16String(
        &pszServerPrincipalName,
        pCredToken->payload.krb5Tgt.pwszServerPrincipal);
    BAIL_ON_NT_STATUS(Status);

    krb5Error = krb5_parse_name(pContext, pszServerPrincipalName, &creds.server);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    /* Convert times */
    creds.times.authtime = pCredToken->payload.krb5Tgt.authTime;
    creds.times.starttime = pCredToken->payload.krb5Tgt.startTime;
    creds.times.endtime = pCredToken->payload.krb5Tgt.endTime;
    creds.times.renew_till = pCredToken->payload.krb5Tgt.renewTillTime;

    /* Convert encryption key */
    creds.keyblock.enctype = pCredToken->payload.krb5Tgt.keyType;
    creds.keyblock.length = (unsigned int) pCredToken->payload.krb5Tgt.ulKeySize;
    creds.keyblock.contents = pCredToken->payload.krb5Tgt.pKeyData;

    /* Convert tgt */
    creds.ticket_flags = pCredToken->payload.krb5Tgt.tgtFlags;
    creds.ticket.length = pCredToken->payload.krb5Tgt.ulTgtSize;
    creds.ticket.data = (char*) pCredToken->payload.krb5Tgt.pTgtData;

    /* Initialize the credential cache with the client principal name */
    krb5Error = krb5_cc_initialize(pContext, pCache, creds.client);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    /* Store the converted credentials in the cache */
    krb5Error = krb5_cc_store_cred(pContext, pCache, &creds);
    if (krb5Error)
    {
        Status = STATUS_UNSUCCESSFUL;
        BAIL_ON_NT_STATUS(Status);
    }

    *ppszCachePath = pszCachePath;

cleanup:

    LWIO_SAFE_FREE_MEMORY(pszClientPrincipalName);
    LWIO_SAFE_FREE_MEMORY(pszServerPrincipalName);

    if (creds.client)
    {
        krb5_free_principal(pContext, creds.client);
    }

    if (creds.server)
    {
        krb5_free_principal(pContext, creds.server);
    }

    if (pCache)
    {
        krb5_cc_close(pContext, pCache);
    }

    if (pContext)
    {
        krb5_free_context(pContext);
    }

    return Status;

error:

    *ppszCachePath = NULL;

    LWIO_SAFE_FREE_MEMORY(pszCachePath);

    goto cleanup;
}

static
void
smb_display_status(
    PCSTR     pszId,
    OM_uint32 maj_stat,
    OM_uint32 min_stat
    )
{
     smb_display_status_1(pszId, maj_stat, GSS_C_GSS_CODE);
     smb_display_status_1(pszId, min_stat, GSS_C_MECH_CODE);
}

static
void
smb_display_status_1(
    PCSTR     pszId,
    OM_uint32 code,
    int       type
    )
{
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc msg;
    OM_uint32 msg_ctx;

    if ( code == 0 )
    {
        return;
    }

    msg_ctx = 0;
    while (1)
    {
        maj_stat = gss_display_status(&min_stat, code,
                                      type, GSS_C_NULL_OID,
                                      &msg_ctx, &msg);

        switch(code)
        {
#ifdef WIN32
            case SEC_E_OK:
            case SEC_I_CONTINUE_NEEDED:
#else
            case GSS_S_COMPLETE:
            case GSS_S_CONTINUE_NEEDED:
#endif
                LWIO_LOG_VERBOSE("GSS-API error calling %s: %d (%s)\n",
                        pszId, code,
                        (char *)msg.value);
                break;

            default:

                LWIO_LOG_ERROR("GSS-API error calling %s: %d (%s)\n",
                        pszId, code,
                        (char *)msg.value);
        }

        (void) gss_release_buffer(&min_stat, &msg);

        if (!msg_ctx)
            break;
    }
}
