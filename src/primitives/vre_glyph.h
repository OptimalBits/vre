
#ifndef VRE_GLYPH_H
#define VRE_GLYPH_H

#include "vre_defs.h"
#include "vre_polygon.h"
#include "vre_matrix.h"
#include "vre_rectangle.h"

typedef struct vre_glyph_contour
{
    vre_uint16  num_segments; // a segment can be a quadratic B-spline or a line
    vre_uint16  *pSegments;   // number of points for every segment.
                              // Note that every segment does overlap with 
                              // previous and next segments in one point.    
    vre_point   *pPoints;
} vre_glyph_contour;

typedef struct vre_glyph
{
    vre_uint16          num_contours;
    vre_glyph_contour   *pContours;
    vre_rectangle       bbox;
} vre_glyph;

typedef struct vre_glyph_component
{
    vre_uint16 flags;
    vre_mat3x3 mat;
    vre_uint16 glyph_id;        
} vre_glyph_component;

/**
Meta Glyph structure.
 
 It holds the glyph data. It can be a standalone glyph in the case
 of a simple glyph, or a list of components in case of composite glyphs.
 
 */

typedef struct vre_meta_glyph
{    
    void *pGlyph;
    vre_uint8 glyph_type; // 0 = simple, 1 = composite
} vre_meta_glyph;

///

vre_result vre_create_glyph( vre_glyph **ppGlyph, vre_uint16 num_contours );

void vre_glyph_destroy( vre_glyph *pGlyph );

void vre_meta_glyph_destroy( vre_meta_glyph *pMetaGlyph );

void vre_glyph_set_bounding_box( vre_glyph *pGlyph, 
                                 vre_int16 x_min,
                                 vre_int16 y_min, 
                                 vre_int16 x_max, 
                                 vre_int16 y_max );

vre_result vre_glyph_to_polygon( vre_glyph *pGlyph, 
                                 vre_style *pStyle,
                                 vre_mat3x3 *pMat, 
                                 vre_polygon **ppPoly );

void vre_glyph_get_bounding_box ( vre_glyph *pGlyph,
                                  vre_rectangle *pBBox,
                                  vre_mat3x3 *pMat );

#endif
