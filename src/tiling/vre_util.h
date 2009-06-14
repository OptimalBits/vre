
#include "vre_defs.h"
#include "vre_rectangle.h"

typedef enum vre_iter_status
{
    VRE_ITER_STATUS_FINISHED = 0,
    VRE_ITER_STATUS_CONTINUE = 1
} vre_iter_status;

typedef struct vre_tile_iter vre_tile_iter;

vre_result vre_create_tile_iterator ( vre_tile_iter **ppIter, 
                                      vre_size *pCanvasSize,
                                      vre_size *pTileSize );
                                      
void vre_reset_iterator ( vre_tile_iter *pIter );

vre_iter_status vre_iterate_tile (vre_tile_iter *pIter, vre_rectangle *pRect );                                      

void vre_copy_tile_to_canvas ( vre_canvas *pCanvas, vre_tile *pTile );

void vre_copy_tile_to_tile ( vre_tile *pTileDst, vre_tile *pTileSrc );
