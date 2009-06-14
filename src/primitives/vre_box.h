/*
 *  vre_box.h
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-06-21.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef VRE_BOX_H
#define VRE_BOX_H

#include "vre_defs.h"
#include "vre_context.h"
#include "vre_render.h"

typedef struct vre_box vre_box;

vre_result vre_box_create ( vre_box **ppPath,
                            vre_style *pStyle,
                            vre_rectangle *pRect );

void vre_box_destroy ( vre_box *pPath );


vre_result vre_box_prepare ( vre_box *pPath,
                             vre_mat3x3 *pMat,
                             vre_context *pContext );

/**
 Renders the box into the specified tile object.
 
 @param 
 
 */
vre_result vre_box_render ( vre_box *pBox, 
                            vre_render *pRender, 
                            vre_mat3x3 *pMat,
                            vre_tile *pTile );

#endif




