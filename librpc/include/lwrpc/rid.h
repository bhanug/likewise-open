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
 * Abstract: RID related type definitions (rpc client library)
 *
 * Authors: Rafal Szczesniak (rafal@likewisesoftware.com)
 */


#ifndef _RID_H_
#define _RID_H_

typedef struct ids {
#ifdef _DCE_IDL_
	[range(0,1024)]
#endif
    UINT32 count;
#ifdef _DCE_IDL_
	[size_is(count)]
#endif
    UINT32 *ids;
} Ids;


typedef struct rid_with_attribute {
    UINT32 rid;
    UINT32 attributes;
} RidWithAttribute;


typedef struct rid_with_attribute_array {
    UINT32     count;
#ifdef _DCE_IDL_
    [size_is(count)]
#endif
    RidWithAttribute *rids;
} RidWithAttributeArray;


typedef struct rid_name {
    UINT32 rid;
    UnicodeString name;
} RidName;


typedef struct rid_name_array {
    UINT32 count;
#ifdef _DCE_IDL_
    [size_is(count)]
#endif
    RidName *entries;
} RidNameArray;


#endif /* _RID_H_ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
