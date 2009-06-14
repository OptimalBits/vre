
#include "vre_glyph.h"
#include "vre_defs.h"
#include "vre_list.h"
#include "vre_bezier.h"
#include "vre_matrix.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"


vre_result vre_create_glyph ( vre_glyph **ppGlyph, vre_uint16 num_contours )
{
    vre_result vres = VRE_ERR_OK;
    vre_uint32 size;
    
    vre_glyph *pGlyph;

    *ppGlyph = 0;

    pGlyph = ( vre_glyph* ) vre_malloc ( sizeof ( vre_glyph ) );
    if ( pGlyph == 0 ) 
    {
        return VRE_ERR_MEM_ALLOC_FAILED;    
    }
    
    vre_memset ( pGlyph, sizeof ( vre_glyph ), 0 );
    
    if ( num_contours == 0 )
    {
        return vres;
    }

    size = sizeof ( vre_glyph_contour ) * num_contours;
    pGlyph->pContours = ( vre_glyph_contour * ) vre_malloc( size );
            
    if ( pGlyph->pContours == 0 )
    {
        vre_free ( pGlyph );        
        vres = VRE_ERR_MEM_ALLOC_FAILED;
    }

    vre_memset ( pGlyph->pContours, sizeof ( size ), 0 );
    
    *ppGlyph = pGlyph;

    return vres;
}

void vre_glyph_destroy ( vre_glyph *pGlyph )
{
    if ( pGlyph != 0 )
    {
        vre_uint32 i;
        
        for (i = 0; i < pGlyph->num_contours; i++)
        {
            vre_free( pGlyph->pContours[i].pSegments );
            vre_free( pGlyph->pContours[i].pPoints ); 
        }
        vre_free ( pGlyph->pContours );
        vre_free ( pGlyph );
    }
}

void vre_composite_glyph_destroy( vre_list *pComponents )
{
    vre_uint32 i;
    vre_uint32 num_elems;
    
    if ( pComponents != 0 )
    {
        num_elems = vre_list_num_elems( pComponents );
    
        for ( i = 0; i < num_elems; i++ )
        {
            vre_free ( vre_list_peek( pComponents, i ));
        }

        vre_list_destroy( pComponents );

    }
}


void vre_meta_glyph_destroy( vre_meta_glyph *pMetaGlyph )
{
    if ( pMetaGlyph != 0 )
    {
        switch ( pMetaGlyph->glyph_type )
        {
            case 0: vre_glyph_destroy( pMetaGlyph->pGlyph );
                break;
            case 1: vre_composite_glyph_destroy( pMetaGlyph->pGlyph );
                break;
        }
        vre_free( pMetaGlyph );
    }
}


void vre_glyph_set_bounding_box ( vre_glyph *pGlyph, 
                                  vre_int16 x_min,
                                  vre_int16 y_min, 
                                  vre_int16 x_max, 
                                  vre_int16 y_max )
{
    pGlyph->bbox.x1 = x_min;
    pGlyph->bbox.y1 = y_min;
    pGlyph->bbox.x2 = x_max;
    pGlyph->bbox.y2 = y_max;
}

void vre_glyph_get_bounding_box ( vre_glyph *pGlyph,
                                  vre_rectangle *pBBox,
                                  vre_mat3x3 *pMat )
{
    vre_uint32 i;
    vre_int32 maxx, minx, maxy, miny;
    
    vre_point src_points[4];
    vre_point dst_points[4];
    
    vre_assert ( pGlyph );
    vre_assert ( pBBox );
    vre_assert ( pMat );
    
    src_points[0].x = pGlyph->bbox.x1;
    src_points[0].y = pGlyph->bbox.y1;
    
    src_points[1].x = pGlyph->bbox.x2;
    src_points[1].y = pGlyph->bbox.y1;

    src_points[2].x = pGlyph->bbox.x2;
    src_points[2].y = pGlyph->bbox.y2;
    
    src_points[3].x = pGlyph->bbox.x1;
    src_points[3].y = pGlyph->bbox.y2;
    
    vre_matrix_transform ( pMat, src_points, dst_points, 4 );
    
    minx = 0x7FFFFFFF;
    maxx = 0x80000000;
    miny = 0x7FFFFFFF;
    maxy = 0x80000000;
    
    for ( i = 0; i < 4; i++ )
    {
        minx = VRE_MIN ( dst_points[i].x, minx );
        maxx = VRE_MAX ( dst_points[i].x, maxx );
        miny = VRE_MIN ( dst_points[i].y, miny );
        maxy = VRE_MAX ( dst_points[i].y, maxy );
    }
    
    pBBox->x1 = minx; 
    pBBox->y1 = miny;
    pBBox->x2 = maxx;
    pBBox->y2 = maxy;
    
}
  
vre_result vre_glyph_to_polygon  ( vre_glyph *pGlyph,
                                   vre_style *pStyle,
                                   vre_mat3x3 *pMat,
                                   vre_polygon **ppPoly)
{
    vre_result vres = VRE_ERR_OK;
    vre_polygon *pPoly;
    vre_bezier *pBezier;
    vre_point tmp_point;
    
    vre_assert ( pGlyph != 0 );
    
    *ppPoly = 0;
    
    vres = vre_polygon_create3( &pPoly, 3, pGlyph->num_contours);
    VRE_RETURN_IF_ERR ( vres );
    
    for (;;)
    {           
        vre_uint32 i;
        vres = vre_polygon_set_num_contours ( pPoly, pGlyph->num_contours );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_bezier_create ( &pBezier, 3 );
        VRE_BREAK_IF_ERR ( vres );
                               
        for ( i = 0; i < pGlyph->num_contours; i++ )
        {
            vre_uint32  s;
            vre_uint32  p = 0;
            vre_glyph_contour   *pContour = &pGlyph->pContours[i];

            vres = vre_polygon_start_contour ( pPoly );
            VRE_BREAK_IF_ERR ( vres );
            
            for ( s = 0; s < pContour->num_segments; s++ )
            {
                //
                //  Add Line
                //
                if ( pContour->pSegments[s] == 2 )
                {                                       
                    vre_matrix_transform (pMat, &pContour->pPoints[p], 
                                          &tmp_point, 1);
                
                    vres = 
                        vre_polygon_add_point( pPoly, &tmp_point );
                    VRE_BREAK_IF_ERR ( vres );
                                                           
                    p++;
                }
                //
                // Quadratic bézier spline
                //         
                else               
                {                    
                    vre_uint32 num_points = pContour->pSegments[s] - 1;
                    vre_point bezier_points[3];
                    vre_point *pPoints = pContour->pPoints;
                                                          
                    vre_matrix_transform (pMat, &pContour->pPoints[p], 
                                          &bezier_points[0], 1); 
                    
                    p++;
                   
                    while ( num_points > 2 )
                    {                    
                                        
                        vre_matrix_transform (pMat, &pContour->pPoints[p], 
                                              &bezier_points[1], 1); 
                        
                        tmp_point.x = (pPoints[p].x + pPoints[p+1].x + 1)/2;
                        tmp_point.y = (pPoints[p].y + pPoints[p+1].y + 1)/2;
                        
                        vre_matrix_transform (pMat, &tmp_point, 
                                              &bezier_points[2], 1); 
                    
                        vre_bezier_set_points( pBezier, bezier_points, 3 ); 
                    
                        vres = vre_bezier_flatten( pBezier, pPoly, 1000, 0 );
                        VRE_BREAK_IF_ERR ( vres );                     
                                                                    
                        bezier_points[0].x = bezier_points[2].x;
                        bezier_points[0].y = bezier_points[2].y;
                        
                        p++;
                        num_points--;
                    }
                    VRE_BREAK_IF_ERR ( vres );
                    
                    vre_matrix_transform (pMat, &pContour->pPoints[p], 
                                          &bezier_points[1], 2); 
                    
                    p++;
                                       
                    vre_bezier_set_points( pBezier, bezier_points, 3 ); 
                                    
                    vres = vre_bezier_flatten( pBezier, pPoly, 1000, 0 );
                    VRE_BREAK_IF_ERR ( vres );                
                }
            }
            
            vre_matrix_transform (pMat, &pContour->pPoints[p], &tmp_point, 1); 
            
            vres = vre_polygon_add_point( pPoly, &tmp_point );
            VRE_BREAK_IF_ERR( vres );
            
            vres = vre_polygon_end_contour ( pPoly );
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        break;    
    }    
    
    if ( vres != VRE_ERR_OK )
    {
        vre_polygon_destroy( pPoly );
    }
    else
    {
        *ppPoly = pPoly;
    }
    
    vre_bezier_destroy ( pBezier );
           
    return vres;
}




