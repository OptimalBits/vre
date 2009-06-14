/*
 *  vre_dynbuffer.h
 *  
 *
 *  Created by Manuel Astudillo on 2007-11-27.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef VRE_DYNBUFFER_H
#define VRE_DYNBUFFER_H

#include "vre_defs.h"

typedef struct vre_dynbuffer vre_dynbuffer;

vre_result vre_dynbuffer_create ( vre_dynbuffer **ppBuffer,
                                  vre_uint32 elem_size,
                                  vre_uint32 inc_size );

void vre_dynbuffer_destroy ( vre_dynbuffer *pBuffer );

/**
    Adds a buffer of num_elems of size elem_size.
 
 */
vre_result vre_dynbuffer_add ( vre_dynbuffer *pBuffer, 
                               void *pData,
                               vre_uint32 num_elems );

vre_uint32 vre_dynbuffer_num_elems ( vre_dynbuffer *pBuffer );

void vre_dynbuffer_get_elems_start ( vre_dynbuffer *pBuffer,
                                     void **ppData, vre_uint32 *num_elems );

/**
    Returns a chunk of data.
 
 */
void vre_dynbuffer_get_elems_iter ( vre_dynbuffer *pBuffer, 
                                   void **ppData, vre_uint32 *num_elems );


/**
  Copies a block of data

 */
void vre_dynbuffer_copy ( vre_dynbuffer *pBuffer,
                          vre_uint32 start, 
                          vre_uint32 num_elems,
                          void *pDst );


#endif


