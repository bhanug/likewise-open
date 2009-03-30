#include "includes.h"

static
int
SamDbCompareAttributeLookupKeys(
    PVOID pKey1,
    PVOID pKey2
    );

DWORD
SamDbAttributeLookupInitContents(
    PSAM_DB_ATTR_LOOKUP   pAttrLookup,
    PSAM_DB_ATTRIBUTE_MAP pAttrMap,
    DWORD                 dwNumMaps
    )
{
    DWORD dwError = 0;
    PLWRTL_RB_TREE pLookupTable = NULL;
    DWORD iAttr = 0;

    memset(pAttrLookup, 0, sizeof(SAM_DB_ATTR_LOOKUP));

    dwError = LwRtlRBTreeCreate(
                    &SamDbCompareAttributeLookupKeys,
                    NULL,
                    NULL,
                    &pLookupTable);
    BAIL_ON_SAMDB_ERROR(dwError);

    for (; iAttr < dwNumMaps; iAttr++)
    {
        PSAM_DB_ATTRIBUTE_MAP pMap = &pAttrMap[iAttr];

        dwError = LwRtlRBTreeAdd(
                       pLookupTable,
                       pMap->wszDirectoryAttribute,
                       pMap);
        BAIL_ON_SAMDB_ERROR(dwError);
    }

    pAttrLookup->pLookupTable = pLookupTable;

cleanup:

    return dwError;

error:

    if (pLookupTable)
    {
        LwRtlRBTreeFree(pLookupTable);
    }

    goto cleanup;
}

VOID
SamDbAttributeLookupFreeContents(
    PSAM_DB_ATTR_LOOKUP pAttrLookup
    )
{
    if (pAttrLookup->pLookupTable)
    {
        LwRtlRBTreeFree(pAttrLookup->pLookupTable);
        pAttrLookup->pLookupTable = NULL;
    }
}

static
int
SamDbCompareAttributeLookupKeys(
    PVOID pKey1,
    PVOID pKey2
    )
{
    PWSTR pwszKey1 = (PWSTR)pKey1;
    PWSTR pwszKey2 = (PWSTR)pKey2;

    return wc16scasecmp(pwszKey1, pwszKey2);
}
