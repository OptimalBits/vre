/*
 *  vre_polyline.h
 *  
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

typedef struct vre_polyline
{
    vre_point *pPoints; 
    vre_uint32 num_points;
};

void vre_polyline_to_polygon ( vre_polyline *pPolyline,
                               vre_style *pStyle,
                               vre_mat3x3 *pMat,
                               vre_polygon *pPoly );



