/*
    vre_clip.c

*/

#include "vre_defs.h"
#include "vre_polygon.h"
#include "vre_rectangle.h"
#include "vre_mem.h"
#include "vre_assert.h"


//
// Prototype declarations.
//
static vre_result clip_edge_horizontal ( vre_point *pFirst, 
                                         vre_point *pSecond, 
                                         vre_int32 y, 
                                         vre_polygon *pOutPoly, 
                                         vre_uint32 mask );
								   
static vre_result clip_edge_vertical( vre_point *pFirst, 
                                      vre_point *pSecond, 
                                      vre_int32 x, 
                                      vre_polygon *pOutPoly, 
                                      vre_uint32 mask );
                                
static void clip_polygon_horizontal_up (vre_polygon *pPolygon,
                                        vre_int32 y,
                                        vre_polygon *pOutPoly,
                                        vre_uint32 start,
                                        vre_uint32 end);
                                     
static void clip_polygon_horizontal_down (vre_polygon *pPolygon,
                                          vre_int32 y,
                                          vre_polygon *pOutPoly,
                                          vre_uint32 start,
                                          vre_uint32 end);                                     
                                     
static void clip_polygon_vertical_left (vre_polygon *pPolygon,
                                         vre_int32 x,
                                         vre_polygon *pOutPoly,
                                         vre_uint32 start,
                                         vre_uint32 end);

static void clip_polygon_vertical_right (vre_polygon *pPolygon,
                                         vre_int32 x,
                                         vre_polygon *pOutPoly,
                                         vre_uint32 start,
                                         vre_uint32 end);
                                   
                                     
/**
    Clips a polygon against a rectangle.
    (Uses Sutherland-Hodgman's clipping algorithm).

    @param pPolygon
    @param pRectangle
    @param pOutPolygon Polygon structure where the output
    will be put. This polygon should have at least 5.0625 times
    more vertices than the input polygon in order to guarantee that
    the clipped polygon will have room on it.
*/
void vre_clip_poly_rectangle ( vre_polygon *pPolygon,
                               vre_rectangle *pRectangle,
                               vre_polygon *pOutPoly,
                               vre_polygon *pTmpPoly )
{    
    vre_uint32 c;
    vre_uint32 num_contours;
    vre_polygon *pTmpPoly2;
    vre_result vres;
    
    vre_assert ( vre_polygon_get_num_vertices (pPolygon) >= 3 );
        
    num_contours = vre_polygon_get_num_contours ( pPolygon );
    
    vre_polygon_create2(&pTmpPoly2, 32);
    
    vres = vre_polygon_set_num_contours ( pOutPoly, num_contours );

    for ( c = 0; c < num_contours; c++ )
    {
        vre_uint32 start, end;
        
        vres = vre_polygon_start_contour ( pOutPoly );
        
        
        start = vre_polygon_get_contour_start (pPolygon, c);
        end = vre_polygon_get_contour_end (pPolygon, c);
        
        //   
        // Clip up
        //
        vre_polygon_reset ( pTmpPoly );
       
        clip_polygon_horizontal_up (pPolygon, pRectangle->y, pTmpPoly, start, end);
        if ( vre_polygon_get_num_vertices ( pTmpPoly ) < 3 )
        {
            continue;
        }
    
        end = vre_polygon_get_num_vertices ( pTmpPoly ) - 1; 
        
        //    
        // Clip Down
        //
        vre_polygon_reset ( pTmpPoly2 );
        clip_polygon_horizontal_down (pTmpPoly, pRectangle->y+pRectangle->h - (1<<16), 
                                      pTmpPoly2, 0, end );
        if ( vre_polygon_get_num_vertices ( pTmpPoly2 ) < 3 )
        {
            continue;
        }
        
         end = vre_polygon_get_num_vertices (pTmpPoly2) - 1; 
    
        //
        // Clip Left
        //
        vre_polygon_reset (pTmpPoly);
        clip_polygon_vertical_left (pTmpPoly2, pRectangle->x, pTmpPoly, 0, end );
        if ( vre_polygon_get_num_vertices (pTmpPoly) < 3 )
        {
            continue;
        }
    
        end = vre_polygon_get_num_vertices (pTmpPoly) - 1;
        
        //
        // Clip Right
        //
        vre_polygon_reset (pTmpPoly2);
        clip_polygon_vertical_right (pTmpPoly, pRectangle->x+pRectangle->w - (1<<16), 
                                     pTmpPoly2, 0, end );
        if ( vre_polygon_get_num_vertices (pTmpPoly2) < 3 )
        {
            continue;
        }
        
        vre_polygon_copy_points ( pOutPoly, pTmpPoly2 );
        
        vres = vre_polygon_end_contour ( pOutPoly );
    }
    
    vre_polygon_destroy( pTmpPoly2 );
	
}

static void clip_polygon_horizontal_up (vre_polygon *pPolygon,
                                        vre_int32 y,
                                        vre_polygon *pOutPoly,
                                        vre_uint32 start,
                                        vre_uint32 end)
{
    vre_uint32 i;
	vre_uint32 mask;
	
    vre_point *pFirst;
	vre_point *pSecond;
    
	vre_assert ( vre_polygon_get_num_vertices (pPolygon) >= 2 );
    
	pFirst = vre_polygon_get_point ( pPolygon, end );
    pSecond = vre_polygon_get_point ( pPolygon, start );
    
    
    mask = 0;
    if ( pFirst->y < y ) mask = 0x02;
    if ( pSecond->y < y ) mask |= 0x01; 
        
    clip_edge_horizontal ( pFirst, pSecond, y, pOutPoly, mask );
    
    pFirst = pSecond;
    pSecond++;
    for ( i = 0; i < end - start; i++ )
    {
        mask = 0;
        if ( pFirst->y < y )  mask = 0x02;
        if ( pSecond->y < y ) mask |= 0x01; 
        
        clip_edge_horizontal ( pFirst, pSecond, y, pOutPoly, mask );		
    
        pFirst++;
        pSecond++;    
    }
}



static void clip_polygon_horizontal_down (vre_polygon *pPolygon,
                                          vre_int32 y,
                                          vre_polygon *pOutPoly,
                                          vre_uint32 start,
                                          vre_uint32 end )
{
    vre_uint32 i;
	vre_uint32 mask;
	vre_point *pFirst;
	vre_point *pSecond;
    
	vre_assert ( vre_polygon_get_num_vertices (pPolygon) >= 2 );
    
	pFirst = vre_polygon_get_point ( pPolygon, end );
    pSecond = vre_polygon_get_point ( pPolygon, start );
    
    mask = 0;
    if ( pFirst->y > y ) mask = 0x02;
    if ( pSecond->y > y ) mask |= 0x01; 
        
    clip_edge_horizontal ( pFirst, pSecond, y, pOutPoly, mask );
    
    pFirst = pSecond;
    pSecond++;
    for ( i = 0; i < end - start; i++ )
    {
        mask = 0;
        if ( pFirst->y > y )  mask = 0x02;
        if ( pSecond->y > y ) mask |= 0x01; 
        
        clip_edge_horizontal ( pFirst, pSecond, y, pOutPoly, mask );		
    
        pFirst++;
        pSecond++;    
    }
}

static void clip_polygon_vertical_left (vre_polygon *pPolygon,
                                        vre_int32 x,
                                        vre_polygon *pOutPoly,
                                        vre_uint32 start,
                                        vre_uint32 end )
{
    vre_uint32 i;
	vre_uint32 mask;
	vre_point *pFirst;
	vre_point *pSecond;

	vre_assert ( vre_polygon_get_num_vertices (pPolygon) >= 2 );
    
	pFirst = vre_polygon_get_point ( pPolygon, end );
    pSecond = vre_polygon_get_point ( pPolygon, start );
    
    mask = 0;
    if ( pFirst->x < x ) mask = 0x02;
    if ( pSecond->x < x ) mask |= 0x01;
        
    clip_edge_vertical ( pFirst, pSecond, x, pOutPoly, mask );
    
    pFirst = pSecond;
    pSecond++;
    for ( i = 0; i < end - start; i++ )
    {
        mask = 0;
        if ( pFirst->x < x )  mask = 0x02;
        if ( pSecond->x < x ) mask |= 0x01; 
        
        clip_edge_vertical ( pFirst, pSecond, x, pOutPoly, mask );
    
        pFirst++;
        pSecond++;    
    }
}

static void clip_polygon_vertical_right (vre_polygon *pPolygon,
                                         vre_int32 x,
                                         vre_polygon *pOutPoly,
                                         vre_uint32 start,
                                         vre_uint32 end )
{
    vre_uint32 i;
	vre_uint32 mask;
	vre_point *pFirst;
	vre_point *pSecond;
	
	vre_assert ( vre_polygon_get_num_vertices (pPolygon) >= 2 );
		
    pFirst = vre_polygon_get_point ( pPolygon, end );
    pSecond = vre_polygon_get_point ( pPolygon, start );
    
    mask = 0;
    if ( pFirst->x > x ) mask = 0x02;
    if ( pSecond->x > x ) mask |= 0x01; 
        
    clip_edge_vertical ( pFirst, pSecond, x, pOutPoly, mask );
    
    pFirst = vre_polygon_get_point ( pPolygon, 0 );
    pSecond++;
    for ( i = 0; i < end - start; i ++ )
    {
        mask = 0;
        if ( pFirst->x > x )  mask = 0x02;
        if ( pSecond->x > x ) mask |= 0x01; 
        
        clip_edge_vertical ( pFirst, pSecond, x, pOutPoly, mask );
    
        pFirst++;
        pSecond++;    
    }
}


// TODO: Remove the divisions (and 64 bits operations)
static void intersect_horizontal ( vre_point *pFirst, 
                                   vre_point *pSecond, 
                                   vre_int32 y, 
                                   vre_point *pOut )
{      
	 vre_int64 x;

     x = pFirst->x + (vre_int64) ( y - pFirst->y ) *
         (pSecond->x - pFirst->x) / (pSecond->y - pFirst->y);

     pOut->x = (vre_fix16) x;

     pOut->y = y;
 
}

static void intersect_vertical ( vre_point *pFirst, 
                                 vre_point *pSecond, 
                                 vre_int32 x, 
                                 vre_point *pOut )
{    
    vre_int64 y;

    y = pFirst->y + (vre_int64) ( x - pFirst->x ) *
        (pSecond->y - pFirst->y) / (pSecond->x - pFirst->x);

    pOut->x = x;
    pOut->y = (vre_fix16) y; 
}


static vre_result clip_edge_horizontal ( vre_point *pFirst, 
                                         vre_point *pSecond, 
                                         vre_int32 y, 
                                         vre_polygon *pOutPoly, 
                                         vre_uint32 mask )
{
    vre_result vres = VRE_ERR_OK;
    vre_point new_point;
    
    switch (mask)
    {
        // Both inside
        case 0: 
                new_point.x = pSecond->x; new_point.y = pSecond->y;
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // First inside, Second outside
        case 1: 
                intersect_horizontal ( pFirst, pSecond, y, &new_point );
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // First outside, Second inside
        case 2:
                intersect_horizontal ( pFirst, pSecond, y, &new_point );
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                if ( vres != VRE_ERR_OK ) break;
                
                new_point.x = pSecond->x; new_point.y = pSecond->y;
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // Both outside
    }

    return vres;
}

static vre_result clip_edge_vertical   ( vre_point *pFirst, 
                                         vre_point *pSecond, 
                                         vre_int32 x, 
                                         vre_polygon *pOutPoly, 
                                         vre_uint32 mask )
{
    vre_result vres = VRE_ERR_OK;
    vre_point new_point;
    
    switch (mask)
    {
        // Both inside
        case 0: 
                new_point.x = pSecond->x; new_point.y = pSecond->y;
                vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // First inside, Second outside
        case 1: 
                intersect_vertical ( pFirst, pSecond, x, &new_point );
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // First outside, Second inside
        case 2:
                intersect_vertical ( pFirst, pSecond, x, &new_point );
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                if ( vres != VRE_ERR_OK ) break;
                
                new_point.x = pSecond->x; new_point.y = pSecond->y;
                vres = vre_polygon_add_point ( pOutPoly, &new_point );
                break;
        // Both outside
    }
    
    return vres;
}

