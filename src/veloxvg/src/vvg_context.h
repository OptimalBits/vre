/*
 *  veloxvg.h
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2007-12-08.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef VVG_H
#define VVG_H

#include "openvg.h"
#include "vre_defs.h"
#include "vre_polygon.h"
#include "vvg_path.h"
#include "vvg_matrix.h"

typedef struct veloxvg_context
{
    VGErrorCode errorCode;
    
    veloxvg_matrix matrix;
    
    vre_uint32     numPaths;
    vvg_path       *ppPaths[256]; // todo, use vre_array.
    vre_tile       *pTargetTile;
    
    vre_polygon    *pClipOutPoly;
    vre_polygon    *pClipTmpPoly;
    
} veloxvg_context;


// The velox VG context instance.
veloxvg_context veloxContext;

void vvgSetTargetTile ( vre_tile *pTile );

void vvgInitContext ( void );


#endif

