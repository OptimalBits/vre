
#include "vre_mem.h"
#include <stdlib.h>
//
// TODO: Implement using duff devices.
//
void vre_memcopy8 ( void *pDst, void *pSrc, vre_uint32 bytes )
{
	vre_uint32 i;
    vre_uint8* pDst8 = (vre_uint8*) pDst;
    vre_uint8* pSrc8 = (vre_uint8*) pSrc;
	
	for ( i = bytes; i != 0; i-- )
	{
        *pDst8 = *pSrc8;
        pDst8++;
        pSrc8++;
	}
}

void vre_memcopy16 ( void *pDst, void *pSrc, vre_uint32 bytes )
{
	
}

void vre_memcopy32 ( void *pDst, void *pSrc, vre_uint32 bytes )
{
	
}


void vre_memset ( void *pDst, vre_uint32 size, vre_uint8 val )
{
    /*
    vre_uint32 i;
	
	for ( i = 0; i < size; i++ )
	{
		((vre_uint8*)pDst)[i] = val;
	}
    */
    memset (pDst, val, size);
}
