/*
 *  vre_box.c
 *  vre
 *
 *  Created by Manuel Astudillo on 2008-06-21.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_box.h"
#include "vre_path.h"
#include "vre_mem.h"

struct vre_box 
{
    vre_path *pPath;
    vre_rectangle rect;
    
};

vre_result vre_box_create ( vre_box **ppBox,
                            vre_style *pStyle,
                            vre_rectangle *pRect )
{
    vre_result vres = VRE_ERR_OK;
    vre_box *pBox;
    
    pBox= VRE_TALLOC ( vre_box );
    if ( pBox == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED; 
    }
    
    vre_memset( pBox, sizeof ( vre_box ), 0);
    
    pBox->rect = *pRect;
    
    for (;;)
    {
        vres = vre_path_create(&pBox->pPath, pStyle );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_path_add_move_to( pBox->pPath, pBox->rect.x, pBox->rect.y );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_path_add_line( pBox->pPath, 
                                   VRE_FALSE, 
                                   pBox->rect.x + pBox->rect.w, 
                                   pBox->rect.y );
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_path_add_line( pBox->pPath, 
                                  VRE_FALSE, 
                                  pBox->rect.x + pBox->rect.w, 
                                  pBox->rect.y + pBox->rect.h);
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_path_add_line( pBox->pPath, 
                                 VRE_FALSE, 
                                 pBox->rect.x, 
                                 pBox->rect.y + pBox->rect.h);
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_path_close( pBox->pPath );
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_box_destroy( pBox );
        pBox = 0;
    }
    
    *ppBox = pBox;
    
    return vres;
}

void vre_box_destroy ( vre_box *pBox )
{
    if ( pBox != 0 )
    {
        vre_path_destroy( pBox->pPath );
        vre_free ( pBox );
    }
}

vre_result vre_box_prepare ( vre_box *pBox,
                             vre_mat3x3 *pMat,
                             vre_context *pContext )
{
    return VRE_ERR_OK;
}

/**
 Renders the box into the specified tile object.
 
 @param 
 
 */
vre_result vre_box_render ( vre_box *pBox, vre_render *pRender, 
                            vre_mat3x3 *pMat,vre_tile *pTile )
{
    return vre_path_render( pBox->pPath, pRender, pMat,pTile );
}


