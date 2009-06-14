/*
 *  vre_path.h
 *  vre_test
 *
 *  Created by Manuel Astudillo on 2006-11-12.
 *  Copyright 2006 Curved Graphics. All rights reserved.
 *
 */

#ifndef VRE_PATH_H
#define VRE_PATH_H

#include "vre_defs.h"
#include "vre_context.h"
#include "vre_bezier.h"
#include "vre_render.h"

typedef struct vre_path vre_path;

vre_result vre_path_create ( vre_path **ppPath,
                             vre_style *pStyle );

void vre_path_destroy ( vre_path *pPath );

vre_result vre_path_add_move_to ( vre_path *pPath, vre_fix16 x, vre_fix16 y );

vre_result vre_path_add_line ( vre_path *pPath, 
                              vre_bool relative, 
                              vre_fix16 x, 
                              vre_fix16 y );

vre_result vre_path_add_bezier (vre_path *pPath, vre_bezier *pBezier );

/**
    Adds a cubic bezier curve.
 
    @params pPath A pointer to a path object.
    @params pPoints Points for the control polygon. Should be 3 points.
 
 */
vre_result vre_path_add_cubic ( vre_path *pPath, 
                                vre_bool relative, 
                                vre_point *pPoints );


vre_result vre_path_add_arc ( vre_path *pPath, vre_point *pStart, 
                              vre_point *pVertAxis, vre_point *pHorzAxis, 
                              vre_fix16 angle );

/**
    Ends a path. If no stroking is enabled, this is the same as vre_path_close.
 
    @param pPath A pointer to a path object.
 
    @return a vre error code.
 */
vre_result vre_path_end ( vre_path *pPath );

/**
 Closes a path. If stroking is enabled the path will be closed with the stroke.
 
 @param pPath A pointer to a path object.
 
 @return a vre error code.
 */
vre_result vre_path_close ( vre_path *pPath );


vre_result vre_path_prepare ( vre_path *pPath,
                              vre_mat3x3 *pMat,
                              vre_context *pContext );

/**
    Renders the path into the specified tile object.
 
    @param 
 
 */
vre_result vre_path_render ( vre_path *pPath,
                             vre_render *pRenderer,
                             vre_mat3x3 *pMat, 
                             vre_tile *pTile );

#endif



