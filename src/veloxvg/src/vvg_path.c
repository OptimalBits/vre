
/*
 *  vvg_path.c
 *  Velox VG
 *
 *  Created by Manuel Astudillo on 2007-11-04.
 *  Copyright 2007  blablah Software. All rights reserved.
 *
 */

#include "openvg.h"
#include "vvg_context.h" 
#include "vre_array.h"
#include "vre_stroke.h"
#include "vre_polygon.h"
#include "vre_bezier.h"
#include "vre_assert.h"


#define STROKE_WIDTH 5 << 16

extern veloxvg_context veloxContext;

static const VGubyte numCoordsLut[]={ 0, 2, 2, 1, 1, 4, 6, 2, 4, 5, 5, 5, 5 };

struct vvg_path
{
    VGint pathFormat;
    VGint dataSize;
    VGfloat scale; 
    VGfloat bias;

    VGbitfield capabilities;
    
    vre_array *pCommands;
    vre_array *pCoordinates;
    
    VGPathDatatype dataType;
    
    vre_polygon *pPolygon;
    
    vre_style   style;
    
    // Stroking
   /* vre_polygon *pStroke;
    
    vre_point firstNormal;
    vre_point firstInnerPoint; // eventually we will need to save 2 points ( since path could
                               // be given CCW instead of CW ).
    vre_point prevNormal;
    vre_point prevInnerPoint;
    
    vre_polygon *pInnerStrokeEdge; // Two polygons to hold inner, respective
    vre_polygon *pOuterStrokeEdge; // outer stroke subpaths.
    */
    vre_stroke *pStroke;
    
    // 
    vre_point  p;
    vre_point  s;
    vre_point  o;
    
    vre_point  st; // Start transformed
    vre_point  t;  // Last transformed
    
    vre_bool firstSubPath;
    
    vre_bezier *pBezierQuad;
    vre_bezier *pBezierCubic;
};

void setError ( VGErrorCode errorCode );

static VGint getDataTypeSize ( VGPathDatatype dataType );
static void destroyPath ( vvg_path *pPath );
static vvg_path *getPath ( VGPath path );
static void getCoordinates ( VGubyte *pData, VGPathDatatype dataType, 
                            VGuint numCoords, vre_fix16 *pCoords );

static void handleClosePath ( vvg_path *pPath );
static void handleMove ( vvg_path *pPath, vre_fix16 x, vre_fix16 y );
static void handleLine ( vvg_path *pPath, vre_bool relative, 
                        vre_fix16 x, vre_fix16 y );
static void handleHLine ( vvg_path *pPath, vre_bool relative, vre_fix16 x );
static void handleVLine ( vvg_path *pPath, vre_bool relative, vre_fix16 y );
static void handleQuad ( vvg_path *pPath, vre_bool relative, 
                        vre_fix16 x0, vre_fix16 y0,
                        vre_fix16 x1, vre_fix16 y1 );
static void handleCubic ( vvg_path *pPath, vre_bool relative, 
                         vre_fix16 x0, vre_fix16 y0,
                         vre_fix16 x1, vre_fix16 y1,
                         vre_fix16 x2, vre_fix16 y2 );


VG_API_CALL VGPath vgCreatePath(VGint pathFormat,
                                VGPathDatatype datatype,
                                VGfloat scale, VGfloat bias,
                                VGint segmentCapacityHint,
                                VGint coordCapacityHint,
                                VGbitfield capabilities)
{
    vre_result  vres;
    vvg_path    *pPath;
    
    //
    // Alloc memory for new Path structure
    //
    
    pPath = (vvg_path*) vre_malloc ( sizeof ( vvg_path ) );
    if ( pPath == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset ( pPath, sizeof ( vvg_path ), 0 );
    
    veloxContext.ppPaths[veloxContext.numPaths++] = pPath;
    
    pPath->pathFormat = pathFormat;
    pPath->dataSize = getDataTypeSize ( datatype );
    pPath->dataType = datatype;
    pPath->scale = scale;
    pPath->bias = bias;
    pPath->capabilities = capabilities;
    
    
    for (;;) 
    {
        //
        // Create dynamic arrays.
        //
        
        // TODO: round to ceil power of 2.
        if ( ( segmentCapacityHint == 0 ) || 
             ( segmentCapacityHint > 1024 ) )
        {
            segmentCapacityHint = 1024;
        };
        
        if ( ( coordCapacityHint == 0 ) || 
             ( coordCapacityHint > 1024 ) )
        {
            coordCapacityHint = 1024;
        }; 
        
        vres = vre_array_create ( segmentCapacityHint,
                                  &pPath->pCommands );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_array_create ( coordCapacityHint,
                                  &pPath->pCoordinates);
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        //
        // Create Polygons
        //
        vres = vre_polygon_createEx ( &pPath->pPolygon );
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        vres = vre_array_create ( coordCapacityHint,
                                 &pPath->pCoordinates);
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        //
        // Create Bezier 
        //
        
        vres = vre_bezier_create ( &pPath->pBezierQuad, 3 );
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        vres = vre_bezier_create ( &pPath->pBezierCubic, 4 );
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        destroyPath ( pPath );
        veloxContext.numPaths--;
        veloxContext.errorCode = VG_OUT_OF_MEMORY_ERROR;
        return -1;
    }
    
    return veloxContext.numPaths - 1;
}

VG_API_CALL void vgClearPath(VGPath path, VGbitfield capabilities);

VG_API_CALL void vgDestroyPath(VGPath path)
{
    vvg_path *pPath;
    
    pPath = getPath ( path );
    destroyPath ( pPath );
    
}

VG_API_CALL void vgRemovePathCapabilities(VGPath path,
                                          VGbitfield capabilities);

VG_API_CALL VGbitfield vgGetPathCapabilities(VGPath path);

VG_API_CALL void vgAppendPath(VGPath dstPath, VGPath srcPath);

VG_API_CALL void vgAppendPathData( VGPath dstPath,
                                   VGint numSegments,
                                   const VGubyte * pathSegments,
                                   const void * pathData)
{
    vre_result vres;
    vvg_path *pPath;
    vre_int8  *pData8;
    vre_int16 *pData16;
    vre_int32 *pData32;
    vre_uint32 index;
    vre_uint32 i,j;
    vre_uint32 numCoords;
    float *pDataF;
    
    float scale;
    float bias;
    
    pPath = getPath ( dstPath );
    if ( pPath )
    {
        vres = vre_array_add ( pPath->pCommands, 
                               (void*) pathSegments,
                               numSegments );
        if ( vres != VRE_ERR_OK )
        {
            setError ( VG_OUT_OF_MEMORY_ERROR );
            return;
        }
        
        index = 0;
        scale = pPath->scale;
        bias = pPath->bias;
        
        pData8 = ( vre_int8* ) pathData;
        pData16 = ( vre_int16* ) pathData;
        pData32 = ( vre_int32* ) pathData;
        pDataF = ( float* ) pathData;
        
        for ( i = numSegments; i != 0; i-- )
        {
            vre_uint8 cmd;
            float val;
            vre_fix16 coords[6];
            vre_fix16 *pCoords;
            
            cmd = pathSegments[index++];
            numCoords = numCoordsLut[cmd>>1];
            
            pCoords = coords;
            
            switch ( pPath->dataType )
            {
                case VG_PATH_DATATYPE_S_8: 
                    for ( j = numCoords; j != 0; j--)
                    {
                        val = (float) (*pData8++);
                        val = val * scale + bias;
                        
                        *pCoords++ = ( vre_fix16 ) ( val * 65536 );
                    } 
                    break;
                
                case VG_PATH_DATATYPE_S_16:
                    for ( j = numCoords; j != 0; j--)
                    {
                        val = (float) (*pData16++);
                        val = val * scale + bias;
                        
                        *pCoords++ = ( vre_fix16 ) ( val * 65536 );
                    } 
                    break;
                
                case VG_PATH_DATATYPE_S_32:
                    for ( j = numCoords; j != 0; j--)
                    {
                        val = (float) (*pData32++);
                        val = val * scale + bias;
                        
                        *pCoords++ = ( vre_fix16 ) ( val * 65536 );
                    }
                    break;
                
                case VG_PATH_DATATYPE_F: 
                    for ( j = numCoords; j != 0; j--)
                    {
                        val = (float) (*pDataF++);
                        val = val * scale + bias;
                        
                        *pCoords++ = ( vre_fix16 ) ( val * 65536 );
                    } 
                    break;
            }
            
            vres = vre_array_add ( pPath->pCoordinates, 
                                   coords, 
                                   numCoords * sizeof ( vre_fix16 ) );
        }
    }
}

VG_API_CALL void vgModifyPathCoords(VGPath dstPath, VGint startIndex,
                                    VGint numSegments,
                                    const void * pathData);

VG_API_CALL void vgTransformPath(VGPath dstPath, VGPath srcPath);

VG_API_CALL VGboolean vgInterpolatePath(VGPath dstPath,
                                        VGPath startPath,
                                        VGPath endPath,
                                        VGfloat amount);

VG_API_CALL VGfloat vgPathLength(VGPath path,
                                 VGint startSegment, VGint numSegments);

VG_API_CALL void vgPointAlongPath(VGPath path,
                                  VGint startSegment, VGint numSegments,
                                  VGfloat distance,
                                  VGfloat * x, VGfloat * y,
                                  VGfloat * tangentX, VGfloat * tangentY);

VG_API_CALL void vgPathBounds(VGPath path,
                              VGfloat * minX, VGfloat * minY,
                              VGfloat * width, VGfloat * height);

VG_API_CALL void vgPathTransformedBounds(VGPath path,
                                         VGfloat * minX, VGfloat * minY,
                                         VGfloat * width, VGfloat * height);


VG_API_CALL void vgDrawPath(VGPath path, VGbitfield paintModes)
{
    vvg_path *pPath;
    vre_array *pCommands;
    vre_array *pCoordinates;
 //   vre_polygon *pPolygon;
    vre_uint32 numElems;
    
    vre_fix16  *pCoords;
    vre_uint32 numAvailableCoords;
    
    vre_uint8 *pCommandsBuffer;
    
    vre_uint32 i, j;
    
    pPath = getPath ( path );
    if ( pPath )
    {
        //
        // Reset the polygons for this path
        //
        vre_polygon_reset ( pPath->pPolygon );
        
        //
        // Reset Stroke ??
        //
        
        pPath->firstSubPath = 1;
    
        // Main polygon
        pCommands = pPath->pCommands;
        pCoordinates = pPath->pCoordinates;
        
        vre_array_get_elems_start ( pCommands, 
                                    (void**) &pCommandsBuffer,
                                    &numElems );
        
        // TODO: Maybe we should re-introduce the concept of element size...
        // Otherwise, a element could be broken between two chunks and thats evil.
        vre_array_get_elems_start ( pCoordinates,
                                    (void*) &pCoords,
                                    &numAvailableCoords );
        
        vre_assert ( (numAvailableCoords & (sizeof ( vre_fix16 ) - 1)) == 0 );
        numAvailableCoords = numAvailableCoords / sizeof ( vre_fix16 );
         
        do
        {
            for ( i = numElems; i != 0; i-- )
            {
                vre_uint8 cmd;
                vre_uint32 relative;
                vre_uint32 numCoords;
                vre_fix16 coords[6];
                
                cmd = *pCommandsBuffer++;
                
                numCoords = numCoordsLut[cmd>>1];
            
                // Get coords
                if ( numAvailableCoords >= numCoords )
                {
                    for ( j = 0; j < numCoords; j++ )
                    {
                        coords[j] = *pCoords++;
                    }
                    numAvailableCoords -= numCoords;
                }
                else
                {
                    for ( j = 0; j < numCoords; j++ )
                    {
                        coords[j] = *pCoords++;
                    }
                    numCoords -= numAvailableCoords;
                    
                    vre_array_get_elems_iter ( pCoordinates,
                                               (void*) &pCoords,
                                               &numAvailableCoords );
                    
                    vre_assert ( numAvailableCoords > numCoords );
                    vre_assert ( numAvailableCoords & (sizeof ( vre_fix16 ) - 1) == 0 );
                    numAvailableCoords = numAvailableCoords / sizeof ( vre_fix16 );
                }

                                        
                relative = cmd & 0x01;
                
                switch ( cmd & 0x1e )
                {
                    case VG_CLOSE_PATH: 
                        handleClosePath ( pPath ); 
                        break;
                    case VG_MOVE_TO: 
                        handleMove ( pPath, coords[0], coords[1] ); 
                        break;
                    case VG_LINE_TO: 
                        handleLine ( pPath, relative, coords[0], coords[1] ); 
                        break;
                    case VG_HLINE_TO: 
                        handleLine ( pPath, relative, coords[0], 0 ); 
                        break;
                    case VG_VLINE_TO: 
                        handleLine ( pPath, relative, 0, coords[0] ); 
                        break;
                    case VG_QUAD_TO: 
                        handleQuad  ( pPath, relative,
                                     coords[0], coords[1], 
                                     coords[2], coords[3] ); 
                        break;
                    case VG_CUBIC_TO: 
                        handleCubic ( pPath, relative,
                                     coords[0], coords[1], 
                                     coords[2], coords[3],
                                     coords[4], coords[5] ); 
                        break;
                    case VG_SQUAD_TO:
                    case VG_SCUBIC_TO:
                    case VG_SCCWARC_TO:
                    case VG_SCWARC_TO:
                    case VG_LCCWARC_TO:
                    case VG_LCWARC_TO:
                        break;
                    //default:
                       // return VG_ILLEGAL_ARGUMENT_ERROR;
                }
            }
            
            vre_array_get_elems_iter ( pCommands, 
                                      (void*) &pCommandsBuffer,
                                      &numElems);
        } while ( numElems != 0 );
        
        // Stroke and Caps
    }
    
}


void setError ( VGErrorCode errorCode )
{
}

/**
    Parses OpenVG commands, and creates a polygon.
 
    All coordinates are always converted the internal reprentation, 
    16.16 fix point.

*/
static VGErrorCode parseCommands ( VGPath path,
                                   VGint numSegments, 
                                   const VGubyte * pathSegments,
                                   const void * pathData )
{
    
    VGubyte *pData;
    
    vre_fix16 coords[6];

    vvg_path *pPath;
    
    VGint i;
    VGint dataIndex;

    if ( path >= veloxContext.numPaths )
    {
        return VG_BAD_HANDLE_ERROR;
    }
    
    pPath = veloxContext.ppPaths[path];
    pData = ( VGubyte* ) pathData;

    for ( i = 0; i < numSegments; i++ )
    {
        VGint numCoords;
        VGint relative;
        VGubyte cmd;
        
        cmd = pathSegments[i];
        
        numCoords = numCoordsLut[cmd>>1];
        
        getCoordinates ( pData, pPath->dataType, numCoords, coords );
        
        relative = cmd & 0x01;
        
        switch ( cmd & 0x1e )
        {
            case VG_CLOSE_PATH: 
                        handleClosePath ( pPath ); 
                        break;
            case VG_MOVE_TO: 
                        handleMove ( pPath, coords[0], coords[1] ); 
                        break;
            case VG_LINE_TO: 
                        handleLine ( pPath, relative, coords[0], coords[1] ); 
                        break;
            case VG_HLINE_TO: 
                        handleHLine ( pPath, relative, coords[0] );
                        break;
            case VG_VLINE_TO: 
                        handleVLine ( pPath, relative, coords[0] );
                        break;
            case VG_QUAD_TO: 
                        handleQuad  ( pPath, relative,
                                      coords[0], coords[1], 
                                      coords[2], coords[3] ); 
                        break;
            case VG_CUBIC_TO: 
                        handleCubic ( pPath, relative,
                                      coords[0], coords[1], 
                                      coords[2], coords[3],
                                      coords[4], coords[5] ); 
                        break;
            case VG_SQUAD_TO:
            case VG_SCUBIC_TO:
            case VG_SCCWARC_TO:
            case VG_SCWARC_TO:
            case VG_LCCWARC_TO:
            case VG_LCWARC_TO:
            default:
                return VG_ILLEGAL_ARGUMENT_ERROR;
        }
    
        // dataIndex += numCoords * pPath;
        dataIndex += numCoords;
    }
    
    return VG_NO_ERROR;
}

void vvg_pathRender ( vvg_path *pPath, vre_rectangle *pClipRect )
{
    
    
    //
    // Render main polygon
    //
    
    //
    // Clip polygon.
    // ( TODO: Use bounding rectangle to decide if 
    //   clipping must be performed at all )
    
    if ( vre_polygon_get_num_vertices ( pPath->pPolygon ) > 2 )
    {
        vre_polygon_reset ( veloxContext.pClipOutPoly );
        vre_polygon_reset ( veloxContext.pClipTmpPoly );
        
        vre_clip_poly_rectangle ( pPath->pPolygon,
                                  pClipRect,
                                  veloxContext.pClipOutPoly,
                                  veloxContext.pClipTmpPoly );
        
        // Hack just to make it work fast
        pPath->style.fg_color = 0xffffffff;
        vre_draw_polygon_concave_aa  ( veloxContext.pClipOutPoly, 
                                       veloxContext.pTargetTile,
                                       &pPath->style );
    }
    
    // Render strokes ( render an array of polygons representing the strokes );
    {
        vre_polygon *pStrokePoly;
        
        vre_stroke_get_polygon ( pPath->pStroke, &pStrokePoly );
    
        vre_polygon_reset ( veloxContext.pClipOutPoly );
        vre_polygon_reset ( veloxContext.pClipTmpPoly );
    
        vre_clip_poly_rectangle ( pStrokePoly,
                                  pClipRect,
                                  veloxContext.pClipOutPoly,
                                  veloxContext.pClipTmpPoly );
    
        pPath->style.fg_color = 0xff000000;
        vre_draw_polygon_concave_aa  ( veloxContext.pClipOutPoly, 
                                      veloxContext.pTargetTile,
                                      &pPath->style );
    }
    
}


static void updateOrigin ( vvg_path *pPath, vre_bool relative, vre_fix16 x, vre_fix16 y )
{
    if ( relative )
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

static void handleClosePath ( vvg_path *pPath )
{
    vre_result vres;
    
    pPath->p = pPath->o = pPath->s;
    
    vre_polygon_end_contour ( pPath->pPolygon );
    
    vres = vre_stroke_close ( pPath->pStroke );
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
    
}

static void handleMove ( vvg_path *pPath, vre_fix16 x, vre_fix16 y )
{
    vre_result vres;
    vre_point dstPoint;
    
    pPath->o.x = x;
    pPath->o.y = y;

    pPath->s = pPath->p = pPath->o;
    
    if ( ! pPath->firstSubPath )
    {
        vre_polygon_end_contour ( pPath->pPolygon );
        vre_polygon_start_contour ( pPath->pPolygon );
    }
    else
    {
        vre_polygon_start_contour ( pPath->pPolygon );
        pPath->firstSubPath = 0;
    }    
    
    // TODO: Use a macros for this transform
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                           &pPath->o, 
                           &dstPoint, 
                           1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    if ( vres != VRE_ERR_OK )
    {
    //    setError ( vvg_translateError (vres) );
    }
    
    pPath->st = pPath->t = dstPoint;
    
    // Create Stroke
    vres = vre_stroke_create ( &pPath->pStroke, 5 << 16, 0, 0, &pPath->st );
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
}


static void handleLine ( vvg_path *pPath, vre_bool relative, 
                         vre_fix16 x, vre_fix16 y )
{
    vre_result vres;
    vre_point dstPoint;
    
    updateOrigin ( pPath, relative, x, y );
    
    pPath->p = pPath->o;
    
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                           &pPath->o, 
                           &dstPoint, 
                           1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
    
    // if ( pPath->stroking )
    vres = vre_stroke_add_line ( pPath->pStroke, dstPoint );
    
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
    
    pPath->t = dstPoint;
}

static void handleHLine ( vvg_path *pPath, vre_bool relative, vre_fix16 x )
{
    vre_result vres;
    vre_point dstPoint;
    
    if ( relative )
    {
        pPath->o.x += x;
    }
    else
    {
        pPath->o.x = x;
    }
    
    pPath->p = pPath->o;
    
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                          &pPath->o, 
                          &dstPoint, 
                          1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
    
    // if ( pPath->stroking )
    vres = vre_stroke_add_line ( pPath->pStroke, dstPoint );
    
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }
    
    pPath->t = dstPoint;
}

static void handleVLine ( vvg_path *pPath, vre_bool relative, vre_fix16 y )
{
    vre_result vres;
    vre_point dstPoint;
    
    if ( relative )
    {
        pPath->o.y += y;
    }
    else
    {
        pPath->o.y = y;
    }
    
    pPath->p = pPath->o;
    
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                          &pPath->o, 
                          &dstPoint, 
                          1 );
    
    vres = vre_polygon_add_point ( pPath->pPolygon, &dstPoint );
    if ( vres != VRE_ERR_OK )
    {
        // setError ( vvg_translateError (vres) );
    }
    
    // if ( pPath->stroking )
    vres = vre_stroke_add_line ( pPath->pStroke, dstPoint );
    
    if ( vres != VRE_ERR_OK )
    {
        //    setError ( vvg_translateError (vres) );
    }    
    pPath->t = dstPoint;    
}

static void handleQuad ( vvg_path *pPath, vre_bool relative, 
                         vre_fix16 x0, vre_fix16 y0,
                         vre_fix16 x1, vre_fix16 y1 )
{
    vre_result vres;
    
    vre_point srcPoints[3];
    vre_point dstPoints[3];
    
    pPath->p.x = x0;
    pPath->p.y = y0;    
    
    srcPoints[0] = pPath->o;
    
    updateOrigin ( pPath, relative, x0, y0 );
    
    srcPoints[1] = pPath->o;
    
    updateOrigin ( pPath, relative, x1, y1 );
    
    srcPoints[2] = pPath->o;
    
    // Transform segment
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                          srcPoints, 
                          dstPoints, 
                          3 );
    
    // Flatten Bezier
    vre_bezier_set_points ( pPath->pBezierQuad, dstPoints, 3 );
    
    vres = vre_bezier_flatten ( pPath->pBezierQuad, pPath->pPolygon, 1000, 1 );
    if ( vres != VRE_ERR_OK )
    {
        // setError ( vvg_translateError (vres) );
    }
    
    vres =  vre_stroke_add_quad_bezier ( pPath->pStroke, 
                                         dstPoints[1],
                                         dstPoints[2] );
    if ( vres != VRE_ERR_OK )
    {
        // setError ( vvg_translateError (vres) );
    }
    
    pPath->t = dstPoints[2];
    
}

static void handleCubic ( vvg_path *pPath, vre_bool relative, 
                          vre_fix16 x0, vre_fix16 y0,
                          vre_fix16 x1, vre_fix16 y1,
                          vre_fix16 x2, vre_fix16 y2 )
{
    vre_result vres;
    
    vre_point srcPoints[4];
    vre_point dstPoints[4];
    
    pPath->p.x = x1;
    pPath->p.y = y1;
    
    srcPoints[0] = pPath->o;
    
    updateOrigin ( pPath, relative, x0, y0 );
    
    srcPoints[1] = pPath->o;
    
    updateOrigin ( pPath, relative, x1, y1 );
    
    srcPoints[2] = pPath->o;
    
    updateOrigin ( pPath, relative, x2, y2 );
    
    srcPoints[3] = pPath->o;
    
    vre_matrix_transform ( veloxContext.matrix.pCurrent, 
                           srcPoints, 
                           dstPoints, 
                           4 );
    
    vre_bezier_set_points ( pPath->pBezierCubic, dstPoints, 4 );
    
    vres = vre_bezier_flatten ( pPath->pBezierQuad, pPath->pPolygon, 65536, 1 );
    if ( vres != VRE_ERR_OK )
    {
        // setError ( vvg_translateError (vres) );
    }
    
    vres =  vre_stroke_add_cubic_bezier ( pPath->pStroke, 
                                          dstPoints[1],
                                          dstPoints[2],
                                          dstPoints[3] );
    if ( vres != VRE_ERR_OK ) 
    {
        // setError ( vvg_translateError (vres) );
    }
}



static void getCoordinates ( VGubyte *pData, VGPathDatatype dataType, 
                             VGuint numCoords, vre_fix16 *pCoords )
{
    for ( ; numCoords != 0; numCoords -- )
    {
        switch ( dataType )
        {
            case VG_PATH_DATATYPE_S_8: 
                *pCoords++ = (*pData++) << 16;
                break;
            
            case VG_PATH_DATATYPE_S_16:
                *pCoords++ = (*((vre_int16*)pData)) << 16;
                pData += 2;
                break;
                
            case VG_PATH_DATATYPE_S_32:
                *pCoords++ = (*((vre_int32*)pData)) << 16;
                pData += 4;
                break;
                
            case VG_PATH_DATATYPE_F: 
                *pCoords++ = (vre_fix16) ((*((float*)pData)) * 65536.0f);
                pData += 4;
                break;
        }
    }
}

static VGint getDataTypeSize ( VGPathDatatype dataType )
{
    switch ( dataType )
    {
        case VG_PATH_DATATYPE_S_8: return 1;
        case VG_PATH_DATATYPE_S_16: return 2;
        case VG_PATH_DATATYPE_S_32: return 4;
        case VG_PATH_DATATYPE_F: return 4;
    }
    return 0;
}

static void destroyPath ( vvg_path *pPath )
{
    if ( pPath != 0 )
    {
        vre_array_destroy ( pPath->pCommands );
        vre_array_destroy ( pPath->pCoordinates );
        vre_polygon_destroy ( pPath->pPolygon );
        vre_stroke_destroy ( pPath->pStroke );
        
        vre_bezier_destroy ( pPath->pBezierQuad );
        vre_bezier_destroy ( pPath->pBezierCubic );
        vre_free ( pPath );
    }
}


static vvg_path *getPath ( VGPath path )
{
    vvg_path *pPath;
    
    if ( path < veloxContext.numPaths )
    {   
        pPath = veloxContext.ppPaths[path];
        return pPath;
    }

    veloxContext.errorCode = VG_BAD_HANDLE_ERROR;
    return 0;

}


