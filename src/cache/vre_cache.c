/*
 *  vre_cache.c
 *  vre_test
 *
 *  Created by Manuel  Astudillo on 2006-06-08.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_cache.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"

typedef struct vre_cache_entry
{
    vre_uint32 id;
    void *pData;
} vre_cache_entry;

struct vre_cache
{
    vre_cache_entry *pEntries;
    vre_uint32 num_entries;
    vre_uint32 mask;
    vre_uint32 iter_pos;
};

vre_result vre_cache_create ( vre_cache **ppCache, vre_uint32 num_entries )
{
    vre_cache *pCache;
    
    vre_assert ( VRE_IS_POWER_OF_2 (num_entries) );
    
    *ppCache = 0;
    
    pCache = ( vre_cache*) vre_malloc( sizeof ( vre_cache ));
    if ( pCache == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pCache->pEntries = ( vre_cache_entry*) 
        vre_malloc( sizeof ( vre_cache_entry ) * num_entries );
    
    if ( pCache->pEntries == 0 )
    {
        vre_free( pCache );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset (pCache->pEntries, num_entries * sizeof ( vre_cache_entry ), 0 ); 
    
    pCache->num_entries = num_entries;
    pCache->mask = num_entries - 1;
    pCache->iter_pos = 0;
    
    *ppCache = pCache;
    
    return VRE_ERR_OK;
}

void vre_cache_destroy ( vre_cache *pCache )
{
    if ( pCache )
    {
        vre_free ( pCache->pEntries );
        vre_free ( pCache );
    }
}

void vre_cache_query ( vre_cache *pCache, vre_uint32 id, void **ppData )
{
    vre_uint32 index = id & pCache->mask;
    
    if ( pCache->pEntries[index].id == id )
    {
        *ppData = pCache->pEntries[index].pData;
    }
    else
    {
        *ppData = 0;
    }
}

void vre_cache_insert ( vre_cache *pCache, vre_uint32 id, 
                        void *pDataIn, 
                        void  **ppDataOut)
{
    vre_uint32 index = id & pCache->mask;
    
    pCache->pEntries[index].id = id;
    
    *ppDataOut = pCache->pEntries[index].pData;
    pCache->pEntries[index].pData = pDataIn;
}

void vre_cache_begin_iter ( vre_cache *pCache )
{
    pCache->iter_pos = 0;
}

vre_result vre_cache_iter ( vre_cache *pCache, void **ppData )
{
    while ( ( pCache->iter_pos < pCache->num_entries ) && 
            ( pCache->pEntries[pCache->iter_pos].pData == 0 ) )
    {
        pCache->iter_pos++;
    }
    
    if ( pCache->iter_pos < pCache->num_entries )
    {
        *ppData = pCache->pEntries[pCache->iter_pos].pData;
        pCache->iter_pos++;
        return VRE_ERR_OK;
    }
    else
    {
        return VRE_ERR_OK;
    }
}



