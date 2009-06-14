/*
 *  vre_rectangle.h
 *  vre
 *
 *  Created by Manuel Astudillo on 2006-03-30.
 *  Copyright 2006 Company AB. All rights reserved.
 *
 */

#ifndef VRE_RECTANGLE_TYPE
#define VRE_RECTANGLE_TYPE

typedef struct vre_rectangle vre_rectangle;

#endif

#ifndef VRE_RECTANGLE_H
#define VRE_RECTANGLE_H

#include "vre_defs.h"
#include "vre_style.h"
#include "vre_matrix.h"
#include "vre_polygon.h"

struct vre_rectangle
{
    vre_fix16   x;
    vre_fix16   y;
    vre_ufix16  w;
    vre_ufix16  h;
    
    vre_fix16   x1;
    vre_fix16   y1;
    vre_fix16   x2;
    vre_fix16   y2;
};


typedef enum vre_intersection
{
    VRE_INTERSECT_DISJOINT,
    VRE_INTERSECT_NOT_DISJOINT
} vre_intersection;

void vre_rectangle_copy ( vre_rectangle *pDst,
                          vre_rectangle *pSrc );

vre_result vre_rectangle_to_polygon ( vre_rectangle *pRectangle,
                                      vre_style *pStyle,
                                      vre_mat3x3 *pMatrix,
                                      vre_polygon *pPolygon );

vre_bool vre_rectangle_are_intersecting ( vre_rectangle *pRectA, 
                                          vre_rectangle *pRectB );

vre_intersection vre_rectangle_intersection ( vre_rectangle *pRectA, 
                                              vre_rectangle *pRectB,
                                              vre_rectangle *pRectOut );

void vre_rectangle_union ( vre_rectangle *pRectA, 
                           vre_rectangle *pRectB,
                           vre_rectangle *pRectOut );


vre_uint32 vre_rectangle_area ( vre_rectangle *pRect );


#endif
