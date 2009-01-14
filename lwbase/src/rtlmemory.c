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

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        rtlmemory.c
 *
 * Abstract:
 *
 *        Likewise Memory Utilities
 *
 * Authors: Danilo Almeida (dalmeida@likewise.com)
 */

#include "includes.h"

VOID
LwRtlMemoryZero(
    IN OUT LW_PVOID pMemory,
    IN size_t Size
    )
{
    memset(pMemory, 0, Size);
}

PVOID
LwRtlMemoryAllocate(
    IN size_t Size
    )
{
    PVOID pMemory = NULL;

    // TODO-Document behavior for Size == 0.
    assert(Size > 0);

    // Note -- If this allocator changes, need to change iostring routines.
    pMemory = malloc(Size);
    if (pMemory)
    {
        memset(pMemory, 0, Size);
    }

    return pMemory;
}

PVOID
LwRtlMemoryRealloc(
    LW_IN LW_PVOID pMemory,
    LW_IN size_t Size
    )
{
    assert(Size > 0);

    return realloc(pMemory, Size);
}

VOID
LwRtlMemoryFree(
    IN OUT LW_PVOID pMemory
    )
{
    assert(pMemory);
    free(pMemory);
}
