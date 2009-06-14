

#ifndef VRE_LINE_TYPE
#define VRE_LINE_TYPE

typedef struct vre_line vre_line;

#endif


#ifndef VRE_LINE_H
#define VRE_LINE_H

#include "vre_defs.h"
#include "vre_matrix.h"
#include "vre_polygon.h"

struct vre_line
{
    vre_point start_point;
    vre_point end_point;
};

void vre_line_to_polygon ( vre_line *pLine,
                           vre_style *pStyle,
                           vre_mat3x3 *pMat,
                           vre_polygon *pPolygon );

#endif
