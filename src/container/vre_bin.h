/*
 *  vre_bin.h
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-17.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef VRE_BIN_H
#define VRE_BIN_H

#include "vre_defs.h"
#include "vre_iostream.h"
#include "vre_matrix.h"
#include "vre_render.h"


typedef struct vre_bin vre_bin;

vre_result vre_bin_create ( vre_bin **ppBin );
void vre_bin_destroy ( vre_bin *pBin );

vre_result vre_bin_load ( vre_iostream *pStream, vre_bin **ppBin );

vre_result vre_bin_render ( vre_bin *pBin, 
                            vre_render *pRender, 
                            vre_mat3x3 *pMat, 
                            vre_tile *pTile );


#endif
