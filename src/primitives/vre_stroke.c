/**
 *  vre_stroke.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-02-13.
 *  Copyright 2008 Celera Graphics. All rights reserved.
 *
 */

#include "vre_stroke.h"
#include "vre_bezier.h"
#include "vre_circle.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"

typedef enum 
{
    VRE_STROKE_LINE,
    VRE_STROKE_QUAD_BEZIER,
    VRE_STROKE_CUBIC_BEZIER
} vre_stroke_primitive_type;

struct vre_stroke 
{
    vre_point firstPoint;
    vre_point firstInnerPoint;
    vre_point firstNormal;
    
    vre_point prevInnerPoint;
    vre_point prevInnerPoints[4];
    vre_point prevOuterPoints[4];
    
    vre_point prevPoint;
    vre_point prevNormal;
    
    vre_point prevPrimitive[4]; 
    vre_stroke_primitive_type prevPrimitiveType;
    
    vre_polygon *pInnerEdge; // If clockwise, otherwise, they mean the oposite.
    vre_polygon *pOuterEdge;
    vre_polygon *pStroke;
    
    vre_uint32 numPoints;
    
    vre_bezier *pBezier;

    vre_ufix16 width;
    
    vre_stroke_caps_func    pCapsFunc;
    vre_stroke_joint_func   pJointFunc;
};

static void strokePathSegment ( vre_stroke *pStroke,
                                vre_point *pSrcPoints,
                                vre_point *pDstPointsA,
                                vre_point *pDstPointsB,
                                vre_uint32 numPoints );

// Joints and end caps functions.
static vre_result bevelJoint ( vre_point *pCorner,
                               vre_polygon *pPolygonA,
                               vre_polygon *pPolygonB,
                               vre_point *pPrevNormal, 
                               vre_point *pNextNormal,
                               vre_ufix16 stroke_width );

static vre_result buttEndCap ( vre_point *pEndPoint,
                              vre_polygon *pPolygonA,
                              vre_polygon *pPolygonB,
                              vre_point *pNormal, 
                              vre_ufix16 stroke_width );

static vre_result roundJoint ( vre_point *pCorner,
                              vre_polygon *pPolygonA,
                              vre_polygon *pPolygonB,
                              vre_point *pPrevNormal, 
                              vre_point *pNextNormal,
                              vre_ufix16 stroke_width );

static vre_result prevStroke ( vre_stroke *pStroke, vre_point prevInnerPoint );

vre_result vre_stroke_create ( vre_stroke **ppStroke,
                               vre_ufix16 width,
                               vre_stroke_caps_func pCapsFunc,
                               vre_stroke_joint_func pJointFunc,
                               vre_point *pStartPoint )
{
    vre_result vres;
    
    vre_stroke *pStroke;
    
    for (;;)
    {
        pStroke = VRE_TALLOC ( vre_stroke );
        
        if ( pStroke == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        vre_memset ( pStroke, sizeof ( vre_stroke ), 0 );
        
        vres = vre_polygon_createEx ( &(pStroke->pInnerEdge ));
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_polygon_createEx ( &(pStroke->pOuterEdge ));
        VRE_BREAK_IF_ERR ( vres );
        
        vre_polygon_start_contour ( pStroke->pInnerEdge );
        vre_polygon_start_contour ( pStroke->pOuterEdge );
        
        vres = vre_polygon_createEx ( &(pStroke->pStroke ));
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_bezier_create ( &(pStroke->pBezier), 3 );
        VRE_BREAK_IF_ERR ( vres );
        
        pStroke->width = width;
        
        pStroke->pCapsFunc;
        pStroke->pJointFunc;
        
        pStroke->firstPoint = *pStartPoint;
        
        pStroke->prevPoint = *pStartPoint;
        pStroke->prevNormal.x = 0;
        pStroke->prevNormal.y = 0;
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_stroke_destroy ( pStroke );
    }
    
    *ppStroke = pStroke;
    
    return vres;
}

void vre_stroke_destroy ( vre_stroke *pStroke )
{
    if ( pStroke )
    {
        vre_polygon_destroy ( pStroke->pInnerEdge );
        vre_polygon_destroy ( pStroke->pOuterEdge );
    }
}

vre_result vre_stroke_add_line ( vre_stroke *pStroke, vre_point p0 )
{
    vre_result vres = VRE_ERR_OK;
    vre_point nextNormal;
    vre_fix16 width;
    
    pStroke->numPoints ++;
    
    width = ( pStroke->width + 1 ) >> 1;
    
    nextNormal.y =  -(p0.x - pStroke->prevPoint.x);
    nextNormal.x =  p0.y - pStroke->prevPoint.y;
    
    vre_vector_scale(&nextNormal, width );
    
    if ( pStroke->numPoints == 1 )
    {
        pStroke->firstNormal = nextNormal;
    }
    
    //
    // Apply Joint
    //
    
    if ( pStroke->numPoints >= 2 )
    {
        vres = bevelJoint ( &pStroke->prevPoint,
                            pStroke->pInnerEdge,
                            pStroke->pOuterEdge,
                            &pStroke->prevNormal, 
                            &nextNormal, 
                            width );
        
        VRE_RETURN_IF_ERR ( vres );
    }
    
    pStroke->prevPoint = p0;
    pStroke->prevNormal = nextNormal;
    
    return vres;
}

vre_result vre_stroke_add_lines ( vre_stroke *pStroke, 
                                  vre_point *pPoints, 
                                  vre_uint32 num_points )
{
    vre_result vres;
    vre_uint32 i;
    
    for ( i = num_points; i != 0; i-- )
    {
        vres =  vre_stroke_add_line ( pStroke, *pPoints );
        VRE_RETURN_IF_ERR ( vres );
        pPoints++;
    }
    
    return VRE_ERR_OK;
}

vre_result vre_stroke_add_quad_bezier ( vre_stroke *pStroke,
                                        vre_point p0,
                                        vre_point p1 )
{
    vre_result vres;
    vre_point points[3];
    vre_point innerPoints[3];
    vre_point outerPoints[3];
    
    points[0] = pStroke->prevPoint;
    points[1] = p0;
    points[2] = p1;
    
    
    // Compute quad bezier and stroke it
    
    pStroke->prevPoint = p1;
    pStroke->prevPrimitiveType = VRE_STROKE_QUAD_BEZIER;
    
    return VRE_ERR_OK;
}

vre_result vre_stroke_add_cubic_bezier ( vre_stroke *pStroke,
                                        vre_point p0,
                                        vre_point p1,
                                        vre_point p2 )
{
    vre_point points[4];
    
    points[0] = pStroke->prevPoint;
    points[1] = p0;
    points[2] = p1;
    points[3] = p2;
    
    // Compute cubic bezier and stroke it.
    
    pStroke->prevPoint = p2;
    pStroke->prevPrimitiveType = VRE_STROKE_CUBIC_BEZIER;
    
    return VRE_ERR_OK;
}

/**
    Ends polygon, leaving it open. I.e, Caps must be applied on both ends.
 
 */ 
vre_result vre_stroke_end ( vre_stroke *pStroke )
{
    vre_result vres;
    
    //
    // Apply end caps.
    //
    vres = buttEndCap ( &pStroke->firstPoint,
                       pStroke->pInnerEdge, 
                       pStroke->pOuterEdge,
                       &pStroke->firstNormal, 
                       pStroke->width );
    
    vres = buttEndCap ( &pStroke->prevPoint,
                       pStroke->pInnerEdge,
                       pStroke->pOuterEdge,
                       &pStroke->prevNormal, 
                       pStroke->width );
    
    return vres;
}

/**
    Closes stroke. There will be no end caps in this stroke.
 */
vre_result vre_stroke_close ( vre_stroke *pStroke )
{
    vre_result vres;
    vre_fix16 width;
    
    vre_assert ( pStroke->numPoints > 1 );
    
    width = ( pStroke->width + 1 ) >> 1;
    
    vres = vre_stroke_add_line ( pStroke, pStroke->firstPoint );
    VRE_RETURN_IF_ERR ( vres );
    
    {
        vres = bevelJoint ( &pStroke->firstPoint, 
                            pStroke->pInnerEdge, 
                            pStroke->pOuterEdge,
                            &pStroke->prevNormal, 
                            &pStroke->firstNormal, 
                            width );
        VRE_RETURN_IF_ERR ( vres );
        
    }
    
    return vres;
}


/**
 Generates a polygon containing the stroke.
 
 */
void vre_stroke_get_polygon ( vre_stroke *pStroke,
                              vre_polygon **ppPolygon )
{
    vre_polygon_reset ( pStroke->pStroke );
    
    vre_polygon_end_contour ( pStroke->pInnerEdge );
    vre_polygon_end_contour ( pStroke->pOuterEdge );

    vre_polygon_add_contour ( pStroke->pStroke, pStroke->pInnerEdge );
    vre_polygon_add_contour ( pStroke->pStroke, pStroke->pOuterEdge );
    
    *ppPolygon = pStroke->pStroke;
}

/**
 Tells if vector A is "first" than vector B ( i.e., angle is less than Pi rad. )
 
 Algo: check if counter clockwise angle between v1 and v2 is less than Pi rad.
 
 */
vre_bool vre_vector_is_ahead ( vre_point *v1, 
                               vre_point *v2 )
{
    vre_bool result;
    vre_int32 alpha;
    vre_int32 beta;
    vre_int32 angle;
    
    alpha = vre_atan2(v1->y,v1->x);
    beta = vre_atan2(v2->y,v2->x);
    
    if ( alpha > beta )
    {
        angle = (2*vre_pi - alpha) + beta;
    }
    else
    {
        angle = beta - alpha;
    }
    
    if ( angle < vre_pi )
    {
        result = VRE_TRUE;
    }
    else
    {   
        result = VRE_FALSE;
    }
    
    return result;
}

vre_int32 vre_vector_ccw_angle ( vre_point *v1, vre_point *v2 )
{
    vre_int32 alpha;
    vre_int32 beta;
    vre_int32 angle;
    
    alpha = vre_atan2(v1->y,v1->x);
    beta = vre_atan2(v2->y,v2->x);
    
    if ( alpha > beta )
    {
        angle = (2*vre_pi - alpha) + beta;
    }
    else
    {
        angle = beta - alpha;
    }
    
    return angle;
}




/*
    Generate points for bevel joint.
 
 */
static vre_result bevelJoint ( vre_point *pCorner,
                              vre_polygon *pPolygonA,
                              vre_polygon *pPolygonB,
                              vre_point *pPrevNormal, 
                              vre_point *pNextNormal,
                              vre_ufix16 stroke_width )
{
    vre_result vres = VRE_ERR_OK;
    vre_point vertexNormal;
    vre_point innerJoint;
    vre_point outerJoint[2];
    vre_int32 dot_product;
    vre_int32 inv_sin;
    vre_int32 angle;
    
    //
    // Compute vertex pseudo normal
    //
    
    vertexNormal.x = ( pPrevNormal->x + pNextNormal->x );
    vertexNormal.y = ( pPrevNormal->y + pNextNormal->y );
    
    vre_vector_scale( pPrevNormal, 1<<16 );
    vre_vector_scale( pNextNormal, 1<<16 );
    
    //
    // Use some trigonometry to find intersection point for inner joint.
    // ( same length can be used for mitter joint ).
    //
    
    dot_product = vre_vector_dot_product (pPrevNormal, pNextNormal);
    
    // TODO Use same angle for everything...
    //angle = vre_vector_ccw_angle ( pPrevNormal, pNextNormal );
    angle = (vre_pi - vre_arccos (dot_product)) / 2;        
    inv_sin = ((vre_uint32) 0xffffffff) / vre_sin (angle);
    
    vre_vector_scale (&vertexNormal, ((vre_int64)stroke_width * inv_sin)>>16);
    
    //
    // Offset Inner joint
    //
    vre_vector_scale( pPrevNormal, stroke_width );
    vre_vector_scale( pNextNormal, stroke_width );
    
    if ( vre_vector_is_ahead ( pPrevNormal, pNextNormal )  )
    {
        innerJoint.x = pCorner->x - vertexNormal.x;
        innerJoint.y = pCorner->y - vertexNormal.y;
        
        outerJoint[0].x = pCorner->x + pPrevNormal->x;
        outerJoint[0].y = pCorner->y + pPrevNormal->y;
        
        outerJoint[1].x = pCorner->x + pNextNormal->x;
        outerJoint[1].y = pCorner->y + pNextNormal->y;
        
        vres = vre_polygon_add_points ( pPolygonA, &innerJoint, 1 );
        vres = vre_polygon_add_points ( pPolygonB, outerJoint, 2 );
    }
    else
    {
        innerJoint.x = pCorner->x + vertexNormal.x;
        innerJoint.y = pCorner->y + vertexNormal.y;
        
        outerJoint[0].x = pCorner->x - pPrevNormal->x;
        outerJoint[0].y = pCorner->y - pPrevNormal->y;
        
        outerJoint[1].x = pCorner->x - pNextNormal->x;
        outerJoint[1].y = pCorner->y - pNextNormal->y;
        
        vres = vre_polygon_add_points ( pPolygonB, &innerJoint, 1 );
        vres = vre_polygon_add_points ( pPolygonA, outerJoint, 2 );
    }
    return vres;
}



/**
    Applies a Butt end cap
 */
static vre_result buttEndCap ( vre_point *pEndPoint,
                             vre_polygon *pPolygonA,
                             vre_polygon *pPolygonB,
                             vre_point *pNormal, 
                             vre_ufix16 stroke_width )
{
    vre_result vres;
    vre_point point;
    
    vre_vector_scale( pNormal, stroke_width );
    
    point.x = pEndPoint->x + pNormal->x;
    point.y = pEndPoint->y + pNormal->y;
    vres = vre_polygon_add_points ( pPolygonB, &point, 1 );
    VRE_RETURN_IF_ERR ( vres );
    
    point.x = pEndPoint->x - pNormal->x;
    point.y = pEndPoint->y - pNormal->y;
    vres = vre_polygon_add_points ( pPolygonA, &point, 1 );
    VRE_RETURN_IF_ERR ( vres );
    
    return vres;
}   
 
static vre_result roundJoint ( vre_point *pCorner,
                               vre_polygon *pPolygonA,
                               vre_polygon *pPolygonB,
                               vre_point *pPrevNormal, 
                               vre_point *pNextNormal,
                               vre_ufix16 stroke_width )
{
    vre_result vres = VRE_ERR_OK;
    vre_point vertexNormal;
    vre_point innerJoint;
    vre_point outerJoint[2];
    vre_int32 dot_product;
    vre_int32 inv_sin;
    vre_int32 angle;
    vre_int32 corner_length;
    
    vre_circle *pCircle;
    
    //
    // Compute vertex pseudo normal
    //
    
    vertexNormal.x = ( pPrevNormal->x + pNextNormal->x );
    vertexNormal.y = ( pPrevNormal->y + pNextNormal->y );
    
    vre_vector_scale( pPrevNormal, 1<<16 );
    vre_vector_scale( pNextNormal, 1<<16 );
    
    //
    // Use some trigonometry to find intersection point for inner joint.
    // ( same length can be used for mitter joint ).
    //
    
    dot_product = vre_vector_dot_product (pPrevNormal, pNextNormal);
    
    // TODO Use same angle for everything...
    //angle = vre_vector_ccw_angle ( pPrevNormal, pNextNormal );
    angle = (vre_pi - vre_arccos (dot_product)) / 2;        
    inv_sin = ((vre_uint32) 0xffffffff) / vre_sin (angle);
    
    corner_length = ((vre_int64)stroke_width * inv_sin)>>16;
    vre_vector_scale (&vertexNormal, corner_length);
    
    //
    // Offset Inner joint
    //
    vre_vector_scale( pPrevNormal, stroke_width );
    vre_vector_scale( pNextNormal, stroke_width );
    
    if ( vre_vector_is_ahead ( pPrevNormal, pNextNormal )  )
    {
        innerJoint.x = pCorner->x - vertexNormal.x;
        innerJoint.y = pCorner->y - vertexNormal.y;
        
        outerJoint[0].x = pCorner->x + pPrevNormal->x;
        outerJoint[0].y = pCorner->y + pPrevNormal->y;
        
        outerJoint[1].x = pCorner->x + pNextNormal->x;
        outerJoint[1].y = pCorner->y + pNextNormal->y;
        
        vre_circle_create( &pCircle, 
                           &outerJoint[0], 
                           &outerJoint[1], 
                           corner_length );
        
        vre_circle_flatten( pCircle, 0, pPolygonB, 65536 );
        vre_circle_destroy( pCircle );
        
        vres = vre_polygon_add_points ( pPolygonA, &innerJoint, 1 );
    }
    else
    {
        innerJoint.x = pCorner->x + vertexNormal.x;
        innerJoint.y = pCorner->y + vertexNormal.y;
        
        outerJoint[0].x = pCorner->x - pPrevNormal->x;
        outerJoint[0].y = pCorner->y - pPrevNormal->y;
        
        outerJoint[1].x = pCorner->x - pNextNormal->x;
        outerJoint[1].y = pCorner->y - pNextNormal->y;
        
        vre_circle_create( &pCircle, 
                           &outerJoint[0], 
                           &outerJoint[1], 
                           -corner_length );
        
        vre_circle_flatten( pCircle, 0, pPolygonA, 65536 );
        vre_circle_destroy( pCircle );
                
        vres = vre_polygon_add_points ( pPolygonB, &innerJoint, 1 );
    }
    return vres;
}