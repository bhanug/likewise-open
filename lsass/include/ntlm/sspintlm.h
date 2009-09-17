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
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        sspintlm.h
 *
 * Abstract:
 *
 *          Common structure definitions
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Marc Guy (mguy@likewisesoftware.com)
 *
 */
#ifndef __SSPINTLM_H__
#define __SSPINTLM_H__

#include <lw/types.h>
#include <lw/attrs.h>

//******************************************************************************
//
// S T R U C T S
//

typedef CHAR SEC_CHAR;

typedef struct _SecBuffer
{
    DWORD cbBuffer;
    DWORD BufferType;
    PVOID pvBuffer;
} SecBuffer, *PSecBuffer;

typedef struct _SecBufferDesc
{
    // At this point, we do not require version information
    // DWORD      ulVersion;
    DWORD cBuffers;
    PSecBuffer pBuffers;
} SecBufferDesc, *PSecBufferDesc;

typedef struct _SecPkgContext_Names
{
    SEC_CHAR *pUserName;
} SecPkgContext_Names, *PSecPkgContext_Names;

typedef struct _SecPkgContext_SessionKey
{
    ULONG SessionKeyLength;
    PBYTE SessionKey;
} SecPkgContext_SessionKey, *PSecPkgContext_SessionKey;

typedef struct _SecPkgContext_Sizes
{
    DWORD cbMaxToken;
    DWORD cbMaxSignature;
    DWORD cbBlockSize;
    DWORD cbSecurityTrailer;
} SecPkgContext_Sizes, *PSecPkgContext_Sizes;

typedef union _SecPkgContext
{
    PSecPkgContext_Names pNames;
    PSecPkgContext_SessionKey pSessionKey;
    PSecPkgContext_Sizes pSizes;
} SecPkgContext, *PSecPkgContext;

typedef struct _SecPkgCred_Names
{
    SEC_CHAR *pUserName;
} SecPkgCred_Names, *PSecPkgCred_Names;

typedef struct _SecPkgCred
{
    PSecPkgCred_Names pNames;
} SecPkgCred, *PSecPkgCred;

typedef struct _LUID
{
    DWORD LowPart;
    INT HighPart;
} LUID, *PLUID;

typedef struct _SEC_WINNT_AUTH_IDENTITY
{
    PCHAR User;
    DWORD UserLength;
    PCHAR Domain;
    DWORD DomainLength;
    PCHAR Password;
    DWORD PasswordLength;
    DWORD Flags;
} SEC_WINNT_AUTH_IDENTITY, *PSEC_WINNT_AUTH_IDENTITY;

typedef INT64 SECURITY_INTEGER, *PSECURITY_INTEGER;

typedef SECURITY_INTEGER TimeStamp, *PTimeStamp;

typedef struct _SECURITY_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PUSHORT Buffer;
} SECURITY_STRING, * PSECURITY_STRING;

typedef struct _NTLM_SEC_BUFFER
{
    USHORT usLength;
    USHORT usMaxLength;
    DWORD dwOffset;
} NTLM_SEC_BUFFER, *PNTLM_SEC_BUFFER;

typedef struct _WIN_VERSION_INFO
{
    BYTE bMajor;
    BYTE bMinor;
    SHORT sBuild;
    DWORD dwReserved;
} WIN_VERSION_INFO, *PWIN_VERSION_INFO;

struct _NTLM_CONTEXT;
typedef struct _NTLM_CONTEXT *NTLM_CONTEXT_HANDLE, **PNTLM_CONTEXT_HANDLE;

#define NTLM_CRED_INBOUND    1
#define NTLM_CRED_OUTBOUND   2

struct _NTLM_CREDENTIALS;
typedef struct _NTLM_CREDENTIALS *NTLM_CRED_HANDLE, **PNTLM_CRED_HANDLE;

typedef struct _NTLM_SIGNATURE
{
    DWORD dwVersion;
    DWORD dwCounterValue;
    DWORD dwCrc32;
    DWORD dwMsgSeqNum;
} NTLM_SIGNATURE, *PNTLM_SIGNATURE;

//******************************************************************************
//
// D E F I N E S
//

#define NTLM_CTXT_ATTR_SESSION_KEY  1
#define NTLM_CTXT_ATTR_SIZES        2

#define SECBUFFER_TOKEN   0
#define SECBUFFER_DATA    1
#define SECBUFFER_PADDING 2
#define SECBUFFER_STREAM  10

#define SEC_WINNT_AUTH_IDENTITY_UNICODE  0
#define SEC_WINNT_AUTH_IDENTITY_ANSI     1

#define NTLM_NATIVE_DATA_REP    0
#define NTLM_OTHER_DATA_REP   1

#define NTLM_VERSION                    0x00000001

//  NTLM FLAGS
//
#define NTLM_FLAG_UNICODE               0x00000001  /* unicode charset */
#define NTLM_FLAG_OEM                   0x00000002  /* oem charset */
#define NTLM_FLAG_REQUEST_TARGET        0x00000004  /* ret trgt in challenge */
#define NTLM_FLAG_SIGN                  0x00000010  /* sign requested */
#define NTLM_FLAG_SEAL                  0x00000020  /* encryption requested */
#define NTLM_FLAG_DATAGRAM              0x00000040  /* udp message */
#define NTLM_FLAG_LM_KEY                0x00000080  /* use LM key for crypto */
#define NTLM_FLAG_NETWARE               0x00000100  /* netware - unsupported */
#define NTLM_FLAG_NTLM                  0x00000200  /* use NTLM auth */
#define NTLM_FLAG_DOMAIN                0x00001000  /* domain supplied */
#define NTLM_FLAG_WORKSTATION           0x00002000  /* wks supplied */
#define NTLM_FLAG_LOCAL_CALL            0x00004000  /* loopback auth */
#define NTLM_FLAG_ALWAYS_SIGN           0x00008000  /* use dummy sig */
#define NTLM_FLAG_TYPE_DOMAIN           0x00010000  /* domain authenticator */
#define NTLM_FLAG_TYPE_SERVER           0x00020000  /* server authenticator */
#define NTLM_FLAG_TYPE_SHARE            0x00040000  /* share authenticator */
#define NTLM_FLAG_NTLM2                 0x00080000  /* use NTLMv2 key */
#define NTLM_FLAG_INIT_RESPONSE         0x00100000  /* unknown */
#define NTLM_FLAG_ACCEPT_RESPONSE       0x00200000  /* unknown */
#define NTLM_FLAG_NON_NT_SESSION_KEY    0x00400000  /* unknown */
#define NTLM_FLAG_TARGET_INFO           0x00800000  /* target info used */
#define NTLM_FLAG_UNKNOWN_02000000      0x02000000  /* needed, for what? */
#define NTLM_FLAG_128                   0x20000000  /* 128-bit encryption */
#define NTLM_FLAG_KEY_EXCH              0x40000000  /* perform key exchange */
#define NTLM_FLAG_56                    0x80000000  /* 56-bit encryption */

#define NTLM_FLAG_NEGOTIATE_DEFAULT ( \
    NTLM_FLAG_OEM                   | \
    NTLM_FLAG_REQUEST_TARGET        | \
    NTLM_FLAG_NTLM                  | \
    NTLM_FLAG_DOMAIN                | \
    NTLM_FLAG_56                    )
    //NTLM_FLAG_128                   |
    //NTLM_FLAG_NTLM2                 |
    //NTLM_FLAG_UNICODE               |

#define NTLM_FLAG_SRV_SUPPORTS ( \
    NTLM_FLAG_OEM              | \
    NTLM_FLAG_REQUEST_TARGET   | \
    NTLM_FLAG_NTLM             | \
    NTLM_FLAG_LOCAL_CALL       | \
    NTLM_FLAG_ALWAYS_SIGN      | \
    NTLM_FLAG_WORKSTATION      | \
    NTLM_FLAG_TARGET_INFO      | \
    NTLM_FLAG_56               )
    //NTLM_FLAG_128              |
    //NTLM_FLAG_NTLM2            |
    //NTLM_FLAG_UNICODE          |

// Possible information to query our context for
#define SECPKG_ATTR_ACCESS_TOKEN                1
#define SECPKG_ATTR_AUTHORITY                   2
#define SECPKG_ATTR_CLIENT_SPECIFIED_TARGET     3
#define SECPKG_ATTR_DCE_INFO                    4
#define SECPKG_ATTR_FLAGS                       5
#define SECPKG_ATTR_KEY_INFO                    6
#define SECPKG_ATTR_LAST_CLIENT_TOKEN_STATUS    7
#define SECPKG_ATTR_LIFESPAN                    8
#define SECPKG_ATTR_LOCAL_CRED                  9
#define SECPKG_ATTR_NAMES                       10
#define SECPKG_ATTR_NATIVE_NAMES                11
#define SECPKG_ATTR_NEGOTIATION_INFO            12
#define SECPKG_ATTR_PACKAGE_INFO                13
#define SECPKG_ATTR_PASSWORD_EXPIRY             14
#define SECPKG_ATTR_ROOT_STORE                  15
#define SECPKG_ATTR_SESSION_KEY                 16
#define SECPKG_ATTR_SIZES                       17
#define SECPKG_ATTR_TARGET_INFORMATION          18

#define SECPKG_CRED_ATTR_NAMES                  19
#define SECPKG_ATTR_SUPPORTED_ALGS              20
#define SECPKG_ATTR_CIPHER_STRENGTHS            21
#define SECPKG_ATTR_SUPPORTED_PROTOCOLS         22

//******************************************************************************
//
// E X T E R N S
//

//******************************************************************************
//
// P R O T O T Y P E S
//

DWORD
NtlmClientAcceptSecurityContext(
    IN PNTLM_CRED_HANDLE pCredential,
    IN OUT PNTLM_CONTEXT_HANDLE phContext,
    IN PSecBufferDesc pInput,
    IN DWORD fContextReq,
    IN DWORD TargetDataRep,
    IN OUT PNTLM_CONTEXT_HANDLE phNewContext,
    IN OUT PSecBufferDesc pOutput,
    OUT PDWORD  pfContextAttr,
    OUT PTimeStamp ptsTimeStamp
    );

DWORD
NtlmClientAcquireCredentialsHandle(
    IN SEC_CHAR *pszPrincipal,
    IN SEC_CHAR *pszPackage,
    IN DWORD fCredentialUse,
    IN PLUID pvLogonID,
    IN PVOID pAuthData,
    OUT PNTLM_CRED_HANDLE pCredential,
    OUT PTimeStamp ptsExpiry
    );

DWORD
NtlmClientDecryptMessage(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN OUT PSecBufferDesc pMessage,
    IN DWORD MessageSeqNo,
    OUT PBOOLEAN pbEncrypted
    );

DWORD
NtlmClientDeleteSecurityContext(
    IN PNTLM_CONTEXT_HANDLE phContext
    );

DWORD
NtlmClientEncryptMessage(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN BOOLEAN bEncrypt,
    IN OUT PSecBufferDesc pMessage,
    IN DWORD MessageSeqNo
    );

DWORD
NtlmClientExportSecurityContext(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN DWORD fFlags,
    OUT PSecBuffer pPackedContext,
    OUT OPTIONAL HANDLE *pToken
    );

DWORD
NtlmClientFreeCredentialsHandle(
    IN PNTLM_CRED_HANDLE pCredential
    );

DWORD
NtlmClientImportSecurityContext(
    IN PSECURITY_STRING *pszPackage,
    IN PSecBuffer pPackedContext,
    IN OPTIONAL HANDLE pToken,
    OUT PNTLM_CONTEXT_HANDLE phContext
    );

DWORD
NtlmClientInitializeSecurityContext(
    IN OPTIONAL PNTLM_CRED_HANDLE phCredential,
    IN OPTIONAL PNTLM_CONTEXT_HANDLE phContext,
    IN OPTIONAL SEC_CHAR * pszTargetName,
    IN DWORD fContextReq,
    IN DWORD Reserved1,
    IN DWORD TargetDataRep,
    IN OPTIONAL PSecBufferDesc pInput,
    IN DWORD Reserved2,
    IN OUT OPTIONAL PNTLM_CONTEXT_HANDLE phNewContext,
    IN OUT OPTIONAL PSecBufferDesc pOutput,
    OUT PDWORD pfContextAttr,
    OUT OPTIONAL PTimeStamp ptsExpiry
    );

DWORD
NtlmClientMakeSignature(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN BOOLEAN bEncrypt,
    IN OUT PSecBufferDesc pMessage,
    IN DWORD MessageSeqNo
    );

DWORD
NtlmClientQueryCredentialsAttributes(
    IN PNTLM_CRED_HANDLE phCredential,
    IN DWORD ulAttribute,
    OUT PVOID pBuffer
    );

DWORD
NtlmClientQueryContextAttributes(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN DWORD ulAttribute,
    OUT PVOID pBuffer
    );

DWORD
NtlmClientVerifySignature(
    IN PNTLM_CONTEXT_HANDLE phContext,
    IN PSecBufferDesc pMessage,
    IN DWORD MessageSeqNo,
    OUT PBOOLEAN pbVerified,
    OUT PBOOLEAN pbEncryted
    );

DWORD
NtlmFreeContextBuffer(
    IN PVOID pBuffer
    );

#endif // __SSPINTLM_H__

/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
