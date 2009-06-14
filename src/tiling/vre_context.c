
#include "vre_context.h"
#include "vre_defs.h"
#include "vre_render.h"
#include "vre_polygon.h"
#include "vre_rtree.h"
#include "vre_list.h"

#include "vre_rectangle.h"
#include "vre_clip.h"

#include "vre_assert.h"
#include "vre_mem.h"

typedef struct vre_primitive
{
    void *pUserData;
    
    vre_mat3x3  mat;
    vre_style   *pStyle;

    vre_primitive_funcs *pFuncs;

    vre_uint32 num_primitive;
} vre_primitive;

struct vre_context
{    
    vre_rtree *pRTree;

    vre_style *pCurrentStyle;
    vre_list  *pStyleList;

    vre_uint32 num_primitives;

    vre_uint32 foreground_color;
    vre_uint32 background_color;
    vre_uint32 stroke_color;
};

static void node_destroy ( void *pData );

vre_result vre_context_create ( vre_context **ppContext )
{
    vre_result  vres;
    vre_context *pContext;

    *ppContext = 0;

    pContext = VRE_TALLOC ( vre_context );
       
    if ( pContext == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
      
    vre_memset (pContext, sizeof (vre_context), 0 );      
    
    for (;;)
    {
        vres = vre_rtree_create ( &pContext->pRTree, node_destroy );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_list_create ( &pContext->pStyleList );
        VRE_BREAK_IF_ERR ( vres );

        break;
    }

    if ( vres != VRE_ERR_OK )
    {
        vre_context_destroy ( pContext );
        return vres;
    }

    *ppContext = pContext;
    return VRE_ERR_OK;
}

static 
void node_destroy ( void *pData )
{
    vre_primitive *pPrimitive;

    if ( pData != 0 )
    {
        pPrimitive = ( vre_primitive* ) pData;
        if ( pPrimitive->pFuncs->destroyer != 0 )
        {
            pPrimitive->pFuncs->destroyer (pPrimitive->pUserData);
        }

        vre_free ( pData );
    }
}

void vre_context_destroy ( vre_context *pContext )
{
    if ( pContext != 0 )
    {   
        vre_uint32 i;

        vre_rtree_destroy ( pContext->pRTree );
        
        for ( i = vre_list_num_elems ( pContext->pStyleList ); i != 0; i-- )
        {
            vre_free ( vre_list_get_first ( pContext->pStyleList ) );
        }
        
        vre_list_destroy  ( pContext->pStyleList );
        vre_free ( pContext );
    }
}

vre_result vre_context_add_primitive ( vre_context *pContext, 
                                       void *pUserData,
                                       vre_primitive_funcs *pFuncs,
                                       vre_mat3x3 *pMat,
                                       vre_rectangle *pRect )
{
    vre_result    vres;
    vre_primitive *pPrimitive;
    
    vre_assert ( pContext );
    vre_assert ( pUserData );
    vre_assert ( pFuncs );
    vre_assert ( pMat );
    vre_assert ( pRect );

    pPrimitive = VRE_TALLOC ( vre_primitive );
    if ( pPrimitive == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }

    pPrimitive->pUserData = pUserData;
    pPrimitive->pFuncs    = pFuncs;
    pPrimitive->num_primitive = pContext->num_primitives;

    pContext->num_primitives++;

    vre_matrix_copy ( &(pPrimitive->mat), pMat );

    pPrimitive->pStyle = pContext->pCurrentStyle;
    
    vres = vre_rtree_insert ( pContext->pRTree, pRect, pPrimitive );

    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pPrimitive );
    }
    
    return vres;
}


vre_result vre_context_render ( vre_context *pContext, 
                                vre_tile    *pTile,
                                vre_mat3x3  *pMat )
{
    vre_result  vres;
    vre_rtree   *pRTree;
    vre_primitive *pPrimitive;
    vre_list    *pPrimitivesList;
    vre_list    *pSortedList;
    vre_mat3x3  mat;
    vre_mat3x3  *pTransformedMat;

    vre_rectangle rect;

    vre_uint32 prevkey;

    vre_assert ( pContext );
    vre_assert ( pTile );

    vres = vre_list_create ( &pPrimitivesList );
    VRE_RETURN_IF_ERR ( vres );

    // temp solution
    vres = vre_list_create ( &pSortedList );
    VRE_RETURN_IF_ERR ( vres );

    pRTree = pContext->pRTree;
    
    rect.x1 = (pTile->x) << 16;
    rect.y1 = (pTile->y) << 16;
    rect.x2 = (pTile->x + pTile->w) << 16;
    rect.y2 = (pTile->y + pTile->h) << 16;
   
    //
    // Get all primitives that intersect with the searching rectangle.
    //
    vres = vre_rtree_search ( pRTree, &rect, pPrimitivesList );
    if ( vres != VRE_ERR_OK )
    {
        vre_list_destroy ( pPrimitivesList );
    }
    VRE_RETURN_IF_ERR (vres);

    //
    // Sort list. This is a bottleneck that has to be addressed in the future.
    //
    pPrimitive =(vre_primitive *) vre_list_get_first ( pPrimitivesList );
    while ( pPrimitive != 0 )
    {
        vre_list_put_sorted ( pSortedList, pPrimitive, pPrimitive->num_primitive );
        pPrimitive =(vre_primitive *) vre_list_get_first ( pPrimitivesList );
    }

    pPrimitive =(vre_primitive *) vre_list_get_first ( pSortedList );

    
    while ( pPrimitive != 0 )
    {
        prevkey = pPrimitive->num_primitive;
        if ( pMat != 0 )
        {
            vre_matrix_mult ( &pPrimitive->mat, pMat, &mat );
            pTransformedMat = &mat;
        }
        else
        {
            pTransformedMat = &pPrimitive->mat;
        }

        vre_assert ( pPrimitive->pFuncs );
        vres = pPrimitive->pFuncs->renderer ( pPrimitive->pUserData, 
                                              pTransformedMat,
                                              pTile,
                                              pPrimitive->pStyle );
        VRE_BREAK_IF_ERR (vres);
        
        pPrimitive = (vre_primitive *) vre_list_get_first ( pSortedList );
        // Check that the sorting is correct.
        if ( pPrimitive != 0 )
        {
            vre_assert ( prevkey < pPrimitive->num_primitive );
        }
    };

    vre_list_destroy ( pPrimitivesList );

    vre_list_destroy ( pSortedList );
    
    return vres;
}

vre_result vre_context_set_style ( vre_context *pContext, vre_style *pStyle )
{
    vre_result vres;

    pContext->pCurrentStyle = VRE_TALLOC ( vre_style );
    if ( pContext->pCurrentStyle == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED; 
    }

    vre_memcopy8 ( pContext->pCurrentStyle, pStyle, sizeof ( vre_style ) );

    vres = vre_list_put_first( pContext->pStyleList, pContext->pCurrentStyle );

    return vres;
}


void vre_set_stroke_width ( vre_context *pContext, int width )
{
    pContext->pCurrentStyle->stroke_width = width;
}

void vre_set_foreground_color ( vre_context *pContext, int r, int g, int b )
{
    pContext->foreground_color = b | (g<<8) | (r<<16) | (0xff<<24);
}


vre_uint32 vre_context_get_fg_color ( vre_context *pContext )
{
	return pContext->foreground_color;
}

vre_style *vre_context_get_style ( vre_context *pContext )
{
	return pContext->pCurrentStyle;
}

vre_rtree  *vre_context_get_rtree ( vre_context *pContext )
{
    return pContext->pRTree;
}



