/*
*  vre_text.c
*  
*
*  Created by Manuel  Astudillo on 2006-03-30.
*  Copyright 2006 Curved Graphics. All rights reserved.
*
*/

#include "vre_text.h"
#include "vre_glyph.h"
#include "vre_rectangle.h"
#include "vre_array.h"
#include "vre_mem.h"
#include "vre_clip.h"
#include "vre_render.h"
#include "vre_rtree.h"
#include "vre_context.h"
#include "vre_assert.h"

typedef struct vre_cursor
{
	vre_int32 x;
	vre_int32 y;

} vre_cursor;

// it means that starting from char number "char_number" the text will be printed on line "line_number"
typedef struct vre_char_line_info
{
	vre_uint32 char_number;
	vre_int32 line_number;

} vre_char_line_info;



struct vre_text
{
	vre_int32 line_number;  // current line number of vre_text instance. 
	vre_uint32 num_chars;   // number of characters in the text  
	vre_list* pLines;       // information for knowing when to change cursor's y coordinate to new line.

	vre_cursor cursor;
	vre_font   *pFont;
	vre_mat3x3  mat;

	vre_char_type char_type;
	vre_uint8 char_size;

	vre_array *pChars;
	vre_uint32 encoding;

	vre_rectangle bbox;

	vre_rtree *pRTree;

	vre_style *pStyle;

	vre_primitive_funcs funcs;
};

typedef struct text_user_data
{
	vre_font   *pFont;
	vre_uint32 glyph_id;
} text_user_data;

static
vre_result vre_text_renderer ( void *pUserData, 
							  vre_mat3x3 *pMat,
							  vre_tile   *pTile,
							  vre_style  *pStyle);

static
void vre_text_destroyer ( void *pUserData );


vre_result vre_text_create ( vre_text **ppText,
							vre_font *pFont,
							vre_style *pStyle,
							vre_mat3x3 *pMat,
							vre_char_type type)
{
	vre_result vres = VRE_ERR_OK; 
	vre_text *pText;

	*ppText = 0;

	if ( pFont == 0 )
	{
		return VRE_ERR_UNSUPORTED;
	}

	pText = VRE_TALLOC ( vre_text );

	if ( pText == 0 )
	{
		return VRE_ERR_MEM_ALLOC_FAILED;
	}

	vre_memset(pText, sizeof ( vre_text ), 0);

	pText->pFont = pFont;

	pText->char_type = type;

	pText->funcs.renderer = vre_text_renderer;
	pText->funcs.destroyer = vre_text_destroyer;

	for (;;)
	{
		vre_uint32 char_size;
		vre_mat3x3 ident_mat;

		// Style cannot be referenced like this. it must be copied or something.
		pText->pStyle = pStyle; // TODO: Create a style module and fix this.

		if ( pMat != 0 )
		{
			vre_matrix_identity( &ident_mat );
			vre_matrix_mult(&ident_mat, pMat, &pText->mat);
		}
		else
		{
			vre_matrix_identity( &pText->mat );
		}

		switch ( pText->char_type )
		{
		case VRE_CHAR_ASCII: char_size = 1; break;
		case VRE_CHAR_UTF16: char_size = 2; break;
		case VRE_CHAR_UTF32: char_size = 4; break;
		default:
			char_size = 0; 
			vre_assert ( 0 );
		}

		vres = vre_array_create ( char_size * 32, &pText->pChars );
		VRE_BREAK_IF_ERR ( vres );

		pText->char_size = (vre_uint8)char_size;

		vres = vre_list_create(&pText->pLines);
		VRE_BREAK_IF_ERR ( vres );

		break;
	}

	if ( vres != VRE_ERR_OK )
	{
		vre_text_destroy ( pText );   
		return vres;
	}

	*ppText = pText;

	return vres;
}


void vre_text_destroy ( vre_text *pText )
{
	if ( pText != 0 )
	{
		vre_array_destroy ( pText->pChars );

		// TODO: would be more efficient and clearer to write vre_list_foreach function and pass it a callback 
		// function that frees the memory.
		if (pText->pLines)
		{
			int i = 0;
			vre_char_line_info* pCharLineInfo;

			while (pCharLineInfo = vre_list_peek(pText->pLines, i))
			{
				vre_free(pCharLineInfo);
				++i;
			}

			vre_list_destroy ( pText->pLines );
		}


		vre_free ( pText );
	}
}

void vre_text_set_style ( vre_text *pText, vre_style *pStyle )
{
	pText;
	pStyle;

}


vre_result vre_text_add_string ( vre_text *pText,
								void *pString,
								vre_uint32 length )
{
	vre_result vres = VRE_ERR_OK;

	vre_assert ( pText != 0 );
	vre_assert ( pText->pChars != 0 );
	vre_assert ( pString );


	vres = vre_array_add( pText->pChars, pString, length * pText->char_size );

	pText->num_chars += length;

	return vres;
}

vre_result vre_text_newline ( vre_text *pText )
{
	vre_result vres = VRE_ERR_OK;
	vre_char_line_info* pCharLineInfo;

	vre_assert(pText != 0);
	vre_assert(pText->pLines != 0);

	++pText->line_number;


	pCharLineInfo = vre_list_peek_last(pText->pLines);

	if (pCharLineInfo && (pCharLineInfo->char_number == pText->num_chars))
	{
		// handles the situation of more than one vre_text_newline in a row. 
		pCharLineInfo->line_number = pText->line_number;
	}
	else
	{
		pCharLineInfo = VRE_TALLOC( vre_char_line_info );
		if ( pCharLineInfo == 0 )
		{
			return VRE_ERR_MEM_ALLOC_FAILED;
		}

		pCharLineInfo->char_number = pText->num_chars;
		pCharLineInfo->line_number = pText->line_number;

		vres = vre_list_put_last(pText->pLines, pCharLineInfo);
	}

	return vres;
}


static vre_result vre_glyph_draw( vre_glyph *pGlyph, 
								  vre_style *pStyle, 
								  vre_tile *pTile, 
								  vre_mat3x3 *pMat )
{
	vre_result vres = VRE_ERR_OK;
	vre_rectangle rect;
	vre_polygon *pPoly;
	vre_polygon *pPolyClipped;
	vre_polygon *pPolyTmp;
    vre_render  *pRenderer;

    // Create renderer and temp polygons ( TO BE MOVED to proper place )
    vres = vre_render_create ( &pRenderer, pTile->w );
    
    
	// Temp
	vres = vre_polygon_create2( &pPolyClipped,32);
	vres = vre_polygon_create2( &pPolyTmp, 32) ;
	//
	// Compute Bounding box
	//

	vres = vre_glyph_to_polygon( pGlyph, pStyle, pMat, &pPoly );
	VRE_RETURN_IF_ERR ( vres );

	//
	// Clip glyph polygon.
	//
	rect.x = 0;//pTile->x << 16;
	rect.y = 0;//pTile->y << 16;
	rect.h = (pTile->h << 16) + 65535;
	rect.w = (pTile->w << 16) + 65000;
	vre_clip_poly_rectangle( pPoly, &rect, pPolyClipped, pPolyTmp );

	//
	// Draw polygon to tile.
	//
	if ( vre_polygon_get_num_vertices (pPolyClipped)  >= 3 )
	{
		vre_draw_polygon_concave_aa( pRenderer, pPolyClipped, pTile, pStyle );
		//vre_draw_polygon_concave( pPolyClipped, pTile, pStyle );
	}

	vre_polygon_destroy( pPoly );
	vre_polygon_destroy( pPolyTmp );
	vre_polygon_destroy( pPolyClipped );
    vre_render_destroy ( pRenderer );

	return vres;
}

void vre_text_set_cursor ( vre_text *pText, vre_int32 x, vre_int32 y)
{
	pText->cursor.x = x;
	pText->cursor.y = y;
}

/**
Create an internal structure to hold the text and provide
faster iterative rendering.
*/
vre_result vre_text_prepare ( vre_text *pText,
							 vre_mat3x3 *pMat,
							 vre_context *pContext )
{
	// tbd - refactor, make sure it works with new_line\
	// reconsider new_line implementation as collection of pText objects, whereas new pText object
	// will have pText->cursor.y to be the next line

	vre_result vres = VRE_ERR_OK;
	vre_bool first_glyph = VRE_TRUE;
	vre_uint32 right_gid, left_gid = 0; // gid - glyph id
	vre_uint32 i, j;
	vre_font *pFont;
	void *pChars;
	vre_font_attr *pAttr;

	vre_meta_glyph  *pMetaGlyph;
	vre_glyph       *pGlyph;

	text_user_data  *pUserData;

	vre_rectangle   bbox;

	vre_uint32 length;
	vre_uint32 num_chars;

	vre_uint32 char_number = 0;
	vre_uint32 k = 0;
	vre_char_line_info* pCharLineInfo = 0;
	vre_bool has_more_line_info_items = VRE_TRUE;

	vre_int32 baseline_to_baseline_dist;

	vre_assert ( pText != 0 );
	vre_assert ( pText->pFont != 0 );
	vre_assert ( pText->pChars != 0 );
	vre_assert ( pText->pLines != 0 );

	pFont = pText->pFont;

	pAttr = vre_font_get_attributes(pFont);
	vre_assert ( pAttr != 0 );    

	//    first_glyph = 0;

	vre_array_get_elems_start ( pText->pChars, &pChars, &length );

	vre_assert ( (length % pText->char_size) == 0 );
	num_chars = length / pText->char_size;

	// http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
	// baseline-to-baseline distance = ascent - descent + linegap 
	baseline_to_baseline_dist = pAttr->ascender - pAttr->descender +  pAttr->line_gap;

	while ( pChars != 0 )
	{
		for ( i = num_chars; i > 0; --i )
		{
			vre_uint32 glyph_id;

			if (has_more_line_info_items)
			{
				pCharLineInfo = (vre_char_line_info*)vre_list_peek(pText->pLines, k);
			}

			if (pCharLineInfo)
			{
				if (pCharLineInfo->char_number == char_number)
				{
					first_glyph = VRE_TRUE;

					pText->cursor.x = 0; // carriage return
					pText->cursor.y = -pCharLineInfo->line_number * baseline_to_baseline_dist;
					++k;
				}
			}
			else
			{
				has_more_line_info_items = VRE_FALSE;
			}

			//
			// Get glyph id
			//
			switch ( pText->char_type )
			{
			case VRE_CHAR_ASCII: 
				{
					vre_uint8 c = *((vre_uint8*)pChars);
					glyph_id = vre_font_map_encoding( pFont, -1, c);
					pChars = ((vre_uint8*)pChars) + 1; 
					break;
				}
			case VRE_CHAR_UTF16:
				{
					vre_uint16 c = *((vre_uint16*)pChars);
					glyph_id = vre_font_map_encoding( pFont, -1, c);
					pChars = ((vre_uint16*)pChars) + 1;
					break;
				}
			case VRE_CHAR_UTF32:
				{
					vre_uint32 c = *((vre_uint32*)pChars);
					glyph_id = vre_font_map_encoding( pFont, -1, c);
					pChars = ((vre_uint32*)pChars) + 1;
					break;
				}
			default: 
				glyph_id = 0;
				vre_assert( 0 );
			}

			vre_font_get_glyph( pFont, &pMetaGlyph, glyph_id);


			//
			// Apply Kerning.
			//
			if (first_glyph)
			{
				left_gid  = glyph_id;
				right_gid = glyph_id;
				first_glyph = VRE_FALSE;
			}
			else
			{   
				vre_int32 dx, dy;

				switch(pAttr->direction)
				{
				case VRE_FONT_DIRECTION_UD: // UD haven't been tested, though should work exactly as LR.

				case VRE_FONT_DIRECTION_LR:
					left_gid = right_gid;
					right_gid = glyph_id;
					break;

				case VRE_FONT_DIRECTION_RL:
					right_gid = left_gid;	
					left_gid = glyph_id;
					break;

				default: 
					vre_assert(!"should never get here");
					break;
				}

				vre_font_apply_kerning ( pFont, left_gid, right_gid, &dx, &dy);

				pText->cursor.x += dx;
				pText->cursor.y += dy;
			}

			if ( pMetaGlyph != 0 )
			{
				//            vre_uint32 adv_width;
				//            vre_uint32 adv_height;

				vre_mat3x3 mat;

				//
				// Simple glyph
				//
				if ( pMetaGlyph->glyph_type == 0 )
				{
					vre_mat3x3 glyph_mat;
					pUserData = VRE_TALLOC( text_user_data );
					if ( pUserData == 0 )
					{
						return VRE_ERR_MEM_ALLOC_FAILED;
					}
					pUserData->pFont = pFont;

					pGlyph = (vre_glyph*) pMetaGlyph->pGlyph;

					vre_matrix_identity( &mat );

					if (pAttr->direction == VRE_FONT_DIRECTION_RL)
					{
						pText->cursor.x -= vre_font_get_advance_width( pFont, glyph_id);
					}

					vre_matrix_translate( &mat, 
						pText->cursor.x, 
						pText->cursor.y);

					vre_matrix_mult ( &mat, pMat, &glyph_mat );

					pUserData->glyph_id = glyph_id;

					vre_glyph_get_bounding_box ( pGlyph, &bbox, &glyph_mat);

					vres = vre_context_add_primitive ( pContext, 
						(void*) pUserData, 
						&pText->funcs, 
						&glyph_mat, 
						&bbox );
					VRE_RETURN_IF_ERR (vres);
				}
				else
				{
					vre_list *pComponents = (vre_list*) pMetaGlyph->pGlyph;
					vre_uint32 num_components = vre_list_num_elems ( pComponents );

					// Draw composite glyphs
					for ( j = 0; j <  num_components; j++ )
					{
						vre_glyph_component *pComponent;

						vre_mat3x3 glyph_mat;
						pUserData = VRE_TALLOC( text_user_data );
						if ( pUserData == 0 )
						{
							return VRE_ERR_MEM_ALLOC_FAILED;
						}
						pUserData->pFont = pFont;

						pComponent = vre_list_peek ( pComponents, j );

						vre_font_get_glyph( pFont, &pMetaGlyph, 
							pComponent->glyph_id);

						vre_assert ( pMetaGlyph->glyph_type == 0 );

						pGlyph = (vre_glyph*) pMetaGlyph->pGlyph;

						vre_matrix_identity ( &mat );
						vre_matrix_translate ( &mat, 
							pText->cursor.x, 
							pText->cursor.y);

						vre_matrix_mult ( &mat, &pComponent->mat, &mat );

						vre_matrix_mult ( &mat, pMat, &glyph_mat );

						pUserData->glyph_id = pComponent->glyph_id;

						vre_glyph_get_bounding_box ( pGlyph, &bbox, &glyph_mat);

						vres = vre_context_add_primitive ( pContext, 
							(void*) pUserData, 
							&pText->funcs, 
							&glyph_mat, 
							&bbox );
						VRE_RETURN_IF_ERR (vres);
					}

					// TODO: CHECK THE FLAG: USE_MY_METRICS
				}

			}


			if ( pAttr->direction != VRE_FONT_DIRECTION_UD )
			{
				if (pAttr->direction == VRE_FONT_DIRECTION_LR)
				{
					pText->cursor.x += vre_font_get_advance_width( pFont, glyph_id);
				}
			}
			else
			{
				pText->cursor.y += vre_font_get_advance_height( pFont, glyph_id); 
			}

			++char_number;
		}

		vre_array_get_elems_iter ( pText->pChars, &pChars, &length );
	} 

	//  vre_rtree_print (pText->pRTree);

	return vres;
}


/**
Render RTree for maximum performance.

*/
/*
vre_result vre_text_draw ( vre_text *pText, 
vre_tile *pTile,
vre_mat3x3 *pMat)
{
vre_result vres = VRE_ERR_OK;
vre_meta_glyph  *pMetaGlyph;
vre_rtree       *pRTree;
vre_list        *pList;
vre_font        *pFont;
vre_mat3x3      *pMatOut;
vre_mat3x3      mat;

vre_rectangle   rect;

vre_assert ( pText );
vre_assert ( pText->pRTree );
vre_assert ( pText->pFont );

vres = vre_list_create ( &pList );
VRE_RETURN_IF_ERR ( vres );

pRTree = pText->pRTree;
pFont = pText->pFont;

rect.x1 = (pTile->x) << 16;
rect.y1 = (pTile->y) << 16;
rect.x2 = (pTile->x + pTile->w) << 16;
rect.y2 = (pTile->y + pTile->h) << 16;

vres = vre_rtree_search ( pRTree, &rect, pList );
if ( vres != VRE_ERR_OK )
{
vre_list_destroy ( pList );
}
VRE_RETURN_IF_ERR (vres);

pGlyphNode = vre_list_get_first ( pList );

while ( pGlyphNode != 0 )
{
vre_font_get_glyph( pFont, &pMetaGlyph, 
pGlyphNode->glyph_id);

vre_assert ( pMetaGlyph );

if ( pMat != 0 )
{
vre_matrix_mult ( &pGlyphNode->mat, pMat, &mat );
pMatOut = &mat;
}
else
{
pMatOut = &pGlyphNode->mat;
}

vres = vre_glyph_draw( pMetaGlyph->pGlyph, 
pText->pStyle, 
pTile, 
pMatOut );

VRE_BREAK_IF_ERR (vres);

pGlyphNode = vre_list_get_first ( pList );
};

vre_list_destroy ( pList );

return vres;
}
*/

static
vre_result vre_text_renderer ( void *pUserData, 
							  vre_mat3x3 *pMat,
							  vre_tile   *pTile,
							  vre_style  *pStyle)
{
	vre_result vres;
	vre_meta_glyph  *pMetaGlyph;
	text_user_data  *pTextUserData;

	vre_assert ( pUserData );

	pTextUserData = ( text_user_data  * ) pUserData;

	vre_font_get_glyph( pTextUserData->pFont, 
		&pMetaGlyph, 
		pTextUserData->glyph_id);

	vre_assert ( pMetaGlyph );

	vres = vre_glyph_draw( pMetaGlyph->pGlyph, pStyle, pTile, pMat );

	return vres;
}

static
void vre_text_destroyer ( void *pUserData )
{
	vre_free ( pUserData );
}



