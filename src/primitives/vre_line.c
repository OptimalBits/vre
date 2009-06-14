/*
 *  vre_line.c
 *  
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_line.h"
/*
void vre_line_to_polygon ( vre_context *pContext, vre_point *pStart, vre_point *pEnd)
{
    vre_uint    first_point;
    vre_uint    i;
    vre_fix8    min_y;
    vre_point   normal;
    vre_point   line_polygon[4];
	
	vre_style *pStyle = vre_context_get_style ( pContext );
    
    // Create line out of a polygon.
    
    // 1) Compute perpendicular unity vector
    normal.x = - (pEnd->y - pStart->y);
    normal.y = pEnd->x - pStart->x;                
    
    printf (" normal.x: %d, normal: %d\n", normal.x, normal.y);
    
    // 2) Make the vector as big as stroke-width
    vre_vector_scale (&normal, pStyle->stroke_width<<8);
    
    printf ("scaled normal.x: %d, normal: %d\n", normal.x, normal.y);
    
    // 3) Create clockwise ordered quad 
    line_polygon[0].x = pStart->x - normal.x/2;;
    line_polygon[0].y = pStart->y - normal.y/2;;
    
    line_polygon[1].x = pEnd->x - normal.x/2;;
    line_polygon[1].y = pEnd->y - normal.y/2;;
    
    line_polygon[2].x = pEnd->x + normal.x/2;
    line_polygon[2].y = pEnd->y + normal.y/2;
    
    line_polygon[3].x = pStart->x + normal.x/2;
    line_polygon[3].y = pStart->y + normal.y/2;
    
    for (i = 0; i < 4; i++)
    {
        printf (" point %d: %d, %d \n",i, line_polygon[i].x, line_polygon[i].y);
    }
    
    // Find lowest y 
    first_point = 0;
    min_y = line_polygon[0].y;
    for ( i = 1; i < 4; i++)
    {
        if (min_y > line_polygon[i].y) 
        {
            first_point = i;
            min_y = line_polygon[i].y;
        }
    }
    
    vre_polygon_convex (pContext, &line_polygon, 4, first_point);
}


*/