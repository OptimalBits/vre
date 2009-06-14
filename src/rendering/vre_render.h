/*
 *  vre_render.h
 *  vre_test
 *
 *  Created by Manuel  Astudillo on 2006-04-20.
 *  Copyright 2006 Polygoniq. All rights reserved.
 *
 */

#ifndef VRE_RENDER_H
#define VRE_RENDER_H

typedef struct vre_render vre_render;

#include "vre_defs.h"
#include "vre_polygon.h"

#define VRE_DEFAULT_NUM_POINTS 2048

vre_result vre_render_create ( vre_render **ppRender, 
                               vre_uint32 max_width );

void vre_render_destroy ( vre_render *pRenderer );

vre_result vre_render_realloc ( vre_render *pRender, vre_uint32 num_points );

/*
void vre_polygon_convex ( vre_context *pContext, vre_point *pPoints, 
                          vre_uint num_points, vre_uint first_point);
*/

vre_result vre_draw_polygon_concave_aa ( vre_render *pRenderer,
                                         vre_polygon *pPoly,
                                         vre_tile *pTile,
                                         vre_style *pStyle );

#endif

