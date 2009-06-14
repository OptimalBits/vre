

#ifndef VRE_BEZIER_H
#define VRE_BEZIER_H

#include "vre_defs.h"
#include "vre_polygon.h"

typedef struct vre_bezier vre_bezier;

typedef vre_uint32 (*VRE_BEZIER_MAP_FUNC) ( void* pUser, vre_point p0 );

vre_result vre_bezier_create ( vre_bezier **ppBezier, 
                               vre_uint32 num_points );

void vre_bezier_destroy ( vre_bezier *pBezier );

void vre_bezier_set_points ( vre_bezier *pBezier,
                             vre_point *pPoints,
                             vre_uint32 num_points );

void vre_bezier_set_map_func ( vre_bezier *pBezier, 
                               VRE_BEZIER_MAP_FUNC pFunc, 
                               void *pUser );
                                                                                          
vre_result vre_bezier_flatten ( vre_bezier *pBezier, 
                                vre_polygon *pPolygon, 
                                vre_fix16 max_deviation,
                                vre_bool include_last_point);                                                              
                               
#endif
