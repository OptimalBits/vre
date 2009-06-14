


#ifndef VRE_VECTOR_H
#define VRE_VECTOR_H

#include "vre_defs.h"
#include "vre_math.h"

vre_int32 vre_vector_length ( vre_point *pPoint );

vre_ufix16 vre_vector_length_apprx ( vre_point *pPoint );

void vre_vector_scale (vre_point *pPoint, vre_ufix16 scale);

vre_fix16 vre_vector_dot_product (vre_point *pU, vre_point *pV);

// Move to proper C file
// Check for parallels lines elsewhere!
void vre_line_intersect ( vre_point *pNormal1, vre_point *pPoint1, 
                          vre_point *pNormal2, vre_point *pPoint2,
                         vre_point *pIntersection );

#endif

