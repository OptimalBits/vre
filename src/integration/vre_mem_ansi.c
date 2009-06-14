
#include "vre_mem.h"
#include <stdlib.h>

typedef struct vre_mem_debug
{
    vre_uint32 alloc_bytes;
} vre_mem_debug;

static vre_uint32 mem_used = 0;
static vre_uint32 mem_peak = 0;

void *vre_malloc (vre_uint32 num_bytes)
{
    vre_uint8 *pMemBlock;
    
    mem_used += num_bytes;
    
    if ( mem_used > mem_peak )
    {
        mem_peak = mem_used;
    }
    
    pMemBlock = malloc (num_bytes + sizeof ( vre_mem_debug) );
    
    *((vre_uint32*) pMemBlock) = num_bytes;
    
    pMemBlock += sizeof(vre_mem_debug);
    
    return pMemBlock;
}

void vre_free (void *p)
{
    vre_uint8 *pPtr8 = p;
    
    if ( p != 0 )
    {
        pPtr8 -=  sizeof ( vre_mem_debug );
    
        mem_used -= *((vre_uint32*)(pPtr8));
    
        free (pPtr8);
    }
}

vre_uint32 vre_mem_peak ()
{
    return mem_peak;
}

