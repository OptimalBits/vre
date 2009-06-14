/*
 *  vre_path.c
 *  vre
 *
 *  Created by Manuel Astudillo on 2007-12-04.
 *  Copyright 2008 . All rights reserved.
 *
 */

#include "vre_defs.h"
#include "vre_mem.h"
#include "vre_path.h"
#include "vre_stroke.h"
#include "vre_array.h"
#include "vre_matrix.h"
#include "vre_polygon.h"

/**
    A path is a sucession of subpaths. Every subpath is composed of one or
    more segments. There can be many types of segments. 
    Segments are implemented with an add function as well as providing 
    a flatten callback.
 
 */


/**
 Function callback for path segments.
 
 @param void* 
 
 @param pStart start point.
 
 @param pPolygon Pointer to polygon. This polygon holds the points and
 contours of the path.
 
 @return vre error code.
 
 */
typedef vre_result (*VRE_SEGMENT_FLATTEN) ( void*, 
                                            vre_point *pStart, 
                                            vre_polygon *pPolygon );

/**
 Function callback for path segments.
 
 @param void* 
 
 @param pPolygon Pointer to polygon. This polygon holds the points and
 contours of the path.
 
 @return vre error code.
 
 */
typedef void (*VRE_SEGMENT_DESTROY) ( void* );

typedef struct 
{
    VRE_SEGMENT_FLATTEN pFlatten;
    
} vre_path_segment;

struct vre_path
{
    vre_array *pSegments;
    
    vre_style  *pStyle;
    vre_stroke *pStroke;
    
    vre_polygon *pPolygon;
    vre_polygon *pClippedPoly;
    vre_polygon *pTmpPoly;
    
    vre_bezier  *pBezierQuad;
    vre_bezier  *pBezierCubic;
    
    vre_mat3x3 matrix;
    
    vre_point  cur; // Current.
    vre_point  o; // This subpath origin.
    
    vre_point s;
    vre_point st;
    
    vre_point t;
    
    vre_point p;
    
    vre_bool isInSubpath;
};

static void updateOrigin ( vre_path *pPath, vre_bool relative, 
                           vre_fix16 x, vre_fix16 y );

vre_result vre_path_create ( vre_path **ppPath,
                             vre_style *pStyle )
{
    vre_result vres = VRE_ERR_OK;
    vre_path *pPath;
    
    pPath = VRE_TALLOC ( vre_path );
    if ( pPath == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED; 
    }
    
    vre_memset( pPath, sizeof ( vre_path ), 0);
    
    for (;;)
    {
        vres = vre_array_create(32, &pPath->pSegments);
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_polygon_createEx ( &pPath->pPolygon );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_polygon_createEx ( &pPath->pClippedPoly );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_polygon_createEx ( &pPath->pTmpPoly );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_bezier_create( &(pPath->pBezierQuad), 3 );
        VRE_BREAK_IF_ERR ( vres );
                                 
        vres = vre_bezier_create( &(pPath->pBezierCubic), 4 );
        VRE_BREAK_IF_ERR ( vres );
        
        vre_matrix_identity ( &pPath->matrix );
        
        pPath->pStyle = pStyle;
        
        pPath->isInSubpath = VRE_FALSE;
        
        break;
    }
    if ( vres != VRE_ERR_OK )
    {
        vre_path_destroy ( pPath );
        *ppPath = 0;
    }
    else
    {
        *ppPath = pPath;
    }
    
    return vres;
}

void vre_path_destroy ( vre_path *pPath )
{
    if ( pPath )
    {
        vre_array_destroy( pPath->pSegments );
        vre_polygon_destroy ( pPath->pPolygon );
        vre_polygon_destroy ( pPath->pClippedPoly );
        vre_polygon_destroy ( pPath->pTmpPoly );
        vre_bezier_destroy  ( pPath->pBezierQuad );
        vre_bezier_destroy  ( pPath->pBezierCubic );
        vre_free ( pPath );
    }
}

/**
    Starts a new subpath.
    
    @param  pPath current path to add the subpath to.
    @param  x, y coordinates as starting point for this subpath.
 
    @return vre error code.
 */
vre_result vre_path_add_move_to ( vre_path *pPath, vre_fix16 x, vre_fix16 y )
{    
    vre_result vres;
    vre_point dstPoint;
    
    pPath->o.x = x;
    pPath->o.y = y;
    
    pPath->s = pPath->p = pPath->o;
    
    //
    // Start a new subpath
    //
    
    if ( pPath->isInSubpath )
    {
        vre_polygon_end_contour ( pPath->pPolygon );
        vre_polygon_start_contour ( pPath->pPolygon );
    }
    else
    {
        vre_polygon_start_contour ( pPath->pPolygon );
        pPath->isInSubpath = VRE_TRUE;
    }    
    
    //
    // Transform point
    //
    
    vre_matrix_transform ( &pPath->matrix, 
                           &pPath->o, 
                           &dstPoint, 
                           1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    VRE_RETURN_IF_ERR ( vres );
    
    pPath->st = pPath->t = dstPoint;
    
    // Create Stroke
    if ( pPath->pStyle->stroke_type != 0 )
    {
        vres = vre_stroke_create ( &pPath->pStroke, 
                                   pPath->pStyle->stroke_width, 0, 0, 
                                   &pPath->st );
        VRE_RETURN_IF_ERR ( vres );
    }
    
    return vres;
}

/**
    Close the current subpath.
 */
vre_result vre_path_close ( vre_path *pPath )
{    
    vre_result vres;
    
    pPath->p = pPath->o = pPath->s;
    
    vre_polygon_end_contour ( pPath->pPolygon );
    
    pPath->isInSubpath = VRE_FALSE;
    
    if ( pPath->pStyle->stroke_type != 0 )
    {
        vres = vre_stroke_close ( pPath->pStroke );
        VRE_RETURN_IF_ERR ( vres );
    }
    
    return VRE_ERR_OK;
}

vre_result vre_path_end ( vre_path *pPath )
{
    vre_result vres;
    
    pPath->p = pPath->o = pPath->s;
    
    vre_polygon_end_contour ( pPath->pPolygon );
    
    pPath->isInSubpath = VRE_FALSE;
    
    if ( pPath->pStyle->stroke_type != 0 )
    {
        vres = vre_stroke_end ( pPath->pStroke );
        VRE_RETURN_IF_ERR ( vres );
    }
    
    return VRE_ERR_OK;
}


vre_result vre_path_add_line ( vre_path *pPath, 
                               vre_bool relative, 
                               vre_fix16 x, 
                               vre_fix16 y )
{
    vre_result vres = VRE_ERR_OK;
    vre_point dstPoint;
    
    updateOrigin ( pPath, relative, x, y );
    
    pPath->p = pPath->o;
    
    vre_matrix_transform ( &pPath->matrix, 
                           &pPath->o, 
                           &dstPoint, 
                           1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    VRE_RETURN_IF_ERR ( vres );
    
    if ( pPath->pStyle->stroke_type != 0 )
    {
        vres = vre_stroke_add_line ( pPath->pStroke, dstPoint );
        VRE_RETURN_IF_ERR ( vres );
    }
        
    pPath->t = dstPoint;
    
    return vres;
}

vre_result vre_path_add_quadric ( vre_path *pPath, 
                                  vre_bool relative, 
                                  vre_point *pPoints )
{
    vre_result vres;
    
    vre_point srcPoints[3];
    vre_point dstPoints[3];
    
    pPath->p.x = pPoints[0].x;
    pPath->p.y = pPoints[0].y;
    
    srcPoints[0] = pPath->o;
    
    updateOrigin ( pPath, relative, pPoints[0].x, pPoints[0].y );
    
    srcPoints[1] = pPath->o;
    
    updateOrigin ( pPath, relative, pPoints[1].x, pPoints[1].y );
    
    srcPoints[2] = pPath->o;
    
    // Transform segment
    vre_matrix_transform ( &pPath->matrix, 
                           srcPoints, 
                           dstPoints, 
                           3 );
    
    // Flatten Bezier
    vre_bezier_set_points ( pPath->pBezierQuad, dstPoints, 3 );
    
    vres = vre_bezier_flatten ( pPath->pBezierQuad, pPath->pPolygon, 1000, 1 );
    VRE_RETURN_IF_ERR ( vres );
    
    if ( pPath->pStyle->stroke_type != 0 )
    {
        vres =  vre_stroke_add_quad_bezier ( pPath->pStroke, 
                                            dstPoints[1],
                                            dstPoints[2] );
        VRE_RETURN_IF_ERR ( vres );
    }
    
    pPath->t = dstPoints[2];
    
    return VRE_ERR_OK;
}

vre_result vre_path_add_cubic ( vre_path *pPath, 
                                vre_bool relative, 
                                vre_point *pPoints )
{
    vre_result vres;
    
    vre_point srcPoints[4];
    vre_point dstPoints[4];
    
    pPath->p.x = pPoints[0].x;
    pPath->p.y = pPoints[0].y;
    
    srcPoints[0] = pPath->o;
    
    updateOrigin ( pPath, relative, pPoints[0].x, pPoints[0].y );
    
    srcPoints[1] = pPath->o;
    
    updateOrigin ( pPath, relative, pPoints[1].x, pPoints[1].y );
    
    srcPoints[2] = pPath->o;
    
    updateOrigin ( pPath, relative, pPoints[2].x, pPoints[2].y );
    
    srcPoints[3] = pPath->o;
    
    vre_matrix_transform ( &pPath->matrix, 
                           srcPoints, 
                           dstPoints, 
                           4 );
    
    vre_bezier_set_points ( pPath->pBezierCubic, dstPoints, 4 );
    
    if ( pPath->pStyle->stroke_type != 0 )
    {
       vre_bezier_set_map_func (  pPath->pBezierCubic, 
                                  (VRE_BEZIER_MAP_FUNC) vre_stroke_add_line, 
                                  pPath->pStroke ); 
    }
    
    vres = vre_bezier_flatten ( pPath->pBezierCubic, pPath->pPolygon, 1000, 1 );
    VRE_RETURN_IF_ERR ( vres );
    
    vre_bezier_set_map_func ( pPath->pBezierCubic, 0, 0 );
    
    return vres;
}

vre_result vre_path_add_arc ( vre_path *pPath, vre_point *pStart, 
                              vre_point *pVertAxis, vre_point *pHorzAxis, 
                              vre_fix16 angle );

vre_result vre_path_prepare ( vre_path *pPath,
                              vre_mat3x3 *pMat,
                              vre_context *pContext );

/**
    Render to a tile.
 */
vre_result vre_path_render ( vre_path *pPath,
                             vre_render *pRenderer,
                             vre_mat3x3 *pMat, 
                             vre_tile *pTile )
{
    vre_result vres = VRE_ERR_OK;
    vre_rectangle clipRectangle;
    
    //
    // Clip polygon.
    // ( TODO: Use bounding rectangle to decide if 
    //   clipping must be performed at all )
    //
    clipRectangle.x = pTile->x << 16;
    clipRectangle.y = pTile->y << 16;
    clipRectangle.w = pTile->w << 16;
    clipRectangle.h = pTile->h << 16;
    
    if ( pPath->pStyle->fill_type != VRE_FILL_TYPE_NONE )
    {
        if ( vre_polygon_get_num_vertices ( pPath->pPolygon ) > 2 )
        {
            vre_polygon *pHackPoly;
            
            vre_polygon_reset ( pPath->pClippedPoly );
            vre_polygon_reset ( pPath->pTmpPoly );
            
            // Temporal hack. This matrix should multiply the original points
            // before flattening.
            
            vre_polygon_clone ( &pHackPoly, pPath->pPolygon );
            
            vres = vre_polygon_trasform_points ( pHackPoly, 
                                                 pPath->pPolygon,
                                                 pMat );
            
            //
            
            vre_clip_poly_rectangle ( pHackPoly,
                                      &clipRectangle,
                                      pPath->pClippedPoly,
                                      pPath->pTmpPoly );
            
            vre_polygon_destroy(pHackPoly);
            
            if ( vre_polygon_get_num_vertices ( pPath->pClippedPoly ) > 2 )
            {
                vre_draw_polygon_concave_aa  ( pRenderer,
                                               pPath->pClippedPoly, 
                                               pTile,
                                               pPath->pStyle );
            }
        }
    }
    
    //
    // Render strokes ( render an array of polygons representing the strokes );
    //
    if ( pPath->pStroke )
    {
        vre_polygon *pStrokePoly;
        vre_stroke_get_polygon ( pPath->pStroke, &pStrokePoly );
        
        if ( vre_polygon_get_num_vertices ( pStrokePoly ) > 2 )
        {
            vre_polygon *pHackPoly;
            
            vre_polygon_reset ( pPath->pClippedPoly );
            vre_polygon_reset ( pPath->pTmpPoly );
            
            // Temporal hack. This matrix should multiply the original points
            // before flattening.
            
            vre_polygon_clone ( &pHackPoly, pStrokePoly );
            
            vres = vre_polygon_trasform_points ( pHackPoly, 
                                                 pStrokePoly,
                                                 pMat );
            
            //
            
            vre_clip_poly_rectangle ( pHackPoly,
                                      &clipRectangle,
                                      pPath->pClippedPoly,
                                      pPath->pTmpPoly );
            
            if ( vre_polygon_get_num_vertices ( pPath->pClippedPoly ) > 2 )
            {
                vre_uint32 tmp = pPath->pStyle->fg_color;
                pPath->pStyle->fg_color = pPath->pStyle->stroke_color;
                
                vre_draw_polygon_concave_aa  ( pRenderer,
                                               pPath->pClippedPoly, 
                                               pTile,
                                               pPath->pStyle );
                pPath->pStyle->fg_color = tmp;
            }
        }
    }
    
    return vres;
}


//------------------------------------------------------------------------------
static void updateOrigin ( vre_path *pPath, vre_bool relative, 
                           vre_fix16 x, vre_fix16 y )
{
    if ( relative == VRE_TRUE )
    {
        pPath->o.x += x;
        pPath->o.y += y;
    }
    else
    {
        pPath->o.x = x;
        pPath->o.y = y;
    }
}

