/*
 *  vre_rectangle.c
 *  vre
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_rectangle.h"
#include "vre_math.h"
#include "vre_assert.h"
#include "vre_mem.h"


void vre_rectangle_copy ( vre_rectangle *pDst, vre_rectangle *pSrc )
{
    vre_memcopy8 ( pDst, pSrc, sizeof( vre_rectangle ) );
}

vre_bool vre_rectangle_are_intersecting ( vre_rectangle *pRectA, 
                                          vre_rectangle *pRectB )
{
    vre_rectangle tmp_rect;
    
    tmp_rect.x1 = pRectA->x1 > pRectB->x1 ? pRectA->x1 : pRectB->x1;
    tmp_rect.y1 = pRectA->y1 > pRectB->y1 ? pRectA->y1 : pRectB->y1;
    
    tmp_rect.x2 = pRectA->x2 < pRectB->x2 ? pRectA->x2 : pRectB->x2;
    tmp_rect.y2 = pRectA->y2 < pRectB->y2 ? pRectA->y2 : pRectB->y2;
    
    if ( ( tmp_rect.x1 > tmp_rect.x2 ) || ( tmp_rect.y1 > tmp_rect.y2 ) )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


vre_intersection vre_rectangle_intersection ( vre_rectangle *pRectA, 
                                              vre_rectangle *pRectB,
                                              vre_rectangle *pRectOut )
{
    vre_rectangle tmp_rect;
    
    tmp_rect.x1 = pRectA->x1 > pRectB->x1 ? pRectA->x1 : pRectB->x1;
    tmp_rect.y1 = pRectA->y1 > pRectB->y1 ? pRectA->y1 : pRectB->y1;
    
    tmp_rect.x2 = pRectA->x2 < pRectB->x2 ? pRectA->x2 : pRectB->x2;
    tmp_rect.y2 = pRectA->y2 < pRectB->y2 ? pRectA->y2 : pRectB->y2;
    
    if ( ( tmp_rect.x1 > tmp_rect.x2 ) || ( tmp_rect.y1 > tmp_rect.y2 ) )
    {
        return VRE_INTERSECT_DISJOINT;
    }
    else
    {
        pRectOut->x1 = tmp_rect.x1;
        pRectOut->y1 = tmp_rect.y1;
        pRectOut->x2 = tmp_rect.x2;
        pRectOut->y2 = tmp_rect.y2;
        
       return VRE_INTERSECT_NOT_DISJOINT;
    }
}


void vre_rectangle_union ( vre_rectangle *pRectA, 
                           vre_rectangle *pRectB,
                           vre_rectangle *pRectOut )
{
    vre_rectangle tmp_rect;
    
    tmp_rect.x1 = VRE_MIN (pRectA->x1, pRectB->x1 );
    tmp_rect.y1 = VRE_MIN (pRectA->y1, pRectB->y1 );
    
    tmp_rect.x2 = VRE_MAX (pRectA->x2, pRectB->x2 );
    tmp_rect.y2 = VRE_MAX (pRectA->y2, pRectB->y2 );
    
    pRectOut->x1 = tmp_rect.x1;
    pRectOut->y1 = tmp_rect.y1;
    pRectOut->x2 = tmp_rect.x2;
    pRectOut->y2 = tmp_rect.y2;
}

vre_uint32 vre_rectangle_area ( vre_rectangle *pRect )
{
    vre_uint32 width;
    vre_uint32 height;
    vre_uint32 area;
    
    vre_assert ( pRect->x2 >= pRect->x1 );
    vre_assert ( pRect->y2 >= pRect->y1 );
    
    width  = (pRect->x2 - pRect->x1)>>8;
    height = (pRect->y2 - pRect->y1)>>8;
    
    area = (vre_uint32) ((((vre_uint64)width) * ((vre_uint64)height))>>16);
    
    return area;
}


