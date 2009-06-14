
#ifndef VRE_CLIP_H
#define VRE_CLIP_H

#include "vre_defs.h"
#include "vre_polygon.h"
#include "vre_rectangle.h"

void vre_clip_poly_rectangle ( vre_polygon *pPolygon,
							   vre_rectangle *pRectangle,
							   vre_polygon *pOutPoly,
							   vre_polygon *pTmpPoly );

#endif
