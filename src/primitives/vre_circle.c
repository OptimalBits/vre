/*
 *  vre_circle.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-06-26.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_circle.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"


struct vre_circle
{
    vre_point start;
    vre_point end;
    vre_fix16 radius;
};

static void draw_arc ( vre_point p0, vre_point p1, 
                       vre_fix16 d, vre_fix16 max_d,
                       vre_point *pPoints,
                       vre_uint32 start,
                       vre_uint32 end ) ;

vre_result vre_circle_create ( vre_circle **ppCircle,
                               vre_point *pStartPoint,
                               vre_point *pEndPoint,
                               vre_fix16 radius )
{
    vre_result vres;
    vre_circle *pCircle;
    
    pCircle = VRE_TALLOC ( vre_circle );
    if ( pCircle != 0 )
    {
        pCircle->start = *pStartPoint;
        pCircle->end = *pEndPoint;
        pCircle->radius = radius;
        *ppCircle = pCircle;
        vres = VRE_ERR_OK;
    }
    else
    {
        *ppCircle = 0;
        vres = VRE_ERR_MEM_ALLOC_FAILED;
    }
    return vres;
}

void vre_circle_destroy ( vre_circle *pCircle )
{
    vre_free ( pCircle );
}

vre_result vre_circle_flatten ( vre_circle *pCircle,
                                vre_mat3x3 *pMat,
                                vre_polygon *pPolygon, 
                                vre_fix16 max_deviation )
{
    vre_result vres = VRE_ERR_OK;
    vre_uint32 num_points;
    vre_uint32 d;
    vre_point *pPoints = 0;
    
    d = VRE_ABS (pCircle->radius);
    num_points = 0;
    while ( d > max_deviation )
    {
        num_points ++;
        d = (d + 2) >> 2;
    }
    num_points = (1 << num_points) - 1;
     
    for (;;)
    {
        vres = vre_polygon_add_point ( pPolygon, &pCircle->start );
        VRE_BREAK_IF_ERR ( vres );
        
        if ( num_points )
        {
            pPoints = (vre_point *) vre_malloc ( sizeof (vre_point) * 
                                                 num_points );
            
            if ( pPoints )
            {
                
                draw_arc ( pCircle->start, pCircle->end, 
                           pCircle->radius, max_deviation, 
                           pPoints, 0, num_points-1 );
                vres = vre_polygon_add_points ( pPolygon, pPoints, num_points );
                VRE_BREAK_IF_ERR ( vres );
            }
            else
            {
                vres = VRE_ERR_MEM_ALLOC_FAILED;
                break;
            }            
        }
        
        vres = vre_polygon_add_point ( pPolygon, &pCircle->end );
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
        
    vre_free ( pPoints );
    
    return vres;
}


static void draw_arc ( vre_point p0, vre_point p1, 
                       vre_fix16 d, vre_fix16 max_d,
                       vre_point *pPoints,
                       vre_uint32 start,
                       vre_uint32 end ) 
{ 
    vre_point v; 
    vre_point pm, pb; 
    vre_uint32 index;
    
    if ( VRE_ABS(d) > max_d)
    {
        v.x = p1.x - p0.x;       // vector from p0 to p1 
        v.y = p1.y - p0.y; 
        
        pm.x = p0.x + ((v.x + 1) >> 1);  // midpoint  
        pm.y = p0.y + ((v.y + 1) >> 1); 
        
        d = (d + 2) >> 2;
        
        vre_vector_scale(&v, d);  // subdivided vector
        
        pb.x = pm.x + v.y;        // bisection point  
        pb.y = pm.y - v.x;
        
        vre_assert ( (end - start) >= 0 );
        
        index = (end + start) >> 1;
        pPoints[index] = pb;
        
        draw_arc(p0, pb, d, max_d, pPoints, start, index-1); // first half arc  
        draw_arc(pb, p1, d, max_d, pPoints, index+1, end);   // second half arc 
    }
}    



