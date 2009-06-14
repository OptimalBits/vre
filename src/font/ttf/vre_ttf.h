
#ifndef VRE_TTF_H
#define VRE_TTF_H

#include "vre_font.h"
#include "vre_defs.h"
#include "vre_iostream.h"
#include "vre_ttf_max_profile.h"
#include "vre_ttf_metrics.h"
#include "vre_glyph.h"
#include "vre_cache.h"
#include "vre_matrix.h"

typedef struct ttf_offset_table ttf_offset_table;
typedef struct ttf_table_directory ttf_table_directory;
typedef struct ttf_header_table ttf_header_table;
typedef struct ttf_cmap_table ttf_cmap_table;
typedef struct ttf_glyph_table ttf_glyph_table;
typedef struct vre_ttf_kerning_table vre_ttf_kerning_table;

typedef struct vre_ttf
{
    vre_iostream        *pStream;
    
    ttf_offset_table    *pOffsetTab;
    ttf_table_directory *pTableDir;

    ttf_max_profile_table *pMaxProfile;

    ttf_header_table    *pHeader;
    
    ttf_hhea_table      *pHhea;
    ttf_htmx_table      *pHtmx;
    
    ttf_cmap_table      *pCmap;
    
    void                *pLoc;
    
    vre_ttf_kerning_table   *pKerning;
    
    vre_cache           *pCache;
    
    vre_uint32          glyph_start_offset;

} vre_ttf;


//
// Construc/Destruct
//
vre_result vre_create_ttf ( vre_ttf **ppTtf );
void vre_destroy_ttf ( vre_ttf *pTtf );

//
// Bind to font interface
//
void vre_ttf_bind( vre_ttf *pTtf, vre_font *pFont );

//
// Loading
//
vre_result vre_load_ttf ( vre_ttf *pTtf, vre_iostream *pStream );

//
// Glyphs
//
void vre_ttf_get_glyph ( vre_ttf *pTtf, 
                         vre_meta_glyph **ppGlyph, 
                         vre_uint32 index );
                         
vre_uint16 vre_ttf_get_upem ( vre_ttf *pTtf );

//
// Encoding
//
vre_uint16 vre_ttf_get_num_encodings ( vre_ttf *pTtf );

vre_uint8 vre_ttf_get_encoding_format ( vre_ttf *pTtf, vre_uint16 encoding );

vre_uint32 vre_ttf_map_encoding ( vre_ttf *pTtf, 
                                  vre_uint32 encoding, 
                                  vre_uint32 index );

//
// Metrics
//                                
vre_int32 vre_ttf_get_advance_width ( vre_ttf *pTtf, 
                                      vre_uint16 index );
vre_int16 vre_ttf_get_lsb ( ttf_htmx_table *pTable, vre_uint16 index );

//
// Kerning
//
void vre_ttf_apply_kerning ( vre_ttf *pTtf, 
                             vre_uint16 left_glyph,
                             vre_uint16 right_glyph,
                             vre_int32  *dx,
                             vre_int32  *dy );

#endif
