

#include <SDL/SDL.h>
#include "vre.h"
#include "primitives/vre_bezier.h"


SDL_Event event;

int main( int argc, char *argv[] )
{
    int done = 0;
    
    vre_result result;

    SDL_Surface *screen;

    vre_canvas  canvas;
    vre_tile    tile;
    vre_style   style;

    vre_context *pContext;
	
	vre_polygon *pPoly;
	vre_polygon *pPolyOut;
	vre_polygon *pPolyTmp;
	
	vre_rectangle rect;
	
	vre_bezier *pBezier;
	vre_bezier *pBezier2;
	
	vre_uint32 i;
                 
    vre_point   bezier[3] = 
    {
        {50<<16, 200<<16},
        {300<<16, 10<<16},
        {450<<16, 380<<16}
    };
    
    vre_point   bezier2[4] =
    {
        {150<<16, 100<<16},
        {300<<16, 0<<16},
        {450<<16, 400<<16},
        {10<<16, 50<<16}   
    };

    vre_point   points[6] = 
    {
        {850<<8, 75<<8},
        {958<<8, 137<<8},
        {958<<8, 262<<8},
        {850<<8, 325<<8},        
        {742<<8, 262<<8},
        {742<<8, 137<<8}    
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


    result = vre_create_context (&pContext);
    if ( result != VRE_ERR_OK )
    {
        printf ("Error creating vre engine");
        return -1;
    }

    result = vre_create_bezier ( &pBezier, 3 );
    result = vre_create_bezier ( &pBezier2, 4 );
    
    vre_bezier_set_points ( pBezier, bezier, 3 );
    vre_bezier_set_points ( pBezier2, bezier2, 4 );


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
	
	vre_polygon_create2 ( &pPolyOut, 100 );	
		              
    vre_set_foreground_color (pContext, 20, 150, 50);  
    vre_set_stroke_width (pContext, 2);

    vre_bezier_flatten ( pBezier, pPolyOut, 300, 0, 0 );   
    vre_polyline (pContext, pPolyOut->pVertices, pPolyOut->num_vertices);
    
    vre_bezier_flatten ( pBezier2, pPolyOut, 1000, 0, 0 );
    vre_polyline (pContext, pPolyOut->pVertices, pPolyOut->num_vertices);
                            

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
