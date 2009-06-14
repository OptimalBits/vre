
#ifndef VRE_CONTEXT_H
#define VRE_CONTEXT_H


#include "vre_defs.h"
#include "vre_matrix.h"
#include "vre_rectangle.h"
#include "vre_rtree.h"
#include "vre_style.h"

typedef struct vre_context vre_context;

//
//
//-----------------------------------------------------------------------------
typedef vre_result (*VRE_PRIMITIVE_RENDER) ( void *pUserData, 
                                             vre_mat3x3 *pMat,
                                             vre_tile   *pTile,
                                             vre_style  *pStyle);

typedef void (*VRE_PRIMITIVE_DESTROY)      ( void *pUserData );

typedef struct vre_primitive_funcs
{
    VRE_PRIMITIVE_RENDER  renderer;
    VRE_PRIMITIVE_DESTROY destroyer;
} vre_primitive_funcs;

//-----------------------------------------------------------------------------

vre_result vre_context_create ( vre_context **ppContext );
void vre_context_destroy ( vre_context *pContext );

vre_result vre_context_add_primitive ( vre_context *pContext, 
                                       void *pUserData,
                                       vre_primitive_funcs *pFuncs,
                                       vre_mat3x3 *pMat,
                                       vre_rectangle *pRect );

vre_result vre_context_render ( vre_context *pContext, 
                                vre_tile    *pTile,
                                vre_mat3x3  *pMat );
// Setters
vre_result vre_context_set_style ( vre_context *pContext, vre_style *pStyle );

// Getters
vre_canvas *vre_context_get_canvas   ( vre_context *pContext );
vre_uint32  vre_context_get_fg_color ( vre_context *pContext );
vre_style  *vre_context_get_style    ( vre_context *pContext );
vre_rtree  *vre_context_get_rtree    ( vre_context *pContext );

#endif
