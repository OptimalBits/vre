

#ifndef VRE_TTF_MAX_PROFILE_H
#define VRE_TTF_MAX_PROFILE_H

#include "vre_defs.h"
#include "vre_iostream.h"

typedef struct ttf_max_profile_table
{
    vre_fix16   table_version;
    vre_uint16 	num_glyphs; // 	The number of glyphs in the font.
    vre_uint16  max_points; // Maximum points in a non-composite glyph.
    vre_uint16  max_contours; // Maximum contours in a non-composite glyph.
    vre_uint16  max_composite_points; // Maximum points in a composite glyph.
    vre_uint16  max_composite_contours; 
    // Maximum contours in a composite glyph.
    vre_uint16  max_zones; 
    // 1 if instructions do not use the twilight zone (Z0), 
    // or 2 if instructions do use Z0; should be set to 2 
    // in most cases.
    vre_uint16  max_twilight_points;   // Maximum points used in Z0.
    vre_uint16  max_storage;           // Number of Storage Area locations.
    vre_uint16  max_function_defs;     // Number of FDEFs.
    vre_uint16  max_instruction_defs;  // Number of IDEFs.
    vre_uint16  max_stack_elements;    // Maximum stack depth2.
    vre_uint16  max_size_of_instructions; 
    // Maximum byte count for glyph instructions.
    vre_uint16  max_component_elements; //	Maximum number of components 
    // referenced at "top level" for any composite glyph.
    vre_uint16  max_component_depth; // Maximum levels of recursion; 
                                // 1 for simple components.
} ttf_max_profile_table;

vre_result vre_read_max_profile_table ( vre_iostream *pStream,
                                        ttf_max_profile_table **ppTable );

#endif

