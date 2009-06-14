

#include <SDL/SDL.h>
#include "vre.h"
#include "geometry/vre_clip.h"
#include "primitives/vre_polygon.h"


SDL_Event event;

int main (int argc, char *argv[])
{
    int done = 0;

    SDL_Surface *screen;

    vre_canvas  canvas;
    vre_tile    tile;
    vre_style   style;

    vre_context *pContext;
	
	vre_polygon *pPoly;
	vre_polygon *pPolyOut;
	vre_polygon *pPolyTmp;
	
	vre_rectangle rect;
                 

    vre_point   triangle[3] = 
    {
        {300<<16, 100<<16},
        {150<<16, 120<<16},
        {400<<16, 140<<16}
    };
    
    vre_point   box[4] =
    {
        {250<<8, 100<<8},
        {350<<8, 100<<8},
        {350<<8, 210<<8},
        {250<<8, 210<<8},    
    };

    vre_point   points[6] = 
    {
        {850<<15, 75<<16},
        {958<<15, 137<<16},
        {958<<15, 262<<16},
        {850<<15, 325<<16},        
        {742<<15, 262<<16},
        {742<<15, 137<<16}    
    };

    vre_point   star[10] =
    {
        {350<<8,75<<8},
        {379<<8,161<<8},
        {469<<8,161<<8},
        {397<<8,215<<8},
        {423<<8,301<<8}, 
        {350<<8,250<<8}, 
        {277<<8,301<<8}, 
        {303<<8,215<<8}, 
        {231<<8,161<<8}, 
        {321<<8,161<<8}
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

    // Draw Polygons 
    canvas.width = 640;
    canvas.height = 480;
    canvas.scanline_width = screen->pitch;
    canvas.pData = screen->pixels;
    
    tile.x = 0;
    tile.y = 0;
    tile.w = 640;
    tile.h = 480;
    tile.scanline_width = screen->pitch;
    tile.pData = screen->pixels;
    
    vre_start_render (pContext, &canvas);

// vre_polygon_convex (pContext, points, 6);
// vre_polygon_convex (pContext, star, 10);

    vre_set_stroke_width (pContext, 5);
	
	vre_polygon_create ( &pPoly, star, 10 );
	
	vre_polygon_create2 ( &pPolyOut, 10*7 );
	vre_polygon_create2 ( &pPolyTmp, 10*7 );
		
	rect.x = 256<<8;
	rect.y = 200<<8;
	rect.w = 256<<8;
	rect.h = 200<<8;
	
	vre_clip_poly_rectangle ( pPoly,
     	                  &rect,
    	                  pPolyOut,
        	              pPolyTmp );
               
    vre_set_foreground_color (pContext, 20, 150, 50);
    style.fg_color = 0xfff00fff;
    
   // vre_draw_polygon_concave (&tile, &style, star, 10);
    
    // vre_draw_polygon_concave (pContext, box, 4);
    vre_set_foreground_color (pContext, 100, 50, 50);
    vre_draw_polygon_concave ( &tile, &style, 
                               pPolyOut->pVertices, 
                               pPolyOut->num_vertices);                                                              

   // vre_polyline (pContext, pPolyOut->pVertices,
   //               pPolyOut->num_vertices); 
                  
    vre_polyline (pContext, triangle, 3);

/*
    vre_line (pContext, &star[0], &star[1]);
    vre_line (pContext, &star[1], &star[2]);
    vre_line (pContext, &star[2], &star[3]);
    vre_line (pContext, &star[3], &star[4]);
    vre_line (pContext, &star[4], &star[5]);  
    vre_line (pContext, &star[5], &star[6]);  
    vre_line (pContext, &star[6], &star[7]); 
    vre_line (pContext, &star[7], &star[8]); 
    vre_line (pContext, &star[8], &star[9]); 
    vre_line (pContext, &star[9], &star[0]);  
*/

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
