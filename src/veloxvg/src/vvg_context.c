/*
 *  vvg_context.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2007-12-15.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "openvg.h"
#include "vvg_context.h"
#include "vvg_path.h"
#include "vre_defs.h"

void vvgSetTargetTile ( vre_tile *pTile )
{
    veloxContext.pTargetTile = pTile;
}

void vvgInitContext ( void )
{    
    vre_result vres;
    
    //
    // Set to null some pointers.
    //
    veloxContext.pClipOutPoly = 0;
    veloxContext.pClipTmpPoly = 0;

    veloxContext.pTargetTile = 0;
    
    
    //
    // Init transformation matrices
    //
    vre_matrix_identity ( &veloxContext.matrix.pathMatrix );
    vre_matrix_identity ( &veloxContext.matrix.imageMatrix );
    vre_matrix_identity ( &veloxContext.matrix.fillPaintMatrix );
    vre_matrix_identity ( &veloxContext.matrix.strokePaintMatrix );
    
    veloxContext.matrix.pCurrent = &veloxContext.matrix.pathMatrix;
    veloxContext.matrix.mode = VG_MATRIX_PATH_USER_TO_SURFACE;

    veloxContext.pClipOutPoly = 0;
    veloxContext.pClipTmpPoly = 0;
    
    vres = vre_polygon_createEx( &veloxContext.pClipOutPoly);
    if ( vres != VRE_ERR_OK )
    {
        return;
    }
    
    vres = vre_polygon_createEx( &veloxContext.pClipTmpPoly );
    if ( vres != VRE_ERR_OK )
    {
        return;
    }
}

void vvgDeInitContext ( void )
{
    
    
}

void vgFinish ( void )
{
    vre_uint32 i;
    vre_rectangle clipRect;
    
    vvg_path **ppPaths;
    
    ppPaths = veloxContext.ppPaths;
    
    clipRect.x = veloxContext.pTargetTile->x << 16;
    clipRect.y = veloxContext.pTargetTile->y << 16;
    clipRect.w = veloxContext.pTargetTile->w << 16;
    clipRect.h = veloxContext.pTargetTile->h << 16;
    
    if ( veloxContext.pTargetTile )
    {
        for ( i = 0; i < veloxContext.numPaths; i++ )
        {
            //
            // Render every path for every Scissor rectangle
            // ( Suggestion: use Rtree for speed improvements ).
            //
            
            vvg_pathRender ( ppPaths[i], &clipRect );
        }
    }
}

void vgFlush ( void )
{
    vgFinish ();
}

VGErrorCode vgGetError ( void )
{
    return veloxContext.errorCode;
}

