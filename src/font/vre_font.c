/*
 *  vre_font.c
 *  
 *
 *  Created by Manuel  Astudillo on 2006-03-30.
 *  Copyright 2006 Curve. All rights reserved.
 *
 */

#include "vre_font.h"
#include "vre_mem.h"
#include "vre_assert.h"


struct vre_font
{
    // Underlying font (only TTF supported for the moment).
    void *pULFont;
    
    vre_uint32 default_encoding;
    
    vre_font_interface interface;
    vre_font_attr attributes;
};

vre_result vre_create_font ( vre_font **ppFont )
{
    vre_font *pFont;
    
   // *ppFont = 0;
    
    pFont = VRE_TALLOC ( vre_font );
    if ( pFont == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset( pFont, sizeof ( vre_font ), 0 );
    
    *ppFont = pFont;
    
    return VRE_ERR_OK;
}

void vre_destroy_font ( vre_font *pFont )
{
    vre_free( pFont );
}

void vre_font_set_ul_font (vre_font *pFont, void *pULFont)
{
    vre_assert ( pFont != 0 );
    
    pFont->pULFont = pULFont;
}

vre_font_interface *vre_font_get_interface (vre_font *pFont)
{
    vre_assert ( pFont != 0 );
    
    return &pFont->interface;
}

vre_font_attr *vre_font_get_attributes (vre_font *pFont)
{
    vre_assert ( pFont != 0 );
    
    return &pFont->attributes;
}



/*
vre_font_data vre_get_font_data ( vre_font *pFont )
{
    return 0;
}
*/

vre_uint32 vre_font_map_encoding( vre_font *pFont,
                                  vre_int32 encoding,
                                  vre_uint32 character )
{
    vre_assert ( pFont != 0);
    vre_assert ( pFont->pULFont != 0 );
    vre_assert ( pFont->interface.map_encoding != 0 );
    
    if ( encoding < 0 )
    {
        encoding = pFont->default_encoding;
    }
    
    return pFont->interface.map_encoding( pFont->pULFont, encoding, character );
}

vre_uint32 vre_font_get_default_encoding ( vre_font *pFont ) 
{
    return pFont->default_encoding;
}

void vre_font_select_encoding ( vre_font *pFont, vre_uint32 encoding )
{
    pFont->default_encoding = encoding;
}


vre_result vre_font_get_glyph( vre_font* pFont, 
                               vre_meta_glyph** ppGlyph, 
                               vre_uint32 id)
{
    vre_assert ( pFont != 0);
    vre_assert ( pFont->pULFont != 0 );
    vre_assert ( pFont->interface.get_glyph != 0 );
    
    return pFont->interface.get_glyph (pFont->pULFont, ppGlyph, id);
}

void vre_font_apply_kerning( vre_font *pFont, 
                             vre_uint32 left, vre_uint32 right,
                             vre_int32 *dx, vre_int32 *dy )
{
    vre_assert ( pFont != 0);
    vre_assert ( pFont->pULFont != 0 );
    vre_assert ( pFont->interface.apply_kerning != 0 );
    
    pFont->interface.apply_kerning( pFont->pULFont, left, right, dx, dy );
}

vre_uint32 vre_font_get_advance_width( vre_font *pFont,
                                       vre_uint32 glyph_id )
{
    vre_uint32 advance_width;
    vre_assert ( pFont != 0);
    vre_assert ( pFont->pULFont != 0 );
    vre_assert ( pFont->interface.get_advance_width != 0 );

    advance_width = pFont->interface.get_advance_width( pFont->pULFont, glyph_id );

    return advance_width;
}

vre_uint32 vre_font_get_advance_height( vre_font *pFont,
                                        vre_uint32 glyph_id )
{
    vre_assert ( pFont != 0);
    vre_assert ( pFont->pULFont != 0 );
    vre_assert ( pFont->interface.get_advance_height != 0 );
    
    return pFont->interface.get_advance_height( pFont->pULFont, glyph_id );
}


