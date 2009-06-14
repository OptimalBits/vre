/*
 *  vre_stroke.h
 *  vre - Vector Rendering Engine.
 *
 *  Created by Manuel Astudillo on 2008-02-13.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_defs.h"
#include "vre_polygon.h"

typedef struct vre_stroke vre_stroke;

typedef vre_result ( *vre_stroke_caps_func )  ( vre_point p0, vre_point p1 );
typedef vre_result ( *vre_stroke_joint_func ) ( vre_point p0, vre_point p1 );

vre_result vre_create_stroke ( vre_stroke **pStroke,
                               vre_stroke_caps_func pCapsFunc,
                               vre_stroke_joint_func pJointFunc,
                               vre_point startPoint );

void vre_stroke_destroy ( vre_stroke *pStroke );

vre_result vre_stroke_add_line ( vre_stroke *pStroke, 
                                 vre_point x0 );

vre_result vre_stroke_add_lines ( vre_stroke *pStroke, 
                                  vre_point  *pPoints, 
                                  vre_uint32 num_points );

vre_result vre_stroke_add_quad_bezier ( vre_stroke *pStroke,
                                        vre_point x0,
                                        vre_point x1 );

vre_result vre_stroke_add_cubic_bezier ( vre_stroke *pStroke,
                                         vre_point x0,
                                         vre_point x1,
                                         vre_point x2 );

vre_result vre_stroke_end ( vre_stroke *pStroke );

vre_result vre_stroke_close ( vre_stroke *pStroke );


/**
    Generates a polygon containing the stroke.
   
 */
void vre_stroke_get_polygon ( vre_stroke *pStroke,
                              vre_polygon **ppPolygon );



                          


