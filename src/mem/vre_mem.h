

#ifndef VRE_MEM_H
#define VRE_MEM_H

#include "vre_defs.h"

#define VRE_TALLOC(type) ((type*) vre_malloc ( sizeof ( type ) ))

//
// Integration Layer
//
void *vre_malloc (vre_uint32 num_bytes);
void vre_free (void *p);


//
// Platform Independent Functions
//
void vre_memcopy8  ( void *pDst, void *pSrc, vre_uint32 bytes );
void vre_memcopy16 ( void *pDst, void *pSrc, vre_uint32 bytes );
void vre_memcopy32 ( void *pDst, void *pSrc, vre_uint32 bytes );

void vre_memset ( void *pDst, vre_uint32 size, vre_uint8 val );


//
// Debug functions
//

vre_uint32 vre_mem_peak ();

#endif
