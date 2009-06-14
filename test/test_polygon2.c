

#include <SDL/SDL.h>
#include "vre.h"
#include "geometry/vre_clip.h"
#include "primitives/vre_polygon.h"
#include "tiling/vre_util.h"

#define TILE_WIDTH  64
#define TILE_HEIGHT 64


SDL_Event event;

int main (int argc, char *argv[])
{
    int done = 0;

    SDL_Surface *screen;

    vre_result  vres;
    
    vre_uint32 *pTile;
    vre_tile_iter *pIter;
    
    vre_size    canvas_size;
    vre_size    tile_size;
    vre_rectangle    tile_rect;

    vre_canvas  canvas;
    vre_tile    tile;
    vre_style   style;

    vre_context *pContext;	                 

    vre_point   triangle[3] = 
    {
        {300<<16, 100<<16},
        {150<<16, 120<<16},
        {400<<16, 140<<16}
    };
    
    vre_point   box[4] =
    {
        {250<<16, 100<<16},
        {350<<16, 100<<16},
        {350<<16, 210<<16},
        {250<<16, 210<<16},    
    };

    vre_point   points[6] = 
    {
        {850<<16, 75<<16},
        {958<<16, 137<<16},
        {958<<16, 262<<16},
        {850<<16, 325<<16},        
        {742<<16, 262<<16},
        {742<<16, 137<<16}    
    };
    
    
      vre_point   points2[6] = 
    {
        {0<<16, 0<<16},
        {58<<16, 37<<16},
        {98<<16, 262<<16},
        {150<<16, 325<<16},        
        {242<<16, 262<<16},
        {342<<16, 137<<16}    
    };

    vre_point   star[10] =
    {
        {350<<16,75<<16},
        {379<<16,161<<16},
        {469<<16,161<<16},
        {397<<16,215<<16},
        {423<<16,301<<16}, 
        {350<<16,250<<16}, 
        {277<<16,301<<16}, 
        {303<<16,215<<16}, 
        {231<<16,161<<16}, 
        {321<<16,161<<16}
    };

    vre_result result;

    result = vre_create_context (&pContext);
    if ( result != VRE_ERR_OK )
    {
        printf ("Error creating vre engine");
        return -1;
    }

    // 
    if( SDL_Init(SDL_INIT_VIDEO) <0 )
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);

    if ( screen == NULL )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    SDL_LockSurface(screen); 

    // Add Polygons to be rendered
    vre_set_stroke_width (pContext, 5);
    vre_set_foreground_color (pContext, 100, 50, 50);
    
    //vres = vre_add_polyline ( pContext, star, 10 );
    
  //  vre_translate ( pContext, VRE_FIX (-100,16), VRE_FIX (-50,16) );
    //vre_rotate ( pContext, VRE_FIX (10, 16) );
    vres = vre_add_polygon  ( pContext, star, 10 );
    vres = vre_add_polygon  ( pContext, box, 4 );
    vres = vre_add_polygon  ( pContext, triangle, 3 );

    // Render Polygons    
    
    canvas.width = 640;
    canvas.height = 480;
    canvas.scanline_width = screen->pitch;
    canvas.pData = screen->pixels;
       
    vre_start_render (pContext, &canvas);
        
    canvas_size.w = 640;
    canvas_size.h = 480;
    tile_size.w = TILE_WIDTH;
    tile_size.h = TILE_HEIGHT;
    
    pTile = (vre_uint32*) vre_malloc ( TILE_WIDTH * TILE_HEIGHT * 4 );
    tile.pData = pTile;
    tile.scanline_width = TILE_WIDTH * 4;
    
    vre_create_tile_iterator ( &pIter, &canvas_size, &tile_size );
        
    while ( vre_iterate_tile ( pIter, &tile_rect ) == VRE_ITER_STATUS_CONTINUE )
    {
         tile.x = tile_rect.x;
         tile.y = tile_rect.y;
         tile.w = tile_rect.w;
         tile.h = tile_rect.h;
         
         vre_render_tile (pContext, &tile );          
         
         vre_copy_tile_to_canvas ( &canvas, &tile);
    }        	     
 
    SDL_UnlockSurface(screen);
    

    SDL_Flip(screen);    

    while(done == 0)
    {
        while ( SDL_PollEvent(&event) )
        {
            if ( event.type == SDL_QUIT )  
            {  
                done = 1;  
            }
        }
    }
    
    SDL_Quit();
    return 0;
}
