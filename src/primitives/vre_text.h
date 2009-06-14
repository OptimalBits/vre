/*
 *  vre_text.h
 *  
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006. All rights reserved.
 *
 */

#ifndef VRE_TEXT_TYPE
#define VRE_TEXT_TYPE

typedef struct vre_text vre_text;

#endif


#ifndef VRE_TEXT_H
#define VRE_TEXT_H

#include "vre_defs.h"
#include "vre_font.h"
#include "vre_context.h"
#include "vre_matrix.h"
#include "vre_polygon.h"

typedef enum vre_char_type 
{
    VRE_CHAR_ASCII = 0,
    VRE_CHAR_UTF8,
    VRE_CHAR_UTF16,
    VRE_CHAR_UTF32
} vre_char_type;

vre_result vre_text_create ( vre_text **ppText,
                             vre_font *pFont,
                             vre_style *pStyle,
                             vre_mat3x3 *pMat,
                             vre_char_type type);

void vre_text_destroy ( vre_text *pText );

void vre_text_set_cursor ( vre_text *pTex, 
                           vre_int32 x, 
                           vre_int32 y );

vre_result vre_text_add_char ( vre_text *pText,
                               void *pChar);

vre_result vre_text_add_string ( vre_text *pText,
                                 void *pString,
                                 vre_uint32 length );

vre_result vre_text_newline ( vre_text *pText );

void vre_text_set_cursor ( vre_text *pText, vre_int32 x, vre_int32 y);

void vre_text_set_style ( vre_text *pText, vre_style *pStyle );

vre_result vre_text_prepare ( vre_text *pText,
                              vre_mat3x3 *pMat,
                              vre_context *pContext );
/*
Deprecated
vre_result vre_text_draw ( vre_text *pText, 
                           vre_tile *pTile,
                           vre_mat3x3 *pMat);
*/
#endif

