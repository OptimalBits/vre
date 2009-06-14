/*
 *  vre_dynbuffer.c
 *  
 *
 *  Created by Manuel Astudillo on 2007-11-27.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_dynbuffer.h"
#include "vre_defs.h"

typedef struct vre_dynbuffer_chunk vre_dynbuffer_chunk;

struct vre_dynbuffer_chunk
{
    void *pData;
    vre_uint32 length;
    vre_dynbuffer_chunk *pNext;
};

struct vre_dynbuffer
{
    vre_uint32 elem_size;
    vre_uint32 inc_size;
    
    vre_dynbuffer_chunk *pFirstChunk;
    vre_dynbuffer_chunk *pCurrent;
    
};

vre_result vre_dynbuffer_create ( vre_dynbuffer **ppBuffer,
                                 vre_uint32 elem_size,
                                 vre_uint32 inc_size )
{
    vre_result vres;
    
    for (;;)
    {
        
        
    }
}

void vre_dynbuffer_destroy ( vre_dynbuffer *pBuffer );

/**
 Adds a buffer of num_elems of size elem_size.
 
 */
vre_result vre_dynbuffer_add ( vre_dynbuffer *pBuffer, 
                              void *pData,
                              vre_uint32 num_elems );

vre_uint32 vre_dynbuffer_num_elems ( vre_dynbuffer *pBuffer );


void vre_dynbuffer_get_elems_start ( vre_dynbuffer *pBuffer,
                                    void **ppData, vre_uint32 *num_elems )
{
    
    
}

/**
 Returns a chunk of data.
 
 */
void vre_dynbuffer_get_elems_iter ( vre_dynbuffer *pBuffer, 
                                   void **ppData, vre_uint32 *num_elems )
{
    
}



