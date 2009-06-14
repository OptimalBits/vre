
#ifndef VRE_ARRAY_H
#define VRE_ARRAY_H

#include "vre_defs.h"

typedef struct vre_array vre_array;


/**
    Array of elements of the same size.
    
 */
vre_result vre_array_create ( vre_uint32 inc_size,
                              vre_array **ppArray );

/**
 
 
 */
void vre_array_destroy ( vre_array *pArray );

/**
 
 */
vre_result vre_array_add ( vre_array *pArray, 
                           void *pData,
                           vre_uint32 length );

/**
 
 */
vre_uint32 vre_array_num_elems ( vre_array *pArray );

// void *vre_array_get_ptr ( vre_array *pArray );
/**
    Iterate across the elements.
 
 */
void vre_array_get_elems_start ( vre_array *pArray,
                                 void **ppData, 
                                 vre_uint32 *length );

void vre_array_get_elems_iter ( vre_array *pArray, 
                                void **ppData, 
                                vre_uint32 *length );

void vre_array_copy_to_buffer ( vre_array *pArray,
                                void *pDst,
                                vre_uint32 start,
                                vre_uint32 len );
/**
 
 New proposed API:
 
 vre_result vre_array_add ( vre_array *pArray, void *pData );
 
 vre_result vre_array_extend ( vre_array *pArray, void *pData, vre_uint32 num_elems );
 
 void vre_array_get_elems_start ( vre_array *pArray, void **ppData );
 
 void vre_array_get_elems_iter ( vre_array *pArray, void **ppData );
 
 void vre_array_get_chunks_start ( vre_array *pArray,
                                   void **pChunk,
                                   vre_uint32 *length );
 
 void vre_array_get_chunks_iter ( vre_array *pArray,
                                  void **pChunk,
                                  vre_uint32 *length );
 
 */





#endif


