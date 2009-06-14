/*
 *  vre_style.h
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-18.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_defs.h"

#ifndef VRE_STYLE_H
#define VRE_STYLE_H

#define VRE_FILL_TYPE_NONE  0
#define VRE_FILL_TYPE_SOLID 1
#define VRE_FILL_TYPE_LINEAR_GRADIENT   2
#define VRE_FILL_TYPE_RADIAL_GRADIENT   3

#define VRE_STROKE_TYPE_NONE    0
#define VRE_STROKE_TYPE_SOLID   1
#define VRE_STROKE_TYPE_DASHED  2

typedef struct vre_style
{
    vre_uint8  fill_type;
    vre_uint8  stroke_type;
    
    vre_uint32  fg_color; //packed format (argb 32bits)
    vre_uint32  bg_color; //packed format (argb 32bits)
    
    vre_uint32  stroke_width;
    vre_uint32  stroke_color; // packed format (argb 32bits)
} vre_style;


void vre_style_init_defaults ( vre_style *pStyle );

#endif
