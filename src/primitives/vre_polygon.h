
#ifndef VRE_POLYGON_TYPE
#define VRE_POLYGON_TYPE

typedef struct vre_polygon vre_polygon;

#endif


#ifndef VRE_POLYGON_H
#define VRE_POLYGON_H

#include "vre_defs.h"
#include "vre_context.h"
#include "vre_matrix.h"

vre_result vre_polygon_create ( vre_polygon **ppPoly, 
								vre_point *pVertices,
								vre_uint32 num_points );

vre_result vre_polygon_create2 ( vre_polygon **ppPoly, 								
								 vre_uint32 num_points );
								 
vre_result vre_polygon_create3 ( vre_polygon **ppPoly,
								 vre_uint32 num_points,
								 vre_uint16 num_contours );

vre_result vre_polygon_createEx ( vre_polygon **ppPoly );

void vre_polygon_destroy ( vre_polygon *pPolygon  );

void vre_polygon_copy ( vre_polygon *pSrc, vre_polygon *pDst );

void vre_polygon_transform ( vre_polygon *pPoly, vre_mat3x3 *pMatrix );

vre_result 
vre_polygon_add_point ( vre_polygon *pPoly, vre_point *pPoint );

vre_result
vre_polygon_add_points ( vre_polygon *pPoly, vre_point *pPoint, vre_uint32 n );

vre_point *vre_polygon_get_point ( vre_polygon *pPoly, vre_uint32 point );

vre_result
vre_polygon_start_contour ( vre_polygon *pPoly );

vre_result
vre_polygon_end_contour ( vre_polygon *pPoly );

vre_uint32 
vre_polygon_get_contour ( vre_polygon *pPoly, vre_uint32 point_index );

vre_result
vre_polygon_add_contour ( vre_polygon *pPoly, vre_polygon *pContour );

void vre_polygon_swap_coords ( vre_polygon *pPoly );

vre_uint32 vre_polygon_get_num_vertices ( vre_polygon *pPoly );

vre_uint32 vre_polygon_get_num_contours ( vre_polygon *pPoly );

vre_result vre_polygon_set_num_contours ( vre_polygon *pPoly, 
                                         vre_uint32 num_contours );

vre_uint32 vre_polygon_get_contour_start ( vre_polygon *Poly, vre_uint32 contour );
vre_uint32 vre_polygon_get_contour_end ( vre_polygon *Poly, vre_uint32 contour );

void vre_polygon_reset ( vre_polygon *pPoly );

vre_result vre_polygon_copy_points ( vre_polygon *pDst, vre_polygon *pSrc );

vre_result vre_polygon_trasform_points ( vre_polygon *pDst, 
                                         vre_polygon *pSrc,
                                         vre_mat3x3 *pMat );

#endif

