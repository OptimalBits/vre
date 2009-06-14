
#include "vre_ttf.h"
#include "vre_ttf_metrics.h"
#include "vre_ttf_max_profile.h"
#include "vre_list.h"
#include "vre_glyph.h"
#include "vre_mem.h"
#include "vre_assert.h"

#define VRE_TTF_CACHE_SIZE 32

struct ttf_offset_table
{
    vre_fix16   version;
    vre_uint16  num_tables;
    vre_uint16  search_range;   // (Maximum power of 2 <= numTables) x 16
    vre_uint16  entry_selector; // Log2(maximum power of 2 <= numTables)
    vre_uint16  range_shift;    // NumTables x 16-searchRange
};

struct ttf_table_directory
{
    vre_uint32  tag;      // 4 - byte identifier    
    vre_uint32  checksum; // checksum for this table
    vre_uint32  offset;   // offset from beg. of TTF file.
    vre_uint32  length;
};


// Table Tags

// Required
#define TTF_TAG_CMAP 0x636D6170 // 'cmap'
#define TTF_TAG_HEAD 0x68656164 // 'head'
#define TTF_TAG_HHEA 0x68686561 // 'hhea'
#define TTF_TAG_HMTX 0x686D7478 // 'hmtx'
#define TTF_TAG_MAXP 0x6D617870 // 'maxp'
#define TTF_TAG_NAME 0x6E616D65 // 'name'
#define TTF_TAG_OS2  0x4F532F32 // 'OS/2'
#define TTF_TAG_POST 0x706F7374 // 'post'

// TrueType related
#define TTF_TAG_CVT  0x63767400 // 'cvt'
#define TTF_TAG_FPGM 0x6670676D // 'fpgm' 
#define TTF_TAG_GLYF 0x676C7966 // 'glyph
#define TTF_TAG_LOCA 0x6C6F6361 // 'loca'
#define TTF_TAG_PREP 0x70726570 // 'prep'
#define TTF_TAG_KERN 0x6B65726E // 'kern'


/*
ULONG
CalcTableChecksum(ULONG *Table, ULONG Length)
{
ULONG Sum = 0L;
ULONG *Endptr = Table+((Length+3) & ~3) / sizeof(ULONG);

while (Table < EndPtr)
	Sum += *Table++;
return Sum;
}
*/



struct ttf_header_table
{
    vre_fix16   table_version;
    vre_fix16   font_version;
    vre_uint32  checksum_adjustment;
    vre_uint32  magic;
    vre_uint16  flags;
    vre_uint16  units_per_em;
    vre_uint32  date_created_low;
    vre_uint32  date_created_high;
    vre_uint32  date_modified_low;
    vre_uint32  date_modified_high;
    vre_int16   x_min;
    vre_int16   y_min;
    vre_int16   x_max;
    vre_int16   y_max;
    vre_uint16  mac_style;
    vre_uint16  lowest_rec_PPEM;
    vre_int16   font_direction_hint;
    vre_int16   index_to_loc_format;
    vre_int16   glyph_data_format;
};

typedef struct ttf_location_table
{
    void    *pOffsets;
            
} ttf_location_table;

typedef struct ttf_encoding_table_0
{   
    vre_uint8  glyph_id_array[256];
} ttf_encoding_table_0;


typedef struct ttf_encoding_table_4
{
    vre_uint16 	seg_count_X2; 	
    vre_uint16 	search_range; 	
    vre_uint16 	entry_selector; 	
    vre_uint16 	range_shift; 	
    vre_uint16 	*pEndCount;
    vre_uint16 	reserved_pad; 	
    vre_uint16 	*pStartCount;
    vre_int16 	*pIdDelta;
    vre_uint16 	*pIdRangeOffset;
    vre_uint16 	*pGlyphIdArray;
} ttf_encoding_table_4;


typedef struct ttf_encoding_group
{
    vre_uint32 	start_char_code;
    vre_uint32 	end_char_code;
    vre_uint32 	start_glyph_id;
} ttf_encoding_group;

typedef struct ttf_encoding_table_12
{
    vre_uint32 nGroups;
    ttf_encoding_group *pGroups;
} ttf_encoding_table_12;

typedef struct ttf_encoding
{
    vre_uint16 format;
    vre_uint16 platform_id;
    vre_uint16 encoding_id;
    vre_uint16 length;
    vre_uint16 language;
   
    void       *pTable;
} ttf_encoding;


typedef struct ttf_encoding_record
{
    vre_uint16 platform_id;
    vre_uint16 encoding_id;
    vre_uint32 offset;
} ttf_encoding_record;


struct ttf_cmap_table
{
    vre_uint16     num_encodings;
    ttf_encoding   **ppEncodings;
};


typedef struct ttf_glyph_header 
{
    vre_int16   num_contours; 
    // If the number of contours is greater than or equal to zero, 
    // this is a simple glyph; if negative, this is a composite glyph.
    vre_int16   x_min;
    vre_int16   y_min;
    vre_int16   x_max;
    vre_int16   y_max;
} ttf_glyph_header;


struct ttf_glyph_table
{
    ttf_glyph_header header;    
    
    void        *pGlyphData;
};

// Simple Glyph flags
#define GLYPH_FLG_ON_CURVE  1
#define GLYPH_FLG_X_SHORT   2
#define GLYPH_FLG_Y_SHORT   4
#define GLYPH_FLG_REPEAT    8
#define GLYPH_FLG_X_SAME   16
#define GLYPH_FLG_Y_SAME   32

typedef struct ttf_glyph_simple
{
    vre_uint16  pEndPoints;
    vre_uint16  instruction_length;
    vre_uint8   *pInstructions;
    vre_uint8   *pFlags;
    void        *pXCoordinates;
    void        *pYCoordinates;   
} ttf_glyph_simple;

// Composite Glyph flags
#define ARG_1_AND_2_ARE_WORDS       1
#define ARGS_ARE_XY_VALUES          2
#define ROUND_XY_TO_GRID            4
#define WE_HAVE_A_SCALE             8
#define RESERVED                    16
#define MORE_COMPONENTS             32
#define WE_HAVE_AN_X_AND_Y_SCALE    64
#define WE_HAVE_A_TWO_BY_TWO       128
#define WE_HAVE_INSTRUCTIONS       256
#define USE_MY_METRICS             512
#define OVERLAP_COMPOUND          1024
#define SCALED_COMPONENT_OFFSET   2048
#define UNSCALED_COMPONENT_OFFSET 4096


void vre_ttf_destroy_cmap_table ( ttf_cmap_table *pCmap );


// Kerning
typedef struct vre_ttf_kerning_subtable
{
    vre_uint16 length;
    vre_uint16 coverage;
    
    void *pFormat;
    vre_uint8 format;
} vre_ttf_kerning_subtable;

struct vre_ttf_kerning_table
{
    vre_uint16 num_tables;
    vre_ttf_kerning_subtable *pSubtables;
};

// Coverage Bits
#define KERNING_HORIZONTAL      1 
#define KERNING_MINIMUM         2 
#define KERNING_CROSS_STREAM    4
#define KERNING_OVERRIDE        8
#define KERNING_FORMAT( format ) ( ((format) & 0xff00) >> 8 )

typedef struct vre_ttf_kerning_pair
{
    vre_uint32 left_right;
    vre_int16  value;
} vre_ttf_kerning_pair;


typedef struct vre_ttf_kerning_format0
{
    vre_uint16 num_pairs;
    vre_uint16 search_range;
    vre_uint16 entry_selector;
    vre_uint16 range_shift;
    
    vre_ttf_kerning_pair *pPairs;
    
} vre_ttf_kerning_format0;

typedef struct vre_ttf_kerning_format2
{
	vre_int dummy;
} vre_ttf_kerning_format2;

static vre_result vre_ttf_read_kerning_format0 ( vre_iostream *pStream, 
                                                 vre_ttf_kerning_format0 **ppFormat );


void vre_ttf_destroy_kerning_table( vre_ttf_kerning_table *pTable );

vre_result vre_read_kerning_table ( vre_iostream *pStream,
                                    vre_ttf_kerning_table **ppTable )
{
    vre_result vres = VRE_ERR_OK;
    vre_ttf_kerning_table *pTable;
    
    *ppTable = 0;
    
    pTable = 
        (vre_ttf_kerning_table*) vre_malloc ( sizeof ( vre_ttf_kerning_table ));
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    for (;;)
    {
        vre_uint32 i;
        
        // Skip version
        vres = vre_iostream_skip (pStream, 2);
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word(pStream, &pTable->num_tables);
        VRE_BREAK_IF_ERR ( vres );
        
        pTable->pSubtables = 
            (vre_ttf_kerning_subtable*)vre_malloc (sizeof(vre_ttf_kerning_subtable)* 
                                               pTable->num_tables );
        if ( pTable->pSubtables == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < pTable->num_tables; i++ )
        {
            vre_uint32 format;
            
            // skip version
            vres = vre_iostream_skip (pStream, 2);
            VRE_BREAK_IF_ERR ( vres );

            vres = vre_iostream_read_word(pStream, 
                                          &pTable->pSubtables[i].length);
            VRE_BREAK_IF_ERR ( vres );
            
            vres = vre_iostream_read_word(pStream, 
                                          &pTable->pSubtables[i].coverage);
            VRE_BREAK_IF_ERR ( vres );
            
            format = KERNING_FORMAT ( pTable->pSubtables[i].coverage );
            pTable->pSubtables[i].format = (vre_uint8) format;
            switch (format)
            {
                case 0: 
                    vres = 
                    vre_ttf_read_kerning_format0( pStream, 
                                                  (vre_ttf_kerning_format0 **)           
                                                  &pTable->pSubtables[i].pFormat );
                    break;
                default:
                    vres = VRE_ERR_UNSUPORTED_FORMAT;
            }
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_ttf_destroy_kerning_table( pTable );
    }
    
    *ppTable = pTable;
    
    return vres;
}

static void vre_ttf_destroy_kerning_format0(vre_ttf_kerning_format0 *pFormat )
{
    if ( pFormat != 0 )
    {
        vre_free ( pFormat->pPairs );
        vre_free ( pFormat );
    }
}

void vre_ttf_destroy_kerning_table( vre_ttf_kerning_table *pTable )
{
    vre_uint32  i;
    
    if ( pTable != 0 )
    {
        for ( i = 0; i < pTable->num_tables; i++ )
        {
            vre_ttf_kerning_subtable *pSubtable = &pTable->pSubtables[i];
            switch ( pSubtable->format )
            {
                case 0: 
                    vre_ttf_destroy_kerning_format0( pSubtable->pFormat );
                    break;
            }
            
            vre_free ( pTable->pSubtables );
        }
        
        vre_free ( pTable );
    }
}


static vre_result vre_ttf_read_kerning_format0 ( vre_iostream *pStream, 
                                                 vre_ttf_kerning_format0 **ppFormat )
{
    vre_result vres = VRE_ERR_OK;
    vre_ttf_kerning_format0 *pFormat;
    
    *ppFormat = 0;
    
    pFormat = VRE_TALLOC ( vre_ttf_kerning_format0 );
    if ( pFormat == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
        
    for (;;)
    {
        vre_uint32 i;
        
        vres = vre_iostream_read_word(pStream, &pFormat->num_pairs);
        VRE_BREAK_IF_ERR ( vres );
            
        vres = vre_iostream_read_word(pStream, &pFormat->search_range);
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word(pStream, &pFormat->entry_selector);
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word(pStream, &pFormat->range_shift);
        VRE_BREAK_IF_ERR ( vres );
        
        pFormat->pPairs = 
            (vre_ttf_kerning_pair*) vre_malloc ( sizeof (vre_ttf_kerning_pair)*
                                                         pFormat->num_pairs );
        if ( pFormat->pPairs == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < pFormat->num_pairs; i++ )
        {
            vre_uint16 left, right;
            
            vres = vre_iostream_read_word(pStream, &left);
            VRE_BREAK_IF_ERR ( vres );
            
            vres = vre_iostream_read_word(pStream, &right);
            VRE_BREAK_IF_ERR ( vres );
            
            pFormat->pPairs[i].left_right = ( left << 16 ) | right;

            vres = vre_iostream_read_sword(pStream, &pFormat->pPairs[i].value);
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
    
    *ppFormat = pFormat;
    
    return vres;
}


                                          
void vre_ttf_apply_kerning ( vre_ttf *pTtf, 
                             vre_uint16 left_glyph,
                             vre_uint16 right_glyph,
                             vre_int32  *dx,
                             vre_int32  *dy )
{
    vre_uint32 j;
    vre_uint32 i;
    
    vre_uint32 index = (left_glyph << 16) | right_glyph;
    
    vre_ttf_kerning_table *pKerning = pTtf->pKerning;
 
    *dx = 0;
    *dy = 0;
    
    if ( pKerning == 0 )
    {
        return;
    }
    
    for ( j = 0; j < pKerning->num_tables; j++ )
    {
        vre_ttf_kerning_subtable *pSubtable = &pKerning->pSubtables[j];
        
        if ( pSubtable == 0 )
        {
            break;
        }
        
        switch ( pSubtable->format )
        {
            case 0:
            {
                vre_ttf_kerning_format0 *pFormat;
                vre_uint32 num_pairs;
                
                pFormat = (vre_ttf_kerning_format0*)  pSubtable->pFormat;
                
                
                if ( ( pFormat == 0 ) || ( pFormat->pPairs == 0 ) )
                {
                    break;
                }

                num_pairs = pFormat->num_pairs;
                
                i = 0;
                while ( ( i < num_pairs) && 
                        ( pFormat->pPairs[i].left_right <= index) )
                {
                    if ( index == pFormat->pPairs[i].left_right )
                    {
                        if ( pSubtable->coverage & KERNING_OVERRIDE )
                        {
                            if ( pSubtable->coverage & KERNING_HORIZONTAL )
                            {
                                *dx = pFormat->pPairs[i].value;
                            }
                            else
                            {
                                *dy = pFormat->pPairs[i].value;
                            }
                        }
                        else
                        {
                            if ( pSubtable->coverage & KERNING_HORIZONTAL )
                            {
                                *dx += pFormat->pPairs[i].value;
                            }
                            else
                            {
                                *dy += pFormat->pPairs[i].value;
                            }
                        }
                        
                    }
                    
                    i++;
                }
                
                break;
            }
        }
    }
    
}
                                  


static 
vre_result vre_read_offset_table ( vre_iostream *pStream, 
                                   ttf_offset_table **pTable);
                                   
static 
vre_result vre_seek_table ( vre_iostream *pStream, 
                            vre_ttf *pTtf, 
                            vre_uint32 tag );
                                   
static 
vre_result vre_read_table_directories ( vre_iostream *pStream, 
                                        ttf_table_directory **ppTableDir,
                                        vre_uint16 num_tables );                                               

static 
vre_result vre_read_header_table ( vre_iostream *pStream, 
                                   ttf_header_table **ppHeader );

static 
vre_result vre_read_cmap_table ( vre_iostream *pStream,
                                 ttf_cmap_table **ppCmap );                                   
                                                                      
static vre_result vre_read_locate_table ( vre_iostream *pStream,                                           
                                          void **ppLoc,
                                          vre_bool format_short,
                                          vre_uint16 num_glyphs);                                   
                                          
static 
void vre_read_glyph_table ( vre_iostream *pStream, 
                            vre_ttf *pTtf );

static vre_result vre_read_glyph_from_id ( vre_iostream *pStream, 
                                           vre_ttf *pTtf, 
                                           vre_uint32 glyph_id,
                                           vre_meta_glyph **ppGlyph);

static vre_result vre_read_glyph ( vre_iostream *pStream,
                                   vre_meta_glyph **ppGlyph );
                                  
static vre_result vre_read_glyph_header ( vre_iostream *pStream, 
                                          ttf_glyph_header *pHeader );
                                          
static vre_result vre_read_glyph_simple ( vre_iostream *pStream,
                                          vre_glyph *pGlyph,
                                          vre_int16 num_contours );
                                          
static vre_result vre_read_glyph_composite ( vre_iostream *pStream,
                                             vre_list **ppComponentList );

vre_result vre_create_ttf ( vre_ttf **ppTtf )
{
    vre_result vres;
    vre_ttf *pTtf;
    
    *ppTtf = 0;
    
    pTtf = VRE_TALLOC ( vre_ttf );
    
    if ( pTtf == 0 )
    { 
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset( pTtf, sizeof ( vre_ttf) ,0 );
    
    //
    // Create cache
    //
    vres = vre_cache_create ( &pTtf->pCache, VRE_TTF_CACHE_SIZE );
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTtf );
        return vres;
    }
    
    *ppTtf = pTtf;
    
    return VRE_ERR_OK;
}

// TODO: free remaining structures.
void vre_destroy_ttf ( vre_ttf *pTtf )
{
    //
    vre_free( pTtf->pOffsetTab );
    vre_free( pTtf->pTableDir );
    
    vre_free( pTtf->pMaxProfile );
    
    vre_free( pTtf->pHhea );
    
    vre_free( pTtf->pHtmx->pHMetrics );
    vre_free( pTtf->pHtmx->pLsbs );
    vre_free( pTtf->pHtmx );
    
    vre_free( pTtf->pHeader );
    
    vre_ttf_destroy_cmap_table( pTtf->pCmap );
    
    vre_free( pTtf->pLoc );
    
    vre_ttf_destroy_kerning_table( pTtf->pKerning );
    
    vre_cache_destroy( pTtf->pCache );
    
    vre_free ( pTtf );
}

void vre_ttf_bind ( vre_ttf *pTtf, vre_font *pFont )
{
    vre_font_interface *pInterface;
    vre_font_attr *pAttributes;
    
    vre_assert ( pTtf != 0 );
    vre_assert ( pTtf->pHeader != 0);
    vre_assert ( pFont != 0 );
    
    vre_font_set_ul_font (pFont, pTtf);
    pInterface = vre_font_get_interface (pFont);
    
    vre_assert ( pInterface != 0 );
    
    pAttributes = vre_font_get_attributes(pFont);
    
    //
    // Bind interface functions
    //
    pInterface->map_encoding = (VRE_MAP_ENCODING) vre_ttf_map_encoding;
    pInterface->get_glyph = (VRE_GET_GLYPH) vre_ttf_get_glyph;
    pInterface->apply_kerning = (VRE_APPLY_KERNING) vre_ttf_apply_kerning;
    pInterface->get_advance_width = (VRE_GET_ADVANCE_WIDTH) vre_ttf_get_advance_width;
    
    //
    // Fill attributes data
    //
    pAttributes->upem = pTtf->pHeader->units_per_em;
	pAttributes->line_gap = pTtf->pHtmx->pHhea->line_gap;
	pAttributes->ascender = pTtf->pHtmx->pHhea->ascender;
	pAttributes->descender = pTtf->pHtmx->pHhea->descender;

}


void vre_ttf_get_glyph ( vre_ttf *pTtf, vre_meta_glyph **ppGlyph, 
                         vre_uint32 index )
{
    vre_meta_glyph *pGlyph;
    
    if ( index < pTtf->pMaxProfile->num_glyphs )
    {
        vre_cache_query( pTtf->pCache, index, (void **) ppGlyph );
        if ( *ppGlyph == 0 )
        {
            vre_read_glyph_from_id( pTtf->pStream, pTtf, index, &pGlyph );
            if ( pGlyph != 0 )
            {
                vre_meta_glyph *pGlyphOut;
                vre_cache_insert( pTtf->pCache, index, (void*) pGlyph, 
                                  (void**) &pGlyphOut );
                vre_meta_glyph_destroy( pGlyphOut );
            }
            *ppGlyph = pGlyph;
        }
    }
    else
    {
        *ppGlyph = 0;
    }
}

vre_uint16 vre_ttf_get_num_encodings ( vre_ttf *pTtf )
{
    vre_assert ( pTtf->pCmap );
    
    return pTtf->pCmap->num_encodings;
}

vre_uint8 vre_ttf_get_encoding_format ( vre_ttf *pTtf, vre_uint16 encoding )
{
    vre_assert ( pTtf->pCmap );
    vre_assert ( pTtf->pCmap->num_encodings > encoding );
    vre_assert ( pTtf->pCmap->ppEncodings[encoding] );
    
    return pTtf->pCmap->ppEncodings[encoding]->format;
}

static 
vre_uint32 vre_ttf_map_encoding_format_0 ( ttf_encoding_table_0 *pTable, 
                                           vre_uint32 index )
{  
    vre_assert ( index < 256 );    

    return pTable->glyph_id_array[index];                
}

static 
vre_uint32 vre_ttf_map_encoding_format_4 ( ttf_encoding_table_4 *pTable, 
                                           vre_uint32 index )
{  
    vre_uint32   i;
    vre_uint16  c;
    
    vre_assert ( index < 65536 );
    
    c = ( vre_uint16) index;
    
    // 
    // Search segment 
    // TODO: Convert to binary search.
    // 
    i = 0;
    while ( ( pTable->pEndCount[i] < c ) && 
            ( i < pTable->seg_count_X2 >> 1 ) &&
            ( pTable->pEndCount[i] != 0xFFFF ) )
    {
        i++;
    }
    
    if ( ( pTable->pEndCount[i] >= c) && 
         ( c >= pTable->pStartCount[i] ) )
    {
        if ( pTable->pIdRangeOffset[i] == 0 )
        {
            return ( c + pTable->pIdDelta[i] ) & 0xffff;
        }
        else
        {
            vre_uint16 id = *(pTable->pIdRangeOffset[i]/2 
                              + (c - pTable->pStartCount[i]) 
                              + &pTable->pIdRangeOffset[i]);
            if ( id != 0 )
            {
                return ( id + pTable->pIdDelta[i] ) & 0xffff;
            }
        }
    }
    else
    {
        return 0;
    }
    
    return 0;
}



vre_uint32 vre_ttf_map_encoding ( vre_ttf *pTtf, 
                                  vre_uint32 encoding, 
                                  vre_uint32 index )
{
    ttf_encoding *pEncoding;
    vre_uint32 val = 0;

    if ( pTtf->pCmap == 0 )
    {
        return index;
    }
    vre_assert ( pTtf->pCmap->num_encodings > encoding );
    vre_assert ( pTtf->pCmap->ppEncodings[encoding] );
    
    pEncoding = pTtf->pCmap->ppEncodings[encoding];
    
    switch ( pEncoding->format )
    {
        case 0:              
                val = vre_ttf_map_encoding_format_0((ttf_encoding_table_0 *)
                                                     pEncoding->pTable,index );
                break;
        case 4: 
                val = vre_ttf_map_encoding_format_4((ttf_encoding_table_4 *)
                                                     pEncoding->pTable,index );
                break;     
    }
    
    return val;
}


vre_uint16 vre_ttf_get_upem ( vre_ttf *pTtf )
{
    vre_assert ( pTtf != 0 );
    vre_assert ( pTtf->pHeader != 0 );
    
    return pTtf->pHeader->units_per_em;
}


vre_result vre_load_ttf ( vre_ttf *pTtf, vre_iostream *pStream )
{
    vre_result vres = VRE_ERR_OK;
    
    ttf_table_directory *pTableDir;
    vre_int i;
    
    vre_int endian;
    
    pTtf->pStream = pStream;
    
    endian = vre_iostream_get_endian ( pStream );
    
    // Set input stream to big endian mode
    vre_iostream_set_endian ( pStream, VRE_BIG_ENDIAN );
       
    // read offset table
    vres = vre_read_offset_table ( pStream, &pTtf->pOffsetTab );
    VRE_RETURN_IF_ERR ( vres );
      
    // Read directories
    vres = vre_read_table_directories ( pStream, &pTtf->pTableDir, 
                                        pTtf->pOffsetTab->num_tables );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    // Read Tables
    pTableDir = pTtf->pTableDir;
        
    // read table maxp (maximum profile)
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_MAXP );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_max_profile_table ( pStream, &pTtf->pMaxProfile );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    // read metrics
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_HHEA );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_hhea_table ( pStream, &pTtf->pHhea );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_HMTX );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_htmx_table ( pTtf->pHhea,
                                 pTtf->pMaxProfile,
                                 pStream, 
                                 &pTtf->pHtmx );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    // read header table 
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_HEAD );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_header_table ( pStream, &pTtf->pHeader );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    // read cmap table
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_CMAP );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_cmap_table ( pStream, &pTtf->pCmap );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    // Read Index to loc table
    vres = vre_seek_table ( pStream, pTtf, TTF_TAG_LOCA );
    VRE_GOTO_EXIT_IF_ERR (vres );
    
    vres = vre_read_locate_table ( pStream, &pTtf->pLoc, 
                                   pTtf->pHeader->index_to_loc_format == 0,
                                   pTtf->pMaxProfile->num_glyphs );
    VRE_GOTO_EXIT_IF_ERR (vres );
        
    // read remaining tables        
    for ( i = 0; i < pTtf->pOffsetTab->num_tables; i++ )
    {
        vres = vre_iostream_seek ( pStream, pTableDir->offset );
        VRE_GOTO_EXIT_IF_ERR (vres );
    
        switch ( pTableDir->tag )
        {                                        
            case TTF_TAG_GLYF:
                vre_read_glyph_table ( pStream, pTtf );
                break;
                
            case TTF_TAG_KERN:
                vres = vre_read_kerning_table ( pStream, &pTtf->pKerning );
                VRE_GOTO_EXIT_IF_ERR (vres );
        }
        
        pTableDir ++;    
    }


exit:
    if ( vres != VRE_ERR_OK )
    {
        // Free allocated resources
        
    }

  //  vre_iostream_set_endian ( pStream, endian );

    return vres;
}


static vre_result vre_seek_table ( vre_iostream *pStream, 
                                   vre_ttf *pTtf, 
                                   vre_uint32 tag )
{
    vre_result vres = VRE_ERR_OK;
    vre_int i;
    ttf_table_directory *pTableDir;
    
    pTableDir = pTtf->pTableDir;

    // TODO: convert to binary search
    for ( i = 0; i < pTtf->pOffsetTab->num_tables; i++ )
    {
        if ( pTableDir->tag == tag )
        {
            vres = vre_iostream_seek ( pStream, pTableDir->offset );
            VRE_GOTO_EXIT_IF_ERR (vres );
            break;
        }
        
        pTableDir ++;
    }
    
exit:
    
    return vres;            
}



static vre_result vre_read_offset_table ( vre_iostream *pStream, 
                                          ttf_offset_table **ppTable)
{
    vre_result vres = VRE_ERR_OK;
    
    ttf_offset_table *pTable;
    
    pTable = (ttf_offset_table*) vre_malloc ( sizeof ( ttf_offset_table ) );
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    *ppTable = pTable;
    

    vres = vre_iostream_read_sdword ( pStream, &(pTable->version) );
    VRE_GOTO_EXIT_IF_ERR ( vres );
    
    // Check for Microsoft/Adobe TTF format    
    if ( pTable->version != 0x00010000 )
    {
        goto exit;
    }
    
    vres = vre_iostream_read_word ( pStream, &(pTable->num_tables) );
    VRE_GOTO_EXIT_IF_ERR ( vres );
    
    vres = vre_iostream_read_word ( pStream, &(pTable->search_range) );
    VRE_GOTO_EXIT_IF_ERR ( vres );
    
    vres = vre_iostream_read_word ( pStream, &(pTable->entry_selector) );
    VRE_GOTO_EXIT_IF_ERR ( vres );
   
    vres = vre_iostream_read_word ( pStream, &(pTable->range_shift) );
    VRE_GOTO_EXIT_IF_ERR ( vres );

exit:
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTable );   
        *ppTable = 0;
    }

    return vres;
}


static vre_result vre_read_table_directories ( vre_iostream *pStream, 
                                               ttf_table_directory **ppTableDir,
                                               vre_uint16 num_tables )
{
    vre_result vres = VRE_ERR_OK;
    
    ttf_table_directory *pTableDir;
    
    vre_int i;
    
    pTableDir = (ttf_table_directory*) vre_malloc ( num_tables * 4 * 4 );
    if ( pTableDir == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    *ppTableDir = pTableDir;

    for ( i = 0; i < num_tables; i++ )
    {
        vres = vre_iostream_read_dword ( pStream, &pTableDir->tag );
        VRE_BREAK_IF_ERR ( vres );       
        
        vres = vre_iostream_read_dword ( pStream, &pTableDir->checksum );
        VRE_BREAK_IF_ERR ( vres );
               
        vres = vre_iostream_read_dword ( pStream, &pTableDir->offset );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_dword ( pStream, &pTableDir->length );
        VRE_BREAK_IF_ERR ( vres );
        
        pTableDir ++;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( *ppTableDir );
        *ppTableDir = 0;
    }
    
    return vres;    
}



static vre_result vre_read_header_table ( vre_iostream *pStream, 
                                          ttf_header_table **ppHeader )
{
    vre_result vres = VRE_ERR_OK;
    ttf_header_table *pHeader;
    
    pHeader = (ttf_header_table*) vre_malloc ( sizeof (ttf_header_table) );
    
    if ( pHeader == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    *ppHeader = pHeader;
    
    for (;;) 
    {
        vres = vre_iostream_read_sdword ( pStream, &pHeader->table_version );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_sdword ( pStream, &pHeader->font_version );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_dword ( pStream, 
                                         &pHeader->checksum_adjustment );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_dword ( pStream, &pHeader->magic );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word ( pStream, &pHeader->flags );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pHeader->units_per_em );
        VRE_BREAK_IF_ERR ( vres );
        
        // Read Dates
        {
            vre_uint8   dummy[16];
            
            vres = vre_iostream_read ( pStream, dummy, 16 );
            VRE_BREAK_IF_ERR ( vres );
        }
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->x_min );
        VRE_BREAK_IF_ERR ( vres );        

        vres = vre_iostream_read_sword ( pStream, &pHeader->y_min );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->x_max );
        VRE_BREAK_IF_ERR ( vres );        

        vres = vre_iostream_read_sword ( pStream, &pHeader->y_max );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word ( pStream, &pHeader->mac_style );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pHeader->lowest_rec_PPEM );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_sword ( pStream, 
                                         &pHeader->font_direction_hint );
        VRE_BREAK_IF_ERR ( vres );        

        vres = vre_iostream_read_sword ( pStream, 
                                         &pHeader->index_to_loc_format );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_sword ( pStream, 
                                         &pHeader->glyph_data_format );
        VRE_BREAK_IF_ERR ( vres );

        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pHeader );
        *ppHeader = 0;
    }
        
    return vres;
}



static vre_result vre_read_encoding ( vre_iostream *pStream,
                                      ttf_encoding **ppEncoding );

vre_result vre_read_cmap_table ( vre_iostream *pStream,
                                 ttf_cmap_table **ppCmap )
{
    vre_result  vres = VRE_ERR_OK;
    ttf_encoding_record *pEncRecords = 0;
    ttf_cmap_table *pCmap;
    vre_uint32 start_offset;
    vre_int i;
	vre_uint16  version;


    //
    // Create structures.
    //
    
    pCmap = (ttf_cmap_table *) vre_malloc ( sizeof ( ttf_cmap_table ));
    if ( pCmap == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset (pCmap, 0, sizeof ( ttf_cmap_table));
        
    for (;;) 
    {        
        start_offset = vre_iostream_get_pos ( pStream );
    
        //
        // Read cmap header
        //
        
        vres = vre_iostream_read_word ( pStream, &version );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word ( pStream, &pCmap->num_encodings );
        VRE_BREAK_IF_ERR ( vres );
        
        //
        // Alloc memory for encoding records        
        //
                
        pEncRecords = (ttf_encoding_record*) 
                      vre_malloc ( sizeof (ttf_encoding_record) * 
                                           pCmap->num_encodings);
        if ( pEncRecords == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for (i = 0; i < pCmap->num_encodings; i++)
        {        
            vre_uint16 platform_id;
            vre_uint16 encoding_id;
            vre_uint32 offset;
        
            vres = vre_iostream_read_word ( pStream, &platform_id );
            VRE_BREAK_IF_ERR ( vres );
                
            vres = vre_iostream_read_word ( pStream, &encoding_id );
            VRE_BREAK_IF_ERR ( vres );

            vres = vre_iostream_read_dword ( pStream, &offset );
            VRE_BREAK_IF_ERR ( vres );
            
            pEncRecords[i].platform_id = platform_id;
            pEncRecords[i].encoding_id = encoding_id;
            pEncRecords[i].offset = offset;
                                                              
        }
        VRE_BREAK_IF_ERR ( vres );
        
        pCmap->ppEncodings = (ttf_encoding**) 
                             vre_malloc ( sizeof (ttf_encoding*) * 
                             pCmap->num_encodings);
                       
        for (i = 0; i < pCmap->num_encodings; i++)
        {
            vres = vre_iostream_seek ( pStream, start_offset + 
                                       pEncRecords[i].offset );
            VRE_BREAK_IF_ERR ( vres );
                           
            vres = vre_read_encoding ( pStream, &pCmap->ppEncodings[i] );
            VRE_BREAK_IF_ERR ( vres );
            
            pCmap->ppEncodings[i]->platform_id = pEncRecords[i].platform_id;
            pCmap->ppEncodings[i]->encoding_id = pEncRecords[i].encoding_id;          
            
        }
        VRE_BREAK_IF_ERR ( vres );
    
        break;
    }
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pEncRecords );
        vre_ttf_destroy_cmap_table (pCmap);
    }
    
    *ppCmap = pCmap;
    
    vre_free ( pEncRecords );

    return vres;
}

static void vre_ttf_destroy_encoding_format0 ( void *pTable )
{
    vre_free ( pTable );
}

static void vre_ttf_destroy_encoding_format4 ( void *pTable )
{
    ttf_encoding_table_4 *pTable4 = (ttf_encoding_table_4 *) pTable;
    
    vre_free (pTable4->pEndCount);
    vre_free (pTable4->pStartCount);
    vre_free (pTable4->pIdDelta);
    vre_free (pTable4->pIdRangeOffset);
    
    vre_free ( pTable );
}

void vre_ttf_destroy_cmap_table ( ttf_cmap_table *pCmap )
{
    vre_uint32 i;
    
    if ( pCmap != 0 )
    {
        for ( i = 0; i < pCmap->num_encodings; i++ )
        {
            ttf_encoding *pEncoding = pCmap->ppEncodings[i];
            switch ( pEncoding->format )
            {
                    case 0:
                        vre_ttf_destroy_encoding_format0 ( pEncoding->pTable );
                        break;
                    case 4:
                        vre_ttf_destroy_encoding_format4 ( pEncoding->pTable );
                        break;
            }
            vre_free ( pEncoding );
        }
        vre_free ( pCmap->ppEncodings );
        vre_free ( pCmap );
    }
}

static vre_result vre_read_encoding_format_0 ( vre_iostream *pStream,
                                               ttf_encoding *pEncoding )
{ 
    ttf_encoding_table_0 *pTable;
    vre_result vres = VRE_ERR_OK;
    
    pEncoding->pTable = 0;
    
    pTable = ( ttf_encoding_table_0 *) 
             vre_malloc (sizeof (ttf_encoding_table_0));
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }             
    
    for (;;)
    {
        vres = vre_iostream_read_word( pStream, &pEncoding->length );
        VRE_BREAK_IF_ERR ( vres );
    
        vres = vre_iostream_read_word( pStream, &pEncoding->language );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read ( pStream, pTable->glyph_id_array, 256 );
        VRE_BREAK_IF_ERR ( vres );
  
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTable );
    }
    else
    {
        pEncoding->pTable = (void*) pTable;
    }
   
    return vres;
}



static vre_result vre_read_encoding_format_4 ( vre_iostream *pStream,
                                               ttf_encoding *pEncoding )
{ 
    ttf_encoding_table_4 *pTable;
    vre_result vres = VRE_ERR_OK;
    vre_uint32 i;
    vre_uint32 idGlyphIndexLength;
    
    pEncoding->pTable = 0;
    
    pTable = ( ttf_encoding_table_4 *) 
        vre_malloc (sizeof (ttf_encoding_table_4));
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset(pTable, sizeof(ttf_encoding_table_4), 0);
    
    for (;;)
    {
        vres = vre_iostream_read_word( pStream, &pEncoding->length );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word( pStream, &pEncoding->language );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word( pStream, &pTable->seg_count_X2 );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word( pStream, &pTable->search_range );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word( pStream, &pTable->entry_selector );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word( pStream, &pTable->range_shift );
        VRE_BREAK_IF_ERR ( vres );

        // Read End Counts
        pTable->pEndCount = 
            (vre_uint16*) vre_malloc ( sizeof (vre_uint16) * 
                                       (pTable->seg_count_X2 >> 1));
        if ( pTable->pEndCount == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < pTable->seg_count_X2 >> 1; i++ )
        {
            vres = vre_iostream_read_word( pStream, &pTable->pEndCount[i] );
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_read_word( pStream, &pTable->reserved_pad );
        VRE_BREAK_IF_ERR ( vres );
        
        // Read Start Counts
        pTable->pStartCount = 
            (vre_uint16*) vre_malloc ( sizeof (vre_uint16) * 
                                       (pTable->seg_count_X2 >> 1));
        if ( pTable->pStartCount == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < pTable->seg_count_X2 >> 1; i++ )
        {
            vres = vre_iostream_read_word( pStream, &pTable->pStartCount[i] );
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        
        // Read Deltas
        pTable->pIdDelta = 
            (vre_int16*) vre_malloc ( sizeof (vre_int16) * 
                                      (pTable->seg_count_X2 >> 1));
        if ( pTable->pIdDelta == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < pTable->seg_count_X2 >> 1; i++ )
        {
            vres = vre_iostream_read_sword( pStream, &pTable->pIdDelta[i] );
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        // Read range offsets (and glypIdArray)
        // Note: glyphIdArray is read in the same array as pIdRangeOffset
        // in order to use Microsofts "obscure indexing trick".
        idGlyphIndexLength = 
            pEncoding->length - ( 16 + pTable->seg_count_X2 * 4 );
        
        pTable->pIdRangeOffset = 
            (vre_uint16*) vre_malloc ( sizeof (vre_uint16) * 
                                       (pTable->seg_count_X2 >> 1) +
                                       idGlyphIndexLength);
        if ( pTable->pIdRangeOffset == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        for ( i = 0; i < (pTable->seg_count_X2+idGlyphIndexLength) >> 1; i++ )
        {
            vres = vre_iostream_read_word( pStream, &pTable->pIdRangeOffset[i]);
            VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free (pTable->pEndCount);
        vre_free (pTable->pStartCount);
        vre_free (pTable->pIdDelta);
        vre_free (pTable->pIdRangeOffset);
        
        vre_free ( pTable );
    }
    else
    {
        pEncoding->pTable = (void*) pTable;
    }
    
    return vres;
}




static vre_result vre_read_encoding ( vre_iostream *pStream,
                                      ttf_encoding **ppEncoding )
{
    vre_result vres = VRE_ERR_OK;
    ttf_encoding *pEncoding;

    *ppEncoding = 0;
    
    pEncoding = ( ttf_encoding* ) vre_malloc ( sizeof ( ttf_encoding ) );
    if ( pEncoding == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    for (;;)
    {
        //
        // Read encoding format
        //
        vres = vre_iostream_read_word( pStream, &pEncoding->format );
        VRE_BREAK_IF_ERR ( vres );                

        switch ( pEncoding->format )
        {
            case 0: vres = vre_read_encoding_format_0 ( pStream, pEncoding );
                    break;
/*                    
            case 2: vres = vre_read_encoding_format_2 ( pStream, pEncoding );
                    break;
  */
            case 4: vres = vre_read_encoding_format_4 ( pStream, pEncoding );
                    break;
    /*
            case 6: vres = vre_read_encoding_format_6 ( pStream, pEncoding );
                    break;
            case 8: vres = vre_read_encoding_format_8 ( pStream, pEncoding );
                    break;
            case 10:vres = vre_read_encoding_format_10 ( pStream, pEncoding );
                    break;
            case 12:vres = vre_read_encoding_format_12 ( pStream, pEncoding );
                    break;
                    */
        }
        
        break;    
    }
    
    *ppEncoding = pEncoding;

    return vres;
}


//
// Glyph Tables
//

static vre_result vre_read_locate_table ( vre_iostream *pStream,                                           
                                          void **ppLoc,
                                          vre_bool format_short,
                                          vre_uint16 num_glyphs)
 {
    vre_uint16 i;
    vre_result vres = VRE_ERR_OK;
   
    
    
    *ppLoc = 0;
     
    if ( format_short )
    {
        vre_uint16 prev, loc;
        vre_uint16 *pLoc = (vre_uint16*) vre_malloc ( num_glyphs * 2 );
        
        if ( pLoc == 0 )
        {
            return VRE_ERR_MEM_ALLOC_FAILED;
        }
        
        prev = 0xffff;
        
        for ( i = 0; i < num_glyphs; i++ )
        {
            vres = vre_iostream_read_word ( pStream, &loc );
            
            if ( vres != VRE_ERR_OK )
            {
                vre_free ( pLoc );
                return vres;
            }
            
            if ( prev == loc )
            {
                pLoc[i-1] = 0xffff;
            }
            
            pLoc[i] = loc;
                        
            prev = loc;
        }
        
        *ppLoc = pLoc;
    }
    else
    {
        vre_uint32 prev;
        vre_uint32 *pLoc = (vre_uint32 *) vre_malloc ( num_glyphs * 4 );
        if ( pLoc == 0 )
        {
            return VRE_ERR_MEM_ALLOC_FAILED;
        }
        
        prev = 0xffffffff;
        
        for ( i = 0; i < num_glyphs; i++ )
        {
            vre_uint32 loc;
            vres = vre_iostream_read_dword ( pStream, &loc );
            
            if ( vres != VRE_ERR_OK )
            {
                vre_free ( pLoc );
                return vres;
            }
            
            if ( prev == loc ) 
            {
                pLoc[i-1] = 0xffffffff;
            }
            
            pLoc[i] = loc;
                        
            prev = loc;
        }
        
        *ppLoc = pLoc;
    }
    
    return VRE_ERR_OK;
 }

static vre_result vre_read_glyph_from_id ( vre_iostream *pStream, 
                                           vre_ttf *pTtf, 
                                           vre_uint32 glyph_id,
                                           vre_meta_glyph **ppGlyph)
{
    vre_result vres = VRE_ERR_OK;
    
    vre_uint32 offset;
    vre_uint32 start_offset = pTtf->glyph_start_offset;
    
    *ppGlyph = 0;
    
    if ( pTtf->pHeader->index_to_loc_format == 0 )
    {
        offset = (((vre_uint16*) pTtf->pLoc)[glyph_id]);
        if ( offset == 0xffff )
        {
            return VRE_ERR_OK;
        }
        
        offset <<=1 ;
    }
    else
    {
        offset = ((vre_uint32*) pTtf->pLoc)[glyph_id];
        
        if ( offset == 0xffffffff )
        {
            return VRE_ERR_OK;
        }
    }
    
    vres = vre_iostream_seek ( pStream, start_offset + offset );
    VRE_RETURN_IF_ERR ( vres );
    
    vres = vre_read_glyph ( pStream, ppGlyph );
    VRE_RETURN_IF_ERR ( vres );
    
    return vres;
}


static void vre_read_glyph_table ( vre_iostream *pStream,
                                         vre_ttf *pTtf )
{
    pTtf->glyph_start_offset = vre_iostream_get_pos ( pStream ); 
}

                                  

static vre_result vre_read_glyph ( vre_iostream *pStream,
                                   vre_meta_glyph **ppGlyph )
{
    vre_result           vres = VRE_ERR_OK;

    ttf_glyph_header     glyph_header;    
    vre_meta_glyph       *pTtfGlyph;
    
           
    *ppGlyph = 0;      
    
    vres = vre_read_glyph_header ( pStream, &glyph_header );
    if ( vres != VRE_ERR_OK )
    {
        return vres;
    }
    
    pTtfGlyph = VRE_TALLOC ( vre_meta_glyph );
    if ( pTtfGlyph == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;        
    }
                    
    for (;;)
    {
        //
        // Read glyph simple (same as vre_glyph)
        //                                    
        if ( glyph_header.num_contours >= 0 )
        {
            vre_glyph *pGlyph;
            
            vres = vre_create_glyph ( &pGlyph, glyph_header.num_contours );
            VRE_BREAK_IF_ERR ( vres );
       
            if ( glyph_header.num_contours != 0 )
            {
                vre_glyph_set_bounding_box ( pGlyph, 
                                             glyph_header.x_min,
                                             glyph_header.y_min,
                                             glyph_header.x_max,
                                             glyph_header.y_max );
        
                vres = vre_read_glyph_simple ( pStream, pGlyph, 
                                               glyph_header.num_contours );
                                           
            }
            
            if ( vres != VRE_ERR_OK )
            {
                vre_glyph_destroy ( pGlyph );
                break;
            }                                           
                                           
            pTtfGlyph->glyph_type = 0;
            pTtfGlyph->pGlyph = pGlyph;
        }
        
        //
        // Read composite glyph.
        //
        else
        {
            vre_list *pComponentList;
        
            vres = vre_read_glyph_composite ( pStream, &pComponentList );
            VRE_BREAK_IF_ERR ( vres );
            
            pTtfGlyph->glyph_type = 1;
            pTtfGlyph->pGlyph = pComponentList;
        }                                
                                
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTtfGlyph );
        *ppGlyph = 0;
    }
    
    *ppGlyph = pTtfGlyph;
    
    return vres;       
}


static vre_result vre_read_glyph_header ( vre_iostream *pStream, 
                                          ttf_glyph_header *pHeader )
{
    vre_result vres = VRE_ERR_OK;

    vres = vre_iostream_read_sword ( pStream, &(pHeader->num_contours) );
    VRE_RETURN_IF_ERR ( vres );
       
    vres = vre_iostream_read_sword ( pStream, &(pHeader->x_min) );
    VRE_RETURN_IF_ERR ( vres );
    
    vres = vre_iostream_read_sword ( pStream, &(pHeader->y_min) );
    VRE_RETURN_IF_ERR ( vres );
    
    vres = vre_iostream_read_sword ( pStream, &(pHeader->x_max) );
    VRE_RETURN_IF_ERR ( vres );
    
    vres = vre_iostream_read_sword ( pStream, &(pHeader->y_max) );
    VRE_RETURN_IF_ERR ( vres );
    
    return vres;
}

static vre_result vre_read_glyph_points ( vre_iostream *pStream,
                                          vre_glyph *pGlyph,
                                          vre_uint8 *pFlags,
                                          vre_uint16 *pEndContoursPts,
                                          vre_uint16 num_contours )
{    
    vre_result vres = VRE_ERR_OK;
    vre_uint16 num_points;
    vre_uint16 *pNumPointsPerSegment;
    vre_uint16 i, j, k;
   
    vre_uint16 num_points_in_segment;
    vre_uint8 xb_coord, yb_coord;
    vre_int32 x_coord, y_coord;
    vre_uint16 seg_index;
    
    vre_uint16 start_point;
    
    vre_point *pPoints; // Array to be moved to pTtf
    
    num_points = pEndContoursPts[num_contours-1] + 1;
    
    // TEMP ( should be allocated only once (since we now max num of points)
    pPoints = vre_malloc ( sizeof ( vre_point ) * num_points );
    vre_memset ( pPoints, 
                 sizeof ( vre_point ) * num_points,
                 0 );
    // TEMP

    vre_memset ( pGlyph->pContours, 
                 sizeof ( vre_glyph_contour ) * num_contours,
                 0 );
    
    //
    // Alloc memory for temporary array
    //
    pNumPointsPerSegment = ( vre_uint16 *) vre_malloc( 2 * num_points );
    if ( pNumPointsPerSegment == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    for (;;)
    {
        x_coord = 0;
        y_coord = 0;
        
        //
        // Read X Coordinates
        //
        for ( k = 0; k < num_points; k++ )
        {                        
            if ( pFlags[k] & GLYPH_FLG_X_SHORT )
            {
                vres = vre_iostream_read_byte ( pStream, &xb_coord);
                VRE_BREAK_IF_ERR ( vres );
                
                if ( pFlags[k] & GLYPH_FLG_X_SAME )
                {
                    x_coord += (vre_int32) xb_coord;
                }                    
                else
                {
                    x_coord -= (vre_int32) xb_coord;
                }
            }
            else if ( ( pFlags[k] & GLYPH_FLG_X_SAME ) == 0)
            {
                vre_int16 tmp;
                vres = vre_iostream_read_sword ( pStream, &tmp);
                VRE_BREAK_IF_ERR ( vres );
                
                x_coord += tmp;                    
            }
            
            pPoints[k].x = x_coord;
        }
        
        //
        // Read Y Coordinates
        //
        for ( k = 0; k < num_points; k++ )
        {                               
            if ( pFlags[k] & GLYPH_FLG_Y_SHORT )
            {
                vres = vre_iostream_read_byte ( pStream, &yb_coord);
                VRE_BREAK_IF_ERR ( vres );
                
                if ( pFlags[k] & GLYPH_FLG_Y_SAME )
                {
                    y_coord += (vre_int32) yb_coord;
                }                    
                else
                {
                    y_coord -= (vre_int32) yb_coord;
                }
            }
            else if ( ( pFlags[k] & GLYPH_FLG_Y_SAME ) == 0)
            {
                vre_int16 tmp;
                vres = vre_iostream_read_sword ( pStream, &tmp);
                VRE_BREAK_IF_ERR ( vres );
                
                y_coord += tmp;
            }                   
            
            pPoints[k].y = y_coord;
        }  
        
        // Organize in contours and in segments.
        k = 0;
        for ( i = 0; i < num_contours; i++ )
        {
            seg_index = 0;
            
            if ( i > 0 )
            {
                num_points = pEndContoursPts[i] - pEndContoursPts[i-1];
            }
            else
            {
                num_points = pEndContoursPts[0] + 1;
            }
            
            if ( num_points == 0 )
            {
                vres = VRE_ERR_FILE_FORMAT_CORRUPT;
                break;
            }
            
            //
            // Alloc memory for point array.
            // ( +2 because it can happen that first and last are off curve).
            pGlyph->pContours[i].pPoints = 
                ( vre_point* ) vre_malloc( sizeof( vre_point )*(num_points + 2));
            
            if ( pGlyph->pContours[i].pPoints == 0 )
            {
                vres = VRE_ERR_MEM_ALLOC_FAILED;
                break;
            }
            
            start_point = 0;
       
            if ( !(pFlags[k] & GLYPH_FLG_ON_CURVE) )
            {
                vre_uint32 p = k+num_points-1;
                if ( !(pFlags[p] & GLYPH_FLG_ON_CURVE) )
                {
                    pGlyph->pContours[i].pPoints[0].x = 
                        (pPoints[k].x + pPoints[p].x + 1) / 2;
                    pGlyph->pContours[i].pPoints[0].y = 
                        (pPoints[k].y + pPoints[p].y + 1) / 2;
                }
                else
                {
                    pGlyph->pContours[i].pPoints[0].x = pPoints[p].x;
                    pGlyph->pContours[i].pPoints[0].y = pPoints[p].y;
                }
                
                start_point = 1;
            }
            
            num_points_in_segment = start_point;
            
            //
            // Organize into segments
            //        
            for ( j = start_point; j < num_points + start_point; j++ )
            {
                num_points_in_segment ++;
                
                if ( pFlags[k] & GLYPH_FLG_ON_CURVE  ) 
                {
                    if ( num_points_in_segment > 1 )
                    {
                        pNumPointsPerSegment[seg_index] = num_points_in_segment;
                        num_points_in_segment = 1;
                        seg_index ++;
                    }
                }
                
                pGlyph->pContours[i].pPoints[j].x = pPoints[k].x;
                pGlyph->pContours[i].pPoints[j].y = pPoints[k].y;
                
                k++;    
            }
            
            //
            // Check if last point is off curve
            //          
            if (!( pFlags[k-1] & GLYPH_FLG_ON_CURVE ))
            {
                pNumPointsPerSegment[seg_index] = num_points_in_segment+1;                
                seg_index++;
                pGlyph->pContours[i].pPoints[j].x = 
                    pGlyph->pContours[i].pPoints[0].x;
                pGlyph->pContours[i].pPoints[j].y = 
                    pGlyph->pContours[i].pPoints[0].y;
            }
            
            // 
            // Alloc exact size array for segments            
            // 
            pGlyph->pContours[i].pSegments = 
                ( vre_uint16 *) vre_malloc( 2 * seg_index );
            if ( pGlyph->pContours[i].pSegments == 0 )
            {
                vres = VRE_ERR_MEM_ALLOC_FAILED;
                break;
            }
            
            pGlyph->pContours[i].num_segments = seg_index;
            
            //
            // Copy segments from temporary array to contour array
            //
            vre_memcopy8 ( pGlyph->pContours[i].pSegments,
                           pNumPointsPerSegment, seg_index * 2 );
            
            VRE_BREAK_IF_ERR ( vres );
        }
        
        break;
    }
    if ( vres != VRE_ERR_OK )
    {
        for ( k = 0; k < i; k++ )
        {
            vre_free ( pGlyph->pContours[i].pPoints );
            pGlyph->pContours[i].pPoints = 0;
            
        }
        vre_free ( pNumPointsPerSegment );
    }
    
    //
    // Free tmp array
    //
    vre_free ( pNumPointsPerSegment );
    vre_free ( pPoints );
    
    return vres;
}
        
static vre_result vre_read_glyph_simple ( vre_iostream *pStream,
                                          vre_glyph *pGlyph,
                                          vre_int16 num_contours )
{
    vre_result  vres = VRE_ERR_OK;
    vre_uint16  *pEndContoursPts;
    vre_uint16  instruction_length;
    vre_uint8   *pFlags;
    vre_uint16  num_points;
    vre_int16   i, j;
    
    pGlyph->num_contours = num_contours;
    
    pEndContoursPts = (vre_uint16*) vre_malloc ( 2 * num_contours );
    if ( pEndContoursPts == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    // read contours end points (last element is number of points)
    for ( i = 0; i < num_contours; i++ )
    {
        vres = vre_iostream_read_word ( pStream, &pEndContoursPts[i] );
        VRE_BREAK_IF_ERR ( vres );    
    }
    
    for (;;)
    {    
        VRE_BREAK_IF_ERR ( vres );
    
        vres = vre_iostream_read_word ( pStream, &instruction_length );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_iostream_skip ( pStream, instruction_length );
        VRE_BREAK_IF_ERR ( vres );
     
        break;    
        
    }
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pEndContoursPts );
        return vres;
    }
        
    num_points = pEndContoursPts[num_contours-1] + 1;
        
    for (;;)
    {    
        // alloc number_of_points flags
        pFlags = ( vre_uint8* ) vre_malloc ( num_points );
        if ( pFlags == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
    
        //parse flags and fill pFlags array
        i = 0;
        do       
        {
            vre_uint8 flag;
            vres = vre_iostream_read_byte ( pStream, &flag );            
            VRE_BREAK_IF_ERR ( vres );      
        
            if ( flag & GLYPH_FLG_REPEAT)
            {
                vre_uint8 repetitions;
                
                vres = vre_iostream_read_byte ( pStream, &repetitions );
                VRE_BREAK_IF_ERR ( vres );
                
                flag &= ~GLYPH_FLG_REPEAT;
                for ( j = 0; j < repetitions + 1; j++ )
                {
                    pFlags[i] = flag;
                    i++;
                }
            }
            else
            {                                      
                pFlags[i] = flag;
                i++;
            }
        } 
        while ( i < num_points );
    
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
        
        vres = vre_read_glyph_points ( pStream, pGlyph, pFlags, 
                                       pEndContoursPts, num_contours );
        break;
    }
    
    vre_free ( pEndContoursPts );
    vre_free ( pFlags );

    return vres;
}



static vre_result vre_read_glyph_composite ( vre_iostream *pStream,
                                             vre_list **ppComponentList )
{
    vre_result vres = VRE_ERR_OK;
    
    vre_uint16 flags;
    vre_uint16 glyph_id;

    vre_uint32 arg1, arg2;
    
    vre_glyph_component *pComponent;
    
    vre_list *pComponentList;

    vres = vre_list_create ( &pComponentList );
    VRE_RETURN_IF_ERR ( vres );
                        
    do
    {   
        pComponent = VRE_TALLOC ( vre_glyph_component );
        if ( pComponent == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
     
        vres = vre_iostream_read_word ( pStream, &flags );
        VRE_BREAK_IF_ERR ( vres );
    
        vres = vre_iostream_read_word ( pStream, &glyph_id );
        VRE_BREAK_IF_ERR ( vres );
        
        pComponent->flags = flags;
        pComponent->glyph_id = glyph_id;
                        
        if ( flags & ARG_1_AND_2_ARE_WORDS )
        {
            vre_int16 arg1W, arg2W;
            
            vres = vre_iostream_read_sword ( pStream, &arg1W );
            VRE_BREAK_IF_ERR ( vres );
    
            vres = vre_iostream_read_sword ( pStream, &arg2W );
            VRE_BREAK_IF_ERR ( vres );
            
            arg1 = arg1W;
            arg2 = arg2W;
        }
        else
        {
            vre_int8 arg1B, arg2B;
            
            vres = vre_iostream_read_sbyte ( pStream, &arg1B );
            VRE_BREAK_IF_ERR ( vres );
    
            vres = vre_iostream_read_sbyte ( pStream, &arg2B );
            VRE_BREAK_IF_ERR ( vres );
                        
            arg1 = arg1B;
            arg2 = arg2B;            
        }
                        
        vre_matrix_identity ( &pComponent->mat );
    
        if ( flags & WE_HAVE_A_SCALE ) 
        {
            vre_int16 scale;
            vres = vre_iostream_read_sword ( pStream, &scale);
            VRE_BREAK_IF_ERR ( vres );
     
            vre_matrix_scale ( &pComponent->mat, ((vre_uint32)scale)<<2, 
                                                 ((vre_uint32)scale)<<2);
    	} 
    	else if ( flags & WE_HAVE_AN_X_AND_Y_SCALE ) 
    	{
            vre_int16 sx, sy;    	
            vres = vre_iostream_read_sword ( pStream, &sx);
            VRE_BREAK_IF_ERR ( vres );
	        
	        vres = vre_iostream_read_sword ( pStream, &sy);
            VRE_BREAK_IF_ERR ( vres );
          
            vre_matrix_scale ( &pComponent->mat, 
                               ((vre_uint32)sx)<<2, 
                               ((vre_uint32)sy)<<2);
	    } 	    	    
	    else if ( flags & WE_HAVE_A_TWO_BY_TWO ) 
	    {
	        vre_int16 s0, s1, s2, s3;
	    	    
  		    vres = vre_iostream_read_sword ( pStream, &s0);
            VRE_BREAK_IF_ERR ( vres );
	    
	        vres = vre_iostream_read_sword ( pStream, &s1);
            VRE_BREAK_IF_ERR ( vres );
	    
	        vres = vre_iostream_read_sword ( pStream, &s2);
            VRE_BREAK_IF_ERR ( vres );

	        vres = vre_iostream_read_sword ( pStream, &s3);
            VRE_BREAK_IF_ERR ( vres );

            vre_matrix_set ( &pComponent->mat, 
                             ((vre_uint32)s0)<<2, ((vre_uint32)s1)<<2, 0,
                             ((vre_uint32)s2)<<2, ((vre_uint32)s3)<<2, 0,
                             0, 0, 1 );
		}
		
		if ( flags & ARGS_ARE_XY_VALUES )
		{
		    vre_matrix_translate ( &pComponent->mat, arg1, arg2 );		
		}
		
		vres = vre_list_put_last ( pComponentList, (void*) pComponent );
		VRE_BREAK_IF_ERR ( vres );
		
		pComponent = 0;
	      		
    } while ( flags & MORE_COMPONENTS );
        
    if ( vres != VRE_ERR_OK )
    {
        vre_free (pComponent);
        vre_list_destroy ( pComponentList );
    }
    
    *ppComponentList = pComponentList;
            
    return VRE_ERR_OK;
}

