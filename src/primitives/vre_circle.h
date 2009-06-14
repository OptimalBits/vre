
/*
check this: 
http://groups.google.com/group/comp.graphics.algorithms/browse_thread/thread/70f423a4904cf868/564f7d9472639595?q=circle+flattening&rnum=3#564f7d9472639595
*/

#ifndef VRE_CIRCLE_H
#define VRE_CIRCLE_H

#include "vre_defs.h"
#include "vre_polygon.h"

typedef struct vre_circle vre_circle;

vre_result vre_circle_create ( vre_circle **ppCircle,
                               vre_point *pStartPoint,
                               vre_point *pEndPoint,
                               vre_fix16 radius );
                               
void vre_circle_destroy ( vre_circle *pCircle );

vre_result vre_circle_flatten ( vre_circle *pCircle,
                               vre_mat3x3 *pMat,
                               vre_polygon *pPolygon, 
                               vre_fix16 max_deviation );                                                            

#endif
