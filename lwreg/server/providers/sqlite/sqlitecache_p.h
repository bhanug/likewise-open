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
 *        sqlitecache_p.h
 *
 * Abstract:
 *
 *        Registry Sqlite Cache private header
 *
 *        Inter-process communication (Server) API for Users
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 *          Wei Fu (wfu@likewise.com)
 */

#ifndef __SQLITECACHE_P_H_
#define __SQLITECACHE_P_H_


PREG_KEY_CONTEXT
SqliteCacheLocateActiveKey(
    IN PCSTR pszKeyName
    );

PREG_KEY_CONTEXT
SqliteCacheLocateActiveKey_inlock(
    IN PCSTR pszKeyName
    );

NTSTATUS
SqliteCacheInsertActiveKey(
    IN PREG_KEY_CONTEXT pKeyResult
    );

NTSTATUS
SqliteCacheInsertActiveKey_inlock(
    IN PREG_KEY_CONTEXT pKeyResult
    );

VOID
SqliteCacheDeleteActiveKey(
    IN PSTR pszKeyName
    );

VOID
SqliteCacheDeleteActiveKey_inlock(
    IN PSTR pszKeyName
    );

void
SqliteCacheResetParentKeySubKeyInfo(
    IN PSTR pszParentKeyName
    );

void
SqliteCacheResetParentKeySubKeyInfo_inlock(
    IN PSTR pszParentKeyName
    );

void
SqliteCacheResetKeyValueInfo(
    IN PSTR pszKeyName
    );

void
SqliteCacheResetKeyValueInfo_inlock(
    IN PSTR pszKeyName
    );

VOID
SqliteCacheReleaseKey(
    PREG_KEY_CONTEXT pKeyResult
    );

VOID
SqliteCacheReleaseKey_inlock(
    PREG_KEY_CONTEXT pKeyResult
    );

void
SqliteCacheFreeHashEntry(
    IN const REG_HASH_ENTRY* pEntry
    );

NTSTATUS
SqliteCacheSubKeysInfo_inlock(
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi
    );

NTSTATUS
SqliteCacheSubKeysInfo(
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi
    );

NTSTATUS
SqliteCacheUpdateSubKeysInfo_inlock(
    IN DWORD dwOffSet,
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi,
    OUT size_t* psNumSubKeys
    );

NTSTATUS
SqliteCacheUpdateSubKeysInfo(
    IN DWORD dwOffSet,
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi,
    OUT size_t* psNumSubKeys
    );

NTSTATUS
SqliteCacheKeyValuesInfo_inlock(
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi
    );

NTSTATUS
SqliteCacheKeyValuesInfo(
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi
    );

NTSTATUS
SqliteCacheUpdateValuesInfo_inlock(
    DWORD dwOffSet,
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi,
    OUT size_t* psNumValues
    );

NTSTATUS
SqliteCacheUpdateValuesInfo(
    DWORD dwOffSet,
    IN OUT PREG_KEY_CONTEXT pKeyResult,
    IN BOOLEAN bDoAnsi,
    OUT size_t* psNumValues
    );


#endif // __SQLITECACHE__P_H_
