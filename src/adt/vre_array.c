/*
 *  vre_array.c
 *  vre_test
 *
 *  Created by Manuel Astudillo on 2006-05-02.
 *  Copyright 2006 Curved Graphics. All rights reserved.
 *
 */

#include "vre_array.h"
#include "vre_mem.h"
#include "vre_assert.h"


typedef struct vre_array_chunk vre_array_chunk;

struct vre_array_chunk
{
    void *pData;
    vre_uint32 length;
    vre_array_chunk *pNext;
};

struct vre_array
{
    vre_array_chunk *pFirst;
    vre_array_chunk *pCurrent;
    vre_array_chunk *pIterator;
    
    vre_uint32 length;
    vre_uint32 chunk_size;
};

static vre_result create_chunk ( vre_uint32 size, vre_array_chunk **ppChunk );

vre_result vre_array_create ( vre_uint32 inc_size,
                             vre_array **ppArray )
{
    vre_result vres = VRE_ERR_OK;
 
    vre_array *pArray;
        
    vre_assert ( inc_size > 0 );
    
    *ppArray = 0;
    
    pArray = VRE_TALLOC ( vre_array );
    if ( pArray == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    //
    // Alloc memory for first chunk
    //
    vres = create_chunk ( inc_size, &(pArray->pFirst) );
    if ( vres != VRE_ERR_OK )
    {
        vre_free( pArray );
        return vres;
    }
    
    pArray->pCurrent = pArray->pFirst;
    
    pArray->chunk_size = inc_size;
    pArray->length = 0;
    
    *ppArray = pArray;
    
    return vres;
}

void vre_array_destroy ( vre_array *pArray )
{
    vre_array_chunk *pChunk;
    vre_array_chunk *pTempChunk;
    
    if ( pArray != 0 )
    {
        pChunk = pArray->pFirst;
        
        while ( pChunk != 0 )
        {
            pTempChunk = pChunk;
            pChunk = pChunk->pNext;
            vre_free ( pTempChunk );
        } while ( pChunk != 0 );
    
        vre_free( pArray );
    }
}

vre_result vre_array_add ( vre_array *pArray, 
                           void *pData,
                           vre_uint32 length )
{
    vre_result vres;
    vre_uint32 bytes_left;

    vre_array_chunk *pChunk;
    vre_uint8 *pDataPtr;
    vre_uint8 *pSrcData;
    
    vre_assert ( pArray != 0 );
    vre_assert ( pArray->pFirst != 0 );
    vre_assert ( length <= pArray->chunk_size );
    
    pSrcData = ( vre_uint8*) pData;
    
    pChunk = pArray->pCurrent;
    pDataPtr = ((vre_uint8*)pChunk->pData) + pChunk->length;
    
    bytes_left = pArray->chunk_size -  pChunk->length;
    
    if ( bytes_left >= length )
    {
        vre_memcopy8 ( pDataPtr, pSrcData, length );
        pChunk->length += length;
        pArray->length += length;
    }
    else
    {
        vre_array_chunk *pNewChunk;
        
        vre_assert ( pChunk->length + bytes_left == pArray->chunk_size );
        vre_memcopy8 ( pDataPtr, pSrcData, bytes_left );
        
        length -= bytes_left;
        
        pChunk->length = pArray->chunk_size;

        pArray->length += bytes_left;
        pSrcData += bytes_left;
        
        vres = create_chunk ( pArray->chunk_size, &pNewChunk );
        if ( vres != VRE_ERR_OK )
        {
            return vres;
        }
        
        pChunk->pNext = pNewChunk;
        vre_memcopy8 ( pNewChunk->pData, pSrcData, length );
        
        pNewChunk->length += length;
        pArray->length += length;
        
        pArray->pCurrent = pNewChunk;
    }
    
    return VRE_ERR_OK;
}

vre_uint32 vre_array_length ( vre_array *pArray )
{
    vre_assert ( pArray != 0 );
        
    return pArray->length; 
}

void vre_array_get_elems_start ( vre_array *pArray,
                                void **ppData, 
                                vre_uint32 *pLength )
{
    if ( pArray->pFirst )
    {
        *ppData = pArray->pFirst->pData;
        *pLength = pArray->pFirst->length;
        pArray->pIterator = pArray->pFirst->pNext;
    }
    else
    {
        *ppData = 0;
        *pLength = 0;
    }
}

void vre_array_get_elems_iter ( vre_array *pArray, 
                               void **ppData, 
                               vre_uint32 *pLength )
{
    if ( pArray->pIterator != 0 )
    {
        *ppData = pArray->pIterator->pData;
        *pLength = pArray->pIterator->length;
        pArray->pIterator = pArray->pIterator->pNext;
    }
    else
    {
        *ppData = 0;
        *pLength = 0;
    }
}

void vre_array_copy_to_buffer ( vre_array *pArray,
                               void *pDst,
                               vre_uint32 start,
                               vre_uint32 len )
{
    // Implement.
    
    
}

static vre_result create_chunk ( vre_uint32 size, vre_array_chunk **ppChunk )
{
    vre_result vres;
    vre_array_chunk *pChunk;
    
    pChunk = vre_malloc( size + sizeof ( vre_array_chunk ) );
    if ( pChunk == 0 )
    {
        vres = VRE_ERR_MEM_ALLOC_FAILED;
    }
    else
    {   
        pChunk->length = 0;
        pChunk->pNext = 0;
        pChunk->pData = ((vre_uint8*) pChunk) + sizeof ( vre_array_chunk );
        vres = VRE_ERR_OK;
    }
    
    *ppChunk = pChunk;
    
    return vres;
}







