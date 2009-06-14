
#include "vre_context.h"
#include "vre_defs.h"
#include "vre_math.h"
#include "vre_vector.h"
#include "vre_assert.h"
#include "vre_render.h"

void vre_polyline ( vre_context *pContext, 
                    vre_point *pPoints, 
                    vre_uint32 num_points )
{
    vre_uint    i;
    vre_uint    p;
    vre_point   normalPrev, normalCur, normalNext;
    vre_point   normalA, normalB;
    vre_point   line_polygon[4];    
	vre_uint64  stroke_width;

	vre_style *pStyle = vre_context_get_style ( pContext );
	vre_canvas *pCanvas = 0;//vre_context_get_canvas ( pContext );
	
	vre_tile   tile;
	
	tile.pData = pCanvas->pData;
	tile.scanline_width = pCanvas->scanline_width;
	tile.x = 0;
	tile.y = 0;
	tile.w = pCanvas->width;
	tile.h = pCanvas->height;
	
    stroke_width = ((vre_uint64)pStyle->stroke_width)<<16;

    vre_assert ( num_points >= 2 );        

    normalPrev.x = - (pPoints[0].y - pPoints[num_points-1].y);
    normalPrev.y = pPoints[0].x - pPoints[num_points-1].x;

    vre_vector_scale (&normalPrev, 65536);

    normalCur.x = - (pPoints[1].y - pPoints[0].y);
    normalCur.y = pPoints[1].x - pPoints[0].x;

    vre_vector_scale (&normalCur, 65536);

    for ( p = 1; p < num_points+1; p++ )
    {
        vre_fix16  angle;
        vre_fix16  dot_product;
        vre_fix16  inv_sin;

        vre_uint next_point = (p+1) % num_points;
        vre_uint cur_point = p % num_points;
        
        normalNext.x = - (pPoints[next_point].y - pPoints[cur_point].y);
        normalNext.y = pPoints[next_point].x - pPoints[cur_point].x;

        vre_vector_scale (&normalNext, 65536);
        
        // Compute pseudo-normal A (for vertex A) 
        normalA.x = normalPrev.x + normalCur.x;
        normalA.y = normalPrev.y + normalCur.y;

        // Compute angle between normals
        dot_product = vre_vector_dot_product (&normalPrev, &normalCur);
        angle = (vre_pi - vre_arccos (dot_product)) / 2;        
        inv_sin = ((vre_uint32) 0xffffffff) / vre_sin (angle);

        vre_vector_scale (&normalA, (stroke_width * inv_sin)>>16);

        // Compute pseudo-normal B (for vertex B)
        normalB.x = normalCur.x + normalNext.x;
        normalB.y = normalCur.y + normalNext.y;

        // Compute angle between normals
        dot_product = vre_vector_dot_product (&normalCur, &normalNext);
        angle = (vre_pi - vre_arccos (dot_product)) / 2;
        inv_sin = ((vre_uint32) 0xffffffff) / vre_sin (angle);

        vre_vector_scale (&normalB, (stroke_width * inv_sin)>>16);

        // Create clockwise ordered quad 
        line_polygon[0].x = pPoints[p-1].x - normalA.x/2;;
        line_polygon[0].y = pPoints[p-1].y - normalA.y/2;;
    
        line_polygon[1].x = pPoints[cur_point].x - normalB.x/2;;
        line_polygon[1].y = pPoints[cur_point].y - normalB.y/2;;
    
        line_polygon[2].x = pPoints[cur_point].x + normalB.x/2;
        line_polygon[2].y = pPoints[cur_point].y + normalB.y/2;

        line_polygon[3].x = pPoints[p-1].x + normalA.x/2;
        line_polygon[3].y = pPoints[p-1].y + normalA.y/2;

        // Assign next normal
        normalPrev.x = normalCur.x;
        normalPrev.y = normalCur.y;

        normalCur.x = normalNext.x;
        normalCur.y = normalNext.y;

        // Find lowest y
        /* 
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
        */
        
        // Convert to 8 bits fixed point
        for ( i = 0; i < 4; i++ )
        {
            line_polygon[i].x = (line_polygon[i].x + 128) >> 8;
            line_polygon[i].y = (line_polygon[i].y + 128) >> 8;
        }

        // Draw segment
        vre_draw_polygon_concave ( &tile, pStyle, &line_polygon[0], 4 );

//        vre_polygon_convex (pContext, &line_polygon[0], 4, first_point);
    }
}
