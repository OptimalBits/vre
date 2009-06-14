/*
 *  vre_style.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-18.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_style.h"



void vre_style_init_defaults ( vre_style *pStyle )
{
    pStyle->fill_type = VRE_FILL_TYPE_NONE;
    pStyle->stroke_type = VRE_STROKE_TYPE_NONE;
    
    pStyle->fg_color = 0xff000000;
    pStyle->bg_color = 0xffffffff;
    
    pStyle->stroke_width = 1 << 16;
    pStyle->stroke_color = 0xff808080;
    
    
}