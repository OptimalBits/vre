
#include "vre_util.h"
#include "vre_rectangle.h"
#include "vre_assert.h"
#include "vre_mem.h"

void vre_copy_tile_to_canvas ( vre_canvas *pCanvas, vre_tile *pTile )
{
    vre_uint8 *pSrc = (vre_uint8*) pTile->pData;
    vre_uint8 *pDst = (vre_uint8*) pCanvas->pData;
     
    vre_int32  i, j, k;
    vre_uint32  offset = pTile->y * pCanvas->scanline_width + 
                         pTile->x * 4;

    vre_uint32 stride = pCanvas->scanline_width - pTile->w * 4;
       
    k = 0;            
    for ( j = 0; j < pTile->h; j++ )
    {              
        for ( i = 0; i < pTile->w; i ++)
        {    
            pDst[offset] = pSrc[k];
            pDst[offset+1] = pSrc[k+1];
            pDst[offset+2] = pSrc[k+2];
            pDst[offset+3] = pSrc[k+3];
            offset +=4;
            k +=4;
        }
        offset += stride;
    }
}


void vre_copy_tile_to_tile ( vre_tile *pTileDst, vre_tile *pTileSrc )
{
    vre_uint8 *pSrc = (vre_uint8*) pTileSrc->pData;
    vre_uint8 *pDst = (vre_uint8*) pTileDst->pData;
     
    vre_int32  i, j, k;
    vre_uint32  offset = pTileSrc->y * pTileDst->scanline_width + 
                         pTileSrc->x * 4;

    vre_uint32 stride = pTileDst->scanline_width - pTileSrc->w * 4;
       
    k = 0;            
    for ( j = 0; j < pTileSrc->h; j++ )
    {              
        for ( i = 0; i < pTileSrc->w; i ++)
        {    
            pDst[offset] = pSrc[k];
            pDst[offset+1] = pSrc[k+1];
            pDst[offset+2] = pSrc[k+2];
            pDst[offset+3] = pSrc[k+3];
            offset +=4;
            k +=4;
        }
        offset += stride;
    }
}


struct vre_tile_iter
{
    vre_rectangle current_rect;
    vre_size      tile_size;
    
    vre_uint32   last_width;
    vre_uint32   last_height;
    
    vre_uint32   width; 
    vre_uint32   height;
    
    vre_uint32   x;
    vre_uint32   y;
};


/**
    Create a rectangle iterator.
    This iterator iterates through a canvas, clipping the rectangle at the
    borders of the canvas if necessary.

*/
vre_result vre_create_tile_iterator ( vre_tile_iter **ppIter, 
                                      vre_size *pCanvasSize,
                                      vre_size *pTileSize )
{ 
    vre_tile_iter   *pIter;

    
    
    pIter = (vre_tile_iter*) vre_malloc (sizeof (vre_tile_iter));
    if ( pIter == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;    
    }
    
    pIter->tile_size.w = pTileSize->w;
    pIter->tile_size.h = pTileSize->h;
    
    pIter->width = pCanvasSize->w / pTileSize->w;
    if ( pCanvasSize->w % pTileSize->w != 0 )
    {
        pIter->last_width = pCanvasSize->w - pIter->width * pTileSize->w;
        pIter->width ++;
    }
    else
    {
        pIter->last_width = pTileSize->w;
    }
    
    pIter->height = pCanvasSize->h / pTileSize->h;
    if ( pCanvasSize->h % pTileSize->h != 0 )
    {
        pIter->last_height = pCanvasSize->h - pIter->height * pTileSize->h;
        pIter->height ++;
    }
    else
    {
        pIter->last_height = pTileSize->h;
    }

    vre_reset_iterator ( pIter );
    
    *ppIter = pIter;
    
    return VRE_ERR_OK;
}


void vre_reset_iterator ( vre_tile_iter *pIter )
{
    pIter->x = 0;
    pIter->y = 0;
    
    pIter->current_rect.x = 0;
    pIter->current_rect.y = 0;
    
    if ( pIter->width > 1 )
    {
        pIter->current_rect.w = pIter->tile_size.w;
    }
    else
    {
        pIter->current_rect.w = pIter->last_width;
    }
        
    if ( pIter->height > 1 )
    {
        pIter->current_rect.h = pIter->tile_size.h;
    }
    else
    {
        pIter->current_rect.h = pIter->last_height;
    }
}



/**
    Performs one iteration.
    
*/
vre_iter_status vre_iterate_tile (vre_tile_iter *pIter, vre_rectangle *pRect )
{
    pRect->x = pIter->current_rect.x;
    pRect->y = pIter->current_rect.y;
    pRect->w = pIter->current_rect.w;
    pRect->h = pIter->current_rect.h;
    
    vre_assert ( pIter->y <= pIter->height );
    
    if ( pIter->y == pIter->height )
    {
        return VRE_ITER_STATUS_FINISHED;
    }

    pIter->x ++;
    if ( pIter->x < pIter->width )
    {
        pIter->current_rect.x += pIter->tile_size.w;
        if ( pIter->x == pIter->width - 1 )
        {
            pIter->current_rect.w = pIter->last_width;    
        }
    }
    else
    {
        pIter->x = 0;
        pIter->current_rect.x = 0;
        pIter->current_rect.w = pIter->tile_size.w;
        pIter->y ++;
        if ( pIter->y < pIter->height )
        {
            pIter->current_rect.y += pIter->tile_size.h;
            if ( pIter->y == pIter->height - 1 )
            {
                pIter->current_rect.h = pIter->last_height;
            }
        }      
    }        
    
    return VRE_ITER_STATUS_CONTINUE;
}


