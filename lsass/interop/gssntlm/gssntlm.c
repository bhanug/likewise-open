/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright (c) Likewise Software.  All rights Reserved.
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
 * license@likewise.com
 */

/*
 * Copyright (C) Likewise Software.  All rights reserved.
 *
 * Module Name:
 *
 *        gssntlm.c
 *
 * Abstract:
 *
 *        GSS wrapper functions for NTLM implementation.
 *
 * Authors: Marc Guy (mguy@likewisesoftware.com)
 *
 */

#include <ntlm/sspintlm.h>
#include <ntlm/gssntlm.h>
#include <lwerror.h>
#include <lwmem.h>
#include <lwdef.h>
#include <stdarg.h>
#include <lwstr.h>
#include <string.h>
#include <assert.h>

gss_OID_desc gGssNtlmOidDesc = {
    .length = GSS_MECH_NTLM_LEN,
    .elements = GSS_MECH_NTLM
    };

gss_OID gGssNtlmOid = &gGssNtlmOidDesc;

#undef BAIL_ON_LW_ERROR
#define BAIL_ON_LW_ERROR(dwError) \
    do { \
        if (dwError) \
        { \
            goto error; \
        } \
    } while (0)

//
// Since there is no GSSAPI mech plugin header, this must be kept in sync with
// gss_mechanism in krb5/src/lib/gssapi/mglueP.h.
//

typedef struct _GSS_MECH_CONFIG
{
    gss_OID_desc MechType;
    PVOID pContext;

    OM_uint32
    (*gss_acquire_cred)(
        OM_uint32*,
        gss_name_t,
        OM_uint32,
        gss_OID_set,
        INT,
        gss_cred_id_t*,
        gss_OID_set*,
        OM_uint32*
        );

    OM_uint32
    (*gss_release_cred)(
        OM_uint32*,
        gss_cred_id_t*
        );

    OM_uint32
    (*gss_init_sec_cred)(
        OM_uint32*,
        gss_cred_id_t,
        gss_ctx_id_t*,
        gss_name_t,
        gss_OID,
        OM_uint32,
        OM_uint32,
        gss_channel_bindings_t,
        gss_buffer_t,
        gss_OID*,
        gss_buffer_t,
        OM_uint32*,
        OM_uint32*
        );

    OM_uint32
    (*gss_accept_sec_context)(
        OM_uint32*,
        gss_ctx_id_t*,
        gss_cred_id_t,
        gss_buffer_t,
        gss_channel_bindings_t,
        gss_name_t*,
        gss_OID*,
        gss_buffer_t,
        OM_uint32*,
        OM_uint32*,
        gss_cred_id_t*
        );

    OM_uint32
    (*gss_process_context_token)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_buffer_t
        );

    OM_uint32
    (*gss_delete_sec_context)(
        OM_uint32*,
        gss_ctx_id_t*,
        gss_buffer_t
        );

    OM_uint32
    (*gss_context_time)(
        OM_uint32*,
        gss_ctx_id_t,
        OM_uint32*
        );

    OM_uint32
    (*gss_get_mic)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_qop_t,
        gss_buffer_t,
        gss_buffer_t
        );

    OM_uint32
    (*gss_verify_mic)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_buffer_t,
        gss_buffer_t,
        gss_qop_t*
        );

    OM_uint32
    (*gss_wrap)(
        OM_uint32*,
        gss_ctx_id_t,
        INT,
        gss_qop_t,
        gss_buffer_t,
        PINT,
        gss_buffer_t
        );

    OM_uint32
    (*gss_unwrap)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_buffer_t,
        gss_buffer_t,
        PINT,
        gss_qop_t*
        );

    OM_uint32
    (*gss_display_status)(
        OM_uint32*,
        OM_uint32,
        INT,
        gss_OID,
        OM_uint32*,
        gss_buffer_t
        );

    OM_uint32
    (*gss_indicate_mechs)(
        OM_uint32*,
        gss_OID_set*
        );

    OM_uint32
    (*gss_compare_name)(
        OM_uint32*,
        gss_name_t,
        gss_name_t,
        PINT);

    OM_uint32
    (*gss_display_name)(
        OM_uint32*,
        gss_name_t,
        gss_buffer_t,
        gss_OID*
        );

    OM_uint32
    (*gss_import_name)(
        OM_uint32*,
        gss_buffer_t,
        gss_OID,
        gss_name_t*
        );

    OM_uint32
    (*gss_release_name)(
        OM_uint32*,
        gss_name_t*);

    OM_uint32
    (*gss_inquire_cred)(
        OM_uint32*,
        gss_cred_id_t,
        gss_name_t*,
        OM_uint32*,
        PINT,
        gss_OID_set*
        );

    OM_uint32
    (*gss_add_cred)(
        OM_uint32*,
        gss_cred_id_t,
        gss_name_t,
        gss_OID,
        gss_cred_usage_t,
        OM_uint32,
        OM_uint32,
        gss_cred_id_t*,
        gss_OID_set*,
        OM_uint32*,
        OM_uint32*
        );

    OM_uint32
    (*gss_export_sec_context)(
        OM_uint32,
        gss_ctx_id_t*,
        gss_buffer_t
        );

    OM_uint32
    (*gss_import_sec_context)(
        OM_uint32,
        gss_buffer_t,
        gss_ctx_id_t*
        );

    OM_uint32
    (*gss_inquire_cred_by_mech)(
        OM_uint32*,
        gss_cred_id_t,
        gss_OID,
        gss_name_t*,
        OM_uint32*,
        OM_uint32*,
        gss_cred_usage_t*
        );

    OM_uint32
    (*gss_inquire_names_for_mech)(
        OM_uint32,
        gss_OID,
        gss_OID_set*
        );

    OM_uint32
    (*gss_inquire_context)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_name_t*,
        gss_name_t*,
        OM_uint32*,
        gss_OID*,
        OM_uint32*,
        PINT,
        PINT
        );

    OM_uint32
    (*gss_internal_release_oid)(
        OM_uint32*,
        gss_OID*
        );

    OM_uint32
    (*gss_wrap_size_limit)(
        OM_uint32*,
        gss_ctx_id_t,
        INT,
        gss_qop_t,
        OM_uint32,
        OM_uint32*
        );

    OM_uint32
    (*gss_export_name)(
        OM_uint32*,
        const gss_name_t,
        gss_buffer_t
        );

    OM_uint32
    (*gss_store_cred)(
        OM_uint32*,
        const gss_cred_id_t,
        gss_cred_usage_t,
        const gss_OID,
        OM_uint32,
        OM_uint32,
        gss_OID_set*,
        gss_cred_usage_t*
        );

    OM_uint32
    (*gss_inquire_sec_context_by_oid)(
        OM_uint32*,
        const gss_ctx_id_t,
        const gss_OID,
        gss_buffer_set_t*
        );

    OM_uint32
    (*gss_inquire_cred_by_oid)(
        OM_uint32*,
        const gss_cred_id_t,
        const gss_OID,
        gss_buffer_set_t*
        );

    OM_uint32
    (*gss_set_sec_context_option)(
        OM_uint32*,
        gss_ctx_id_t*,
        const gss_OID,
        const gss_buffer_t
        );

    OM_uint32
    (*gssspi_set_cred_option)(
        OM_uint32*,
        gss_cred_id_t,
        const gss_OID,
        const gss_buffer_t
        );

    OM_uint32
    (*gssspi_mech_invoke)(
        OM_uint32*,
        const gss_OID,
        const gss_OID,
        gss_buffer_t
        );

    OM_uint32
    (*gss_wrap_aead)(
        OM_uint32*,
        gss_ctx_id_t,
        INT,
        gss_qop_t,
        gss_buffer_t,
        gss_buffer_t,
        PINT,
        gss_buffer_t
        );

    OM_uint32
    (*gss_unwrap_aead)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_buffer_t,
        gss_buffer_t,
        gss_buffer_t,
        PINT,
        gss_qop_t*);

    OM_uint32
    (*gss_wrap_iov)(
        OM_uint32*,
        gss_ctx_id_t,
        INT,
        gss_qop_t,
        PINT,
        gss_iov_buffer_desc,
        INT
        );

    OM_uint32
    (*gss_unwrap_iov)(
        OM_uint32*,
        gss_ctx_id_t,
        PINT,
        gss_qop_t*,
        gss_iov_buffer_desc*,
        INT
        );

    OM_uint32
    (*gss_wrap_iov_length)(
        OM_uint32*,
        gss_ctx_id_t,
        INT,
        gss_qop_t,
        PINT,
        gss_iov_buffer_desc*,
        INT
        );

    OM_uint32
    (*gss_complete_auth_token)(
        OM_uint32*,
        const gss_ctx_id_t,
        gss_buffer_t
        );

    OM_uint32
    (*gss_inquire_context2)(
        OM_uint32*,
        gss_ctx_id_t,
        gss_name_t*,
        gss_name_t*,
        OM_uint32*,
        gss_OID*,
        OM_uint32*,
        PINT,
        PINT,
        gss_buffer_t
        );

} GSS_MECH_CONFIG, *PGSS_MECH_CONFIG;

//
// Prototypes
//

PGSS_MECH_CONFIG
gss_mech_initialize(
    void
    );

//
// Globals
//

static GSS_MECH_CONFIG gNtlmMech =
{
    {GSS_MECH_NTLM_LEN, GSS_MECH_NTLM},
    NULL,

    ntlm_gss_acquire_cred,
    ntlm_gss_release_cred,
    ntlm_gss_init_sec_context,
    ntlm_gss_accept_sec_context,
    NULL, //ntlm_gss_process_context_token,
    ntlm_gss_delete_sec_context,
    NULL, //ntlm_gss_context_time,
    ntlm_gss_get_mic,
    ntlm_gss_verify_mic,
    ntlm_gss_wrap,
    ntlm_gss_unwrap,
    NULL, //ntlm_gss_display_status,
    NULL, //ntlm_gss_indicate_mechs,
    NULL, //ntlm_gss_compare_name,
    ntlm_gss_display_name,
    ntlm_gss_import_name,
    ntlm_gss_release_name,
    ntlm_gss_inquire_cred,
    NULL, //ntlm_gss_add_cred,
    NULL, //ntlm_gss_export_sec_context,
    NULL, //ntlm_gss_import_sec_context,
    NULL, //ntlm_gss_inquire_cred_by_mech,
    NULL, //ntlm_gss_inquire_names_for_mech,
    ntlm_gss_inquire_context,
    NULL, //ntlm_gss_internal_release_oid,
    NULL, //ntlm_gss_wrap_size_limit,
    NULL, //ntlm_gss_export_name,
    NULL, //ntlm_gss_store_cred,
    ntlm_gss_inquire_sec_context_by_oid,
    NULL, //ntlm_gss_inquire_cred_by_oid,
    NULL, //ntlm_gss_set_sec_context_option,
    NULL, //ntlm_gssspi_set_cred_option,
    NULL, //ntlm_gssspi_mech_invoke,
    NULL, //ntlm_gss_wrap_aead,
    NULL, //ntlm_gss_unwrap_aead,
    NULL, //ntlm_gss_wrap_iov,
    NULL, //ntlm_gss_unwrap_iov,
    NULL, //ntlm_gss_wrap_iov_length,
    NULL, //ntlm_gss_complete_auth_token,
    NULL, //ntlm_gss_inquire_context2
};

//
// Function Definitions
//

OM_uint32
ntlm_gss_acquire_cred(
    OM_uint32* pMinorStatus,
    const gss_name_t pDesiredName,
    OM_uint32 nTimeReq,
    const gss_OID_set pDesiredMechs,
    gss_cred_usage_t CredUsage,
    gss_cred_id_t* pOutputCredHandle,
    gss_OID_set* pActualMechs,
    OM_uint32 *pTimeRec
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CRED_HANDLE CredHandle = NULL;
    TimeStamp tsExpiry = 0;
    DWORD fCredentialUse = 0;

    *pOutputCredHandle = (gss_cred_id_t)CredHandle;

    if (pActualMechs)
    {
        *pActualMechs = NULL;
    }

    if (pTimeRec)
    {
        *pTimeRec = 0;
    }

    switch(CredUsage)
    {
    case GSS_C_ACCEPT:
        fCredentialUse = NTLM_CRED_INBOUND;
        break;
    case GSS_C_INITIATE:
        fCredentialUse = NTLM_CRED_OUTBOUND;
        break;
    default:
        MinorStatus = LW_ERROR_INVALID_PARAMETER;
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    MinorStatus = NtlmClientAcquireCredentialsHandle(
        (SEC_CHAR*)pDesiredName,
        "NTLM",
        fCredentialUse,
        NULL,
        NULL,
        &CredHandle,
        &tsExpiry
        );
    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    *pMinorStatus = MinorStatus;
    *pOutputCredHandle = (gss_cred_id_t)CredHandle;

    if (pActualMechs)
    {
        *pActualMechs = NULL;
    }

    if (pTimeRec)
    {
        *pTimeRec = 0;
    }

    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_release_cred(
    OM_uint32* pMinorStatus,
    gss_cred_id_t* pCredHandle
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CRED_HANDLE CredHandle = NULL;

    if (!pCredHandle)
    {
        MajorStatus = GSS_S_NO_CRED;
        MinorStatus = LW_ERROR_NO_CRED;
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    CredHandle = (NTLM_CRED_HANDLE)*pCredHandle;

    MinorStatus = NtlmClientFreeCredentialsHandle(
        &CredHandle
        );

    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    *pMinorStatus = MinorStatus;
    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_init_sec_context(
    OM_uint32* pMinorStatus,
    const gss_cred_id_t InitiatorCredHandle,
    gss_ctx_id_t* pContextHandle,
    const gss_name_t pTargetName,
    const gss_OID pMechType,
    OM_uint32 nReqFlags,
    OM_uint32 nTimeReq,
    const gss_channel_bindings_t pInputChanBindings,
    const gss_buffer_t pInputToken,
    gss_OID* pActualMechType,
    gss_buffer_t pOutputToken,
    OM_uint32* pRetFlags,
    OM_uint32* pTimeRec
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE hContext = NULL;
    NTLM_CONTEXT_HANDLE hNewContext = NULL;
    TimeStamp tsExpiry = 0;
    SecBufferDesc InputBuffer = {0};
    SecBufferDesc OutputBuffer = {0};
    SecBuffer InputToken = {0};
    SecBuffer OutputToken = {0};
    NTLM_CRED_HANDLE CredHandle = (NTLM_CRED_HANDLE)InitiatorCredHandle;
    TimeStamp Expiry = 0;
    DWORD dwNtlmFlags = NTLM_FLAG_NEGOTIATE_DEFAULT;

    InputBuffer.cBuffers = 1;
    InputBuffer.pBuffers = &InputToken;

    OutputBuffer.cBuffers = 1;
    OutputBuffer.pBuffers = &OutputToken;

    InputToken.BufferType = SECBUFFER_TOKEN;

    if(pInputToken)
    {
        InputToken.cbBuffer = pInputToken->length;
        InputToken.pvBuffer = pInputToken->value;
    }

    if( pContextHandle)
    {
        hContext = (NTLM_CONTEXT_HANDLE)*pContextHandle;
    }

    if (nReqFlags & GSS_C_CONF_FLAG)
    {
        dwNtlmFlags |= NTLM_FLAG_SEAL;
    }

    if (nReqFlags & GSS_C_INTEG_FLAG)
    {
        dwNtlmFlags |= NTLM_FLAG_SIGN;
    }
    else
    {
        dwNtlmFlags |= NTLM_FLAG_ALWAYS_SIGN;
    }

    // if no credentials are passed in, create default creds
    if (GSS_C_NO_CREDENTIAL == InitiatorCredHandle)
    {
        MinorStatus = NtlmClientAcquireCredentialsHandle(
            NULL,
            "NTLM",
            NTLM_CRED_OUTBOUND,
            NULL,
            NULL,
            &CredHandle,
            &Expiry
            );
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    MinorStatus = NtlmClientInitializeSecurityContext(
        &CredHandle,
        &hContext,
        (SEC_CHAR*)pTargetName,
        dwNtlmFlags,
        0, // Reserved
        NTLM_NATIVE_DATA_REP,
        &InputBuffer,
        0, // Reserved
        &hNewContext,
        &OutputBuffer,
        pRetFlags,
        &tsExpiry
        );

    if (MinorStatus == LW_WARNING_CONTINUE_NEEDED)
    {
        MajorStatus = GSS_S_CONTINUE_NEEDED;
    }
    else
    {
        BAIL_ON_LW_ERROR(MinorStatus);
    }


cleanup:
    *pMinorStatus = MinorStatus;

    if (GSS_C_NO_CREDENTIAL == InitiatorCredHandle && CredHandle)
    {
        NtlmClientFreeCredentialsHandle(&CredHandle);
    }

    if (pOutputToken)
    {
        pOutputToken->length = OutputToken.cbBuffer;
        pOutputToken->value = OutputToken.pvBuffer;
    }

    if (pActualMechType)
    {
        *pActualMechType = gGssNtlmOid;
    }

    if (pRetFlags)
    {
        *pRetFlags = 0;
    }

    if (pTimeRec)
    {
        *pTimeRec = GSS_C_INDEFINITE;
    }

    if (pContextHandle)
    {
        *pContextHandle = (gss_ctx_id_t)hNewContext;
    }

    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_accept_sec_context(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t *pContextHandle,
    const gss_cred_id_t AcceptorCredHandle,
    const gss_buffer_t pInputTokenBuffer,
    const gss_channel_bindings_t pInputChanBindings,
    gss_name_t* pSrcName,
    gss_OID* pMechType,
    gss_buffer_t pOutputToken,
    OM_uint32* pRetFlags,
    OM_uint32* pTimeRec,
    gss_cred_id_t* pDelegatedCredHandle
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    DWORD dwRetFlags = 0;
    DWORD dwFinalFlags = 0;
    SecBufferDesc InputBuffer = {0};
    SecBufferDesc OutputBuffer = {0};
    SecBuffer InputToken = {0};
    SecBuffer OutputToken = {0};
    TimeStamp tsExpiry = 0;
    NTLM_CONTEXT_HANDLE NewCtxtHandle = NULL;

    *pMinorStatus = LW_ERROR_SUCCESS;

    if (pSrcName)
    {
        *pSrcName = NULL;
    }
    if (pMechType)
    {
        *pMechType = NULL;
    }
    if (pTimeRec)
    {
        *pTimeRec = 0;
    }
    if (pDelegatedCredHandle)
    {
        *pDelegatedCredHandle = NULL;
    }
    if (pRetFlags)
    {
        dwRetFlags = *pRetFlags;
    }

#if 0
    // convert
    switch (dwRetFlags)
    {
    case GSS_C_DELEG_FLAG:
        break;
    case GSS_C_MUTUAL_FLAG:
        break;
    case GSS_C_REPLAY_FLAG:
        break;
    case GSS_C_SEQUENCE_FLAG:
        break;
    case GSS_C_CONF_FLAG:
        break;
    case GSS_C_INTEG_FLAG:
        break;
    case GSS_C_ANON_FLAG:
        break;
    case GSS_C_PROT_READY_FLAG:
        break;
    case GSS_C_TRANS_FLAG:
        break;
    }
#endif

    memset(pOutputToken, 0, sizeof(*pOutputToken));

    InputBuffer.cBuffers = 1;
    InputBuffer.pBuffers = &InputToken;

    OutputBuffer.cBuffers = 1;
    OutputBuffer.pBuffers = &OutputToken;

    InputToken.BufferType = SECBUFFER_TOKEN;
    InputToken.cbBuffer = pInputTokenBuffer->length;
    InputToken.pvBuffer = pInputTokenBuffer->value;

    MinorStatus = NtlmClientAcceptSecurityContext(
        (PNTLM_CRED_HANDLE)(&AcceptorCredHandle),
        (PNTLM_CONTEXT_HANDLE)pContextHandle,
        &InputBuffer,
        dwFinalFlags,
        NTLM_NATIVE_DATA_REP,
        &NewCtxtHandle,
        &OutputBuffer,
        pTimeRec,
        &tsExpiry);

    if (MinorStatus == LW_WARNING_CONTINUE_NEEDED)
    {
        MajorStatus = GSS_S_CONTINUE_NEEDED;
    }
    else
    {
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    pOutputToken->length = OutputToken.cbBuffer;
    pOutputToken->value = OutputToken.pvBuffer;

    *pContextHandle = (gss_ctx_id_t)NewCtxtHandle;
cleanup:
    *pMinorStatus = MinorStatus;
    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_delete_sec_context(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t* pContextHandle,
    gss_buffer_t OutputToken
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE ContextHandle = NULL;

    if (OutputToken)
    {
        OutputToken = GSS_C_NO_BUFFER;
    }

    if (!pContextHandle || !*pContextHandle)
    {
        MajorStatus = GSS_S_NO_CONTEXT;
        MinorStatus = LW_ERROR_NO_CONTEXT;
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    ContextHandle = (NTLM_CONTEXT_HANDLE)*pContextHandle;

    MinorStatus = NtlmClientDeleteSecurityContext(
        &ContextHandle
        );

    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    *pMinorStatus = MinorStatus;
    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_get_mic(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t GssCtxtHandle,
    gss_qop_t Qop,
    gss_buffer_t Message,
    gss_buffer_t Token
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE ContextHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecBufferDesc NtlmMessage = {0};
    SecBuffer NtlmBuffer[2];
    PBYTE pNtlmToken = NULL;
    SecPkgContext_Sizes spcSizes = {0};

    if(Qop != GSS_C_QOP_DEFAULT)
    {
        MajorStatus = GSS_S_BAD_QOP;
        BAIL_ON_LW_ERROR(MajorStatus);
    }

    MinorStatus = NtlmClientQueryContextAttributes(
        &ContextHandle,
        SECPKG_ATTR_SIZES,
        &spcSizes
        );
    BAIL_ON_LW_ERROR(MinorStatus);

    NtlmMessage.cBuffers = 2;
    NtlmMessage.pBuffers = NtlmBuffer;

    MinorStatus = LwAllocateMemory(
        spcSizes.cbMaxSignature,
        OUT_PPVOID(&pNtlmToken));
    BAIL_ON_LW_ERROR(MinorStatus);

    NtlmBuffer[0].BufferType = SECBUFFER_DATA;
    NtlmBuffer[0].cbBuffer = Message->length;
    NtlmBuffer[0].pvBuffer = Message->value;

    NtlmBuffer[1].BufferType = SECBUFFER_TOKEN;
    NtlmBuffer[1].cbBuffer = spcSizes.cbMaxSignature;
    NtlmBuffer[1].pvBuffer = pNtlmToken;

    MinorStatus = NtlmClientMakeSignature(
        &ContextHandle,
        0,
        &NtlmMessage,
        0
        );
    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    *pMinorStatus = MinorStatus;

    // NtlmClientMakeSignature is going to replace NtlmBuffer[1].pvBuffer so
    // it's safe to free this buffer.
    LW_SAFE_FREE_MEMORY(pNtlmToken);

    Token->value = NtlmBuffer[1].pvBuffer;
    if(Token->value)
    {
        Token->length = NtlmBuffer[1].cbBuffer;
    }

    return MajorStatus;

error:
    LW_SAFE_FREE_MEMORY(pNtlmToken);

    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_verify_mic(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t GssCtxtHandle,
    gss_buffer_t Message,
    gss_buffer_t Token,
    gss_qop_t* pQop
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE ContextHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecBufferDesc NtlmMessage = {0};
    SecBuffer NtlmBuffer[2];
    PNTLM_SIGNATURE pSignature = NULL;
    DWORD dwQop = GSS_C_QOP_DEFAULT;

    NtlmMessage.cBuffers = 2;
    NtlmMessage.pBuffers = NtlmBuffer;

    NtlmBuffer[0].BufferType = SECBUFFER_DATA;
    NtlmBuffer[0].cbBuffer = Message->length;
    NtlmBuffer[0].pvBuffer = Message->value;

    NtlmBuffer[1].BufferType = SECBUFFER_TOKEN;
    NtlmBuffer[1].cbBuffer = Token->length;
    NtlmBuffer[1].pvBuffer = Token->value;

    MinorStatus = NtlmClientVerifySignature(
        &ContextHandle,
        &NtlmMessage,
        0,
        &dwQop
        );

    if (MinorStatus)
    {
        MajorStatus = GSS_S_BAD_SIG;
    }
    BAIL_ON_LW_ERROR(MinorStatus);

    pSignature = (PNTLM_SIGNATURE)Token->value;

    if (pSignature->dwVersion == NTLM_VERSION &&
        pSignature->dwCounterValue == 0 &&
        pSignature->dwCrc32 == 0 &&
        pSignature->dwMsgSeqNum == 0)
    {
        dwQop = GSS_C_QOP_DUMMY_SIG;
    }

cleanup:
    if(pQop)
    {
        *pQop = (gss_qop_t)dwQop;
    }

    *pMinorStatus = MinorStatus;
    return MajorStatus;

error:
    dwQop = GSS_C_QOP_DEFAULT;

    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_wrap(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t GssCtxtHandle,
    INT nEncrypt,
    gss_qop_t Qop,
    gss_buffer_t InputMessage,
    PINT pEncrypted,
    gss_buffer_t OutputMessage
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE ContextHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecBufferDesc Message = {0};
    SecBuffer NtlmBuffer[2];
    SecPkgContext_Sizes Sizes = {0};
    DWORD dwBufferSize = 0;
    PBYTE pBuffer = NULL;
    INT nEncrypted = 0;

    Message.cBuffers = 2;
    Message.pBuffers = NtlmBuffer;

    memset(NtlmBuffer, 0, sizeof(SecBuffer) * Message.cBuffers);

    if(Qop != GSS_C_QOP_DEFAULT)
    {
        MajorStatus = GSS_S_BAD_QOP;
        BAIL_ON_LW_ERROR(MajorStatus);
    }

    // Since every encrypted message gets a signature, we need to get the size
    MinorStatus = NtlmClientQueryContextAttributes(
        &ContextHandle,
        SECPKG_ATTR_SIZES,
        &Sizes
        );
    BAIL_ON_LW_ERROR(MinorStatus);

    dwBufferSize += Sizes.cbMaxSignature;       // Token
    dwBufferSize += InputMessage->length;       // Data

    // We only need the padding for the duration of this operation, it should
    // not be returned with the rest of the data.

    dwBufferSize += Sizes.cbSecurityTrailer;    // Padding

    MinorStatus = LwAllocateMemory(dwBufferSize, OUT_PPVOID(&pBuffer));
    BAIL_ON_LW_ERROR(MinorStatus);

    NtlmBuffer[0].BufferType = SECBUFFER_TOKEN;
    NtlmBuffer[0].cbBuffer = Sizes.cbMaxSignature;
    NtlmBuffer[0].pvBuffer = pBuffer;

    NtlmBuffer[1].BufferType = SECBUFFER_DATA;
    NtlmBuffer[1].cbBuffer = InputMessage->length;
    NtlmBuffer[1].pvBuffer =
        (PBYTE)NtlmBuffer[0].pvBuffer + NtlmBuffer[0].cbBuffer;

    memcpy(
        NtlmBuffer[1].pvBuffer,
        InputMessage->value,
        NtlmBuffer[1].cbBuffer);

    MinorStatus = NtlmClientEncryptMessage(
        &ContextHandle,
        nEncrypt ? TRUE : FALSE,
        &Message,
        0
        );
    BAIL_ON_LW_ERROR(MinorStatus);

    // As noted above, we'll trim the size down to exclude the padding
    dwBufferSize -= Sizes.cbSecurityTrailer;

    if (nEncrypt)
    {
        nEncrypted = 1;
    }

cleanup:
    OutputMessage->value = pBuffer;
    OutputMessage->length = dwBufferSize;

    if (pEncrypted)
    {
        *pEncrypted = nEncrypted;
    }

    *pMinorStatus = MinorStatus;
    return MajorStatus;

error:
    LW_SAFE_FREE_MEMORY(pBuffer);
    dwBufferSize = 0;

    nEncrypted = 0;

    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_unwrap(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t GssCtxtHandle,
    gss_buffer_t InputMessage,
    gss_buffer_t OutputMessage,
    PINT pEncrypted,
    gss_qop_t* pQop
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    NTLM_CONTEXT_HANDLE ContextHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecBufferDesc Message;
    SecBuffer NtlmBuffer[2];
    PBYTE pBuffer = NULL;
    DWORD dwBufferSize = 0;
    BOOLEAN bEncrypted = FALSE;
    SecPkgContext_Sizes Sizes = {0};

    LW_ASSERT(InputMessage);

    Message.cBuffers = 2;
    Message.pBuffers = NtlmBuffer;

    memset(NtlmBuffer, 0, sizeof(SecBuffer) * Message.cBuffers);

    // Since every encrypted message gets a signature, we need to get the size
    MinorStatus = NtlmClientQueryContextAttributes(
        &ContextHandle,
        SECPKG_ATTR_SIZES,
        &Sizes
        );
    BAIL_ON_LW_ERROR(MinorStatus);

    LW_ASSERT(InputMessage->length > Sizes.cbMaxSignature);

    // Here we are taking out the signature, but adding back in for the
    // padding.  The padding is only needed for the duration of the operation
    // and will be ignored afterwards.
    dwBufferSize =
        InputMessage->length - Sizes.cbMaxSignature + Sizes.cbSecurityTrailer;

    MinorStatus = LwAllocateMemory(
        dwBufferSize,
        OUT_PPVOID(&pBuffer));
    BAIL_ON_LW_ERROR(MinorStatus);

    // Reduce the size to exclude the padding trailer
    dwBufferSize -= Sizes.cbSecurityTrailer;

    // Copy the input into our buffer... making sure to skip past the signature
    memcpy(
        pBuffer,
        (PBYTE)InputMessage->value + Sizes.cbMaxSignature,
        dwBufferSize);

    // We should be getting in a blob containing a signature and data... no size
    // information.  We place these values into a SECBUFFER_TOKEN and a
    // SECBUFFER_DATA (see ntlm_gss_wrap for how these get packaged
    // together).  Then we send them to the decryptor.
    NtlmBuffer[0].BufferType = SECBUFFER_TOKEN;
    NtlmBuffer[0].cbBuffer = Sizes.cbMaxSignature;
    NtlmBuffer[0].pvBuffer = InputMessage->value;

    NtlmBuffer[1].BufferType = SECBUFFER_DATA;
    NtlmBuffer[1].cbBuffer = dwBufferSize;
    NtlmBuffer[1].pvBuffer = pBuffer;

    MinorStatus = NtlmClientDecryptMessage(
        &ContextHandle,
        &Message,
        0,
        &bEncrypted
        );
    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    // NtlmClientDecryptMessage is going to replace NtlmBuffer[1].pvBuffer, so
    // it should be safe to free this buffer here.
    LW_SAFE_FREE_MEMORY(pBuffer);

    OutputMessage->value = NtlmBuffer[1].pvBuffer;
    OutputMessage->length = NtlmBuffer[1].cbBuffer;

    if (pEncrypted)
    {
        *pEncrypted = bEncrypted;
    }

    *pMinorStatus = MinorStatus;
    return MajorStatus;

error:
    NtlmBuffer[1].pvBuffer = NULL;
    NtlmBuffer[1].cbBuffer = 0;

    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_display_name(
    OM_uint32* pMinorStatus,
    gss_name_t pGssName,
    gss_buffer_t pOutputName,
    gss_OID* ppNameType
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    gss_buffer_t pName = (gss_buffer_t)pGssName;

    if (pName && pOutputName)
    {
        MinorStatus = LwAllocateMemory(pName->length, &pOutputName->value);
        BAIL_ON_LW_ERROR(MinorStatus);

        pOutputName->length = pName->length;
        memcpy(pOutputName->value, pName->value, pOutputName->length);
    }

    if (ppNameType)
    {
        *ppNameType = GSS_C_NT_HOSTBASED_SERVICE;
    }

cleanup:
    *pMinorStatus = MinorStatus;
    return MajorStatus;

error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_import_name(
    OM_uint32* pMinorStatus,
    const gss_buffer_t InputNameBuffer,
    const gss_OID InputNameType,
    gss_name_t* pOutputName
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;

    *pOutputName = GSS_C_NO_NAME;

    BAIL_ON_LW_ERROR(MinorStatus);

cleanup:
    *pMinorStatus = MinorStatus;
    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }
    goto cleanup;
}

OM_uint32
ntlm_gss_release_name(
    OM_uint32* pMinorStatus,
    gss_name_t* ppName
    )
{
    gss_buffer_t pName = NULL;

    if (ppName)
    {
        pName = (gss_buffer_t)*ppName;

        if (pName && pName->length && pName->value)
        {
            LW_SAFE_FREE_MEMORY(pName->value);
            pName->length = 0;
        }

        *ppName = GSS_C_NO_NAME;
    }

    *pMinorStatus = LW_ERROR_SUCCESS;
    return GSS_S_COMPLETE;
}

OM_uint32
ntlm_gss_inquire_cred(
    OM_uint32* pMinorStatus,
    const gss_cred_id_t CredHandle,
    gss_name_t* pName,
    OM_uint32* pLifeTime,
    gss_cred_usage_t* pCredUsage,
    gss_OID_set* pMechs
    )
{
    if (pName)
    {
        *pName = GSS_C_NO_NAME;
    }
    if (pLifeTime)
    {
        *pLifeTime = 0;
    }
    if (pCredUsage)
    {
        *pCredUsage = 0;
    }
    if (pMechs)
    {
        *pMechs = GSS_C_NO_OID_SET;
    }

    return GSS_S_COMPLETE;
}

OM_uint32
ntlm_gss_inquire_context(
    OM_uint32* pMinorStatus,
    gss_ctx_id_t GssCtxtHandle,
    gss_name_t* ppSourceName,
    gss_name_t* ppTargetName,
    OM_uint32* pLifeTime,
    gss_OID* pMechType,
    OM_uint32* pCtxtFlags,
    PINT pLocal,
    PINT pOpen
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    gss_buffer_t pUserName = NULL;
    NTLM_CONTEXT_HANDLE NtlmCtxtHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecPkgContext_Names Names = { 0 };
    gss_name_t pSourceName = NULL;

    if (ppTargetName || pCtxtFlags || pLocal || pOpen)
    {
        MinorStatus = LW_ERROR_NOT_SUPPORTED;
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    if (ppSourceName)
    {
        MinorStatus = NtlmClientQueryContextAttributes(
            &NtlmCtxtHandle,
            SECPKG_ATTR_NAMES,
            &Names);
        BAIL_ON_LW_ERROR(MinorStatus);

        MinorStatus = LwAllocateMemory(
            sizeof(*pUserName),
            OUT_PPVOID(&pUserName));
        BAIL_ON_LW_ERROR(MinorStatus);

        MinorStatus = LwAllocateString(
            Names.pUserName,
            (PSTR*)(PSTR)(&pUserName->value));
        BAIL_ON_LW_ERROR(MinorStatus);

        pUserName->length = strlen(pUserName->value);

        pSourceName = (gss_name_t)pUserName;
        pUserName = NULL;
    }

cleanup:
    if(Names.pUserName)
    {
        NtlmFreeContextBuffer(Names.pUserName);
    }

    if(pUserName)
    {
        LW_SAFE_FREE_MEMORY(pUserName->value);
        LW_SAFE_FREE_MEMORY(pUserName);
    }

    if (ppSourceName)
    {
        *ppSourceName = pSourceName;
    }

    if (pLifeTime)
    {
        *pLifeTime = GSS_C_INDEFINITE;
    }

    if (pMechType)
    {
        *pMechType = gGssNtlmOid;
    }

    return MajorStatus;

error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }

    ntlm_gss_release_name(&MinorStatus, &pSourceName);

    goto cleanup;
}

OM_uint32
ntlm_gss_inquire_sec_context_by_oid(
    OM_uint32* pMinorStatus,
    const gss_ctx_id_t GssCtxtHandle,
    const gss_OID Attrib,
    gss_buffer_set_t* ppBufferSet
    )
{
    OM_uint32 MajorStatus = GSS_S_COMPLETE;
    OM_uint32 MinorStatus = LW_ERROR_SUCCESS;
    gss_OID SessionKeyOid = GSS_C_INQ_SSPI_SESSION_KEY;
    gss_OID NamesOid = GSS_C_NT_STRING_UID_NAME;
    NTLM_CONTEXT_HANDLE NtlmCtxtHandle = (NTLM_CONTEXT_HANDLE)GssCtxtHandle;
    SecPkgContext_SessionKey SessionKey = {0};
    SecPkgContext_Names Names = {0};
    gss_buffer_set_t pBufferSet = NULL;
    gss_buffer_t pBuffer = NULL;

    MinorStatus = LwAllocateMemory(
        sizeof(*pBufferSet),
        OUT_PPVOID(&pBufferSet));
    BAIL_ON_LW_ERROR(MinorStatus);

    MinorStatus = LwAllocateMemory(
        sizeof(*pBuffer),
        OUT_PPVOID(&pBuffer));
    BAIL_ON_LW_ERROR(MinorStatus);

    if (Attrib->length == SessionKeyOid->length &&
        !memcmp(Attrib->elements, SessionKeyOid->elements, Attrib->length))
    {
        MinorStatus = NtlmClientQueryContextAttributes(
            &NtlmCtxtHandle,
            SECPKG_ATTR_SESSION_KEY,
            &SessionKey);
        BAIL_ON_LW_ERROR(MinorStatus);

        pBuffer->value = SessionKey.SessionKey;
        pBuffer->length = SessionKey.SessionKeyLength;
    }
    else if (Attrib->length == NamesOid->length &&
        !memcmp(Attrib->elements, NamesOid->elements, Attrib->length))
    {
        MinorStatus = NtlmClientQueryContextAttributes(
            &NtlmCtxtHandle,
            SECPKG_ATTR_NAMES,
            &Names);
        BAIL_ON_LW_ERROR(MinorStatus);

        pBuffer->value = Names.pUserName;
        pBuffer->length = strlen(pBuffer->value);
    }
    else
    {
        MinorStatus = LW_ERROR_INVALID_PARAMETER;
        BAIL_ON_LW_ERROR(MinorStatus);
    }

    pBufferSet->count = 1;
    pBufferSet->elements = pBuffer;

cleanup:
    *pMinorStatus = MinorStatus;
    *ppBufferSet = pBufferSet;

    return MajorStatus;
error:
    if (MajorStatus == GSS_S_COMPLETE)
    {
        MajorStatus = GSS_S_FAILURE;
    }

    LW_SAFE_FREE_MEMORY(pBuffer);
    LW_SAFE_FREE_MEMORY(pBufferSet);

    goto cleanup;
}

PGSS_MECH_CONFIG
gss_mech_initialize(void)
{
    return &gNtlmMech;
}
