/*
	Vector Rendering Context 
	(c) 2005, Synthetica Software Ltd.
*/

#include "vre_defs.h"
#include "geometry/vre_matrix.h"
#include "tiling/vre_context.h"
#include "primitives/vre_rectangle.h"



#define VRE_FIX(x, n)  ((x) << (n))



//------------------------------------------------------------------------------
vre_result vre_create_context ( vre_context **pContext );

void    vre_destroy_context ( vre_context *pContext );

//------------------------------------------------------------------------------
void    vre_start_render ( vre_context *pContext, vre_canvas *pCanvas );
void    vre_end_render   ( vre_context *pContext );

void    vre_render_tile ( vre_context *pContext, vre_tile *pTile );

//------------------------------------------------------------------------------
void    vre_set_stroke_width    ( vre_context *pContext, vre_int width );

void    vre_set_stroke_type     ( vre_context *pContext, int stroke );

void    vre_set_stroke_color ( vre_context *pContext, int r, int g, int b );

void    vre_set_background_color ( vre_context *pContext, int r, int g, int b);

void    vre_set_foreground_color ( vre_context *pContext, int r, int g, int b );

void    vre_push_style ( vre_context *pContext );

//------------------------------------------------------------------------------
void    vre_set_viewport ( vre_context *pContext, 
                           vre_rectangle *pRect );

// Maybe this functions could return a handle, so that the transform
// can be replaced afterwards if needed... (more flexibility at almost no cost)
void    vre_rotate ( vre_context *pContext, vre_fix16 angle );

void    vre_scale  ( vre_context *pContext, vre_fix16 sx, vre_fix16 sy );

void    vre_skew  ( vre_context *pContext, vre_fix16 fx, vre_fix16 fy );

void    vre_translate ( vre_context *pContext, vre_fix16 tx, vre_fix16 ty );

void    vre_transform ( vre_context *pContext, vre_mat3x3 *pMatrix );

void    vre_push_transform ( vre_context *pContext );

void    vre_pop_transform ( vre_context *pContext );

//------------------------------------------------------------------------------
void    vre_add_line ( vre_context *pContext, 
                       vre_point *pStart, 
                       vre_point *pEnd);

void    vre_add_cubic_bezier ( void );

void    vre_add_spline ( void );

void    vre_add_circle ( vre_context *pContext, vre_point *pPos, 
                         vre_ufix16 radius);

void    vre_add_ellipse ( vre_context *pContext, vre_point *pPos, 
                      vre_ufix16 r1, vre_ufix16 r2);

void    vre_add_rectangle ( vre_context *pContext, vre_point *pPoints);

vre_result vre_add_polygon ( vre_context *pContext, 
                             vre_point *pPoints,
                             vre_uint32 num_points );

vre_result vre_add_polyline ( vre_context *pContext, 
                              vre_point *pPoints, 
                              vre_uint32 num_points );

