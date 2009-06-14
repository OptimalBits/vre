/*
 *  vre_font.h
 *  
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006 Curve. All rights reserved.
 *
 */

#ifndef VRE_FONT_H
#define VRE_FONT_H

#include "vre_defs.h"
#include "vre_glyph.h"


/**
    Interface for font loaders and managers.
    A font loader should return a valid vre_font object.
*/

typedef enum 
{
    VRE_FONT_DIRECTION_LR,
    VRE_FONT_DIRECTION_RL,
    VRE_FONT_DIRECTION_UD
    
} vre_font_direction;


typedef struct 
{
//    vre_font_metrics *pMetrics;
    
    vre_uint32 direction;
    vre_uint16 upem;
    vre_fix16  line_gap;

	vre_int16 ascender;
	vre_int16 descender;
    
} vre_font_attr;

typedef vre_uint32 (*VRE_MAP_ENCODING)( void*, vre_uint32, vre_uint32);
typedef vre_result (*VRE_GET_GLYPH)( void*, vre_meta_glyph**, vre_uint32);
typedef void (*VRE_APPLY_KERNING) ( void*, 
                                    vre_uint32 left, vre_uint32 right, 
                                    vre_int32* dx, vre_int32* dy);

typedef vre_uint32 (*VRE_GET_ADVANCE_WIDTH) ( void*, vre_uint32 glyph_id);
typedef vre_uint32 (*VRE_GET_ADVANCE_HEIGHT) ( void*, vre_uint32 glyph_id);

typedef struct vre_font_interface
{
    VRE_MAP_ENCODING map_encoding;
    VRE_GET_GLYPH get_glyph;
    
    VRE_APPLY_KERNING apply_kerning;
    
    VRE_GET_ADVANCE_WIDTH get_advance_width;
    VRE_GET_ADVANCE_HEIGHT get_advance_height;
} vre_font_interface;

//
// Interface functions to underlying font format
//
typedef struct vre_font vre_font;

vre_result vre_create_font ( vre_font **ppFont );
void vre_destroy_font ( vre_font *pFont );

void vre_font_set_ul_font (vre_font *pFont, void *pULFont);
vre_font_interface *vre_font_get_interface (vre_font *pFont);
vre_font_attr *vre_font_get_attributes (vre_font *pFont);

//
// Interface Wrappers
//
vre_uint32 vre_font_map_encoding( vre_font *pFont,
                                  vre_int32 encoding,
                                  vre_uint32 character );

vre_result vre_font_get_glyph( vre_font* pFont, 
                               vre_meta_glyph** ppGlyph, 
                               vre_uint32 id);

void vre_font_apply_kerning( vre_font *pFont, 
                             vre_uint32 left, vre_uint32 right,
                             vre_int32 *dx, vre_int32 *dy );

vre_uint32 vre_font_get_advance_width( vre_font *pFont,
                                       vre_uint32 glyph_id );

vre_uint32 vre_font_get_advance_height( vre_font *pFont,
                                        vre_uint32 glyph_id );

//vre_font_data vre_get_font_data ( vre_font *pFont );

/*
vre_result vre_get_glyph ( vre_font *pFont, 
                           vre_glyph **ppGlyph, 
                           vre_uint32 glyphId );
*/

typedef struct vre_font_encoding
{
    vre_int dummy;    
} vre_font_encoding;

void vre_font_get_encodings ();

vre_uint32 vre_font_get_default_encoding ( vre_font *pFont );
void vre_font_select_encoding ( vre_font *pFont, vre_uint32 encoding );

#endif




