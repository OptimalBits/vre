/* TTF Loader test */

#include <SDL.h>

#include <stdio.h>

#include "ConvertUTF.h"

#include "vre.h"

#include "vre_ttf.h"
#include "vre_polygon.h"
#include "vre_glyph.h"
#include "vre_bezier.h"
#include "vre_assert.h"
#include "vre_font.h"
#include "vre_text.h"
#include "vre_math.h"
#include "vre_util.h"
#include "vre_mem.h"
#include "vre_rtree.h"


#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

#define VRE_CHAR vre_uint16

SDL_Event event;

void drawString (char *string, vre_style style,
                 vre_tile tile, vre_ttf *pTtf, 
                 vre_uint32 x,                  
                 vre_uint32 y);

vre_result vre_render_( vre_canvas *pCanvas,
                        vre_text *pText );

vre_result render_text ( vre_text *pText, 
                         vre_context *pContext,
                         vre_uint32 upem,
                         vre_tile *pTile, 
                         vre_uint32 x,
                         vre_uint32 y,
                         vre_uint32 scale,
                         vre_uint32 angle );


void fancy_demo ( SDL_Surface *screen );

int main (int argc, char *argv[])
{
    int k, i;
   
    vre_context *pContext;
    vre_font *pFont;
    vre_text *pText;
    vre_ttf *pTtf;
    
    vre_font_attr   *pAttr;
    
    vre_uint32 scale = 185;
    vre_int32 angle = 0;//VRE_PI_4_28>>12;
    
    vre_mat3x3 mat;
    
    vre_iostream *pStream;
    vre_result vres;
    vre_polygon *pPoly;
    vre_glyph *pGlyph;
    
    vre_polygon *pPolyOut;
    
    vre_bezier *pBezier;
	vre_bezier *pBezier2;
    
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
    {450<<16, 500<<16},
    {550<<16, 250<<16}   
    };

    vre_style   style;
    vre_tile    tile;
    vre_canvas  canvas;
    
    VRE_CHAR *strsmall = "This is a text showing how good quality is possible to achieve with an State Of The Art polygon renderer!. Is Hinting really needed?";
  //  VRE_CHAR *string = "&The Quick Brown Fox Jumps Over The Lazy Dog";
    VRE_CHAR *string = "Toyota";
    
    
    VRE_CHAR *string1 ="abcdefghijklmnopqrstuvwxyz";
    VRE_CHAR *string2 ="ABCDEFGHIJKLM";
    VRE_CHAR *string3 ="sgbrjpoi0cq";
    VRE_CHAR *string4 ="Test";
    VRE_CHAR *string5 ="Composites:ŒŠš–";
    VRE_CHAR strutf16[4] = {0x6147, 0x6148,0x6149, 0x6146};
    SDL_Surface *screen;
    
    int done = 0;
    
    vre_uint32 peak_mem;

    // 
    if( SDL_Init(SDL_INIT_VIDEO) <0 )
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 32, 
                               SDL_HWSURFACE|SDL_DOUBLEBUF);

    if ( screen == NULL )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

   
      
    fancy_demo ( screen );
  /*  
    tile.x = 0;
    tile.y = 0;
    tile.w = SCREEN_WIDTH;
    tile.h = SCREEN_HEIGHT;
    tile.scanline_width = screen->pitch;
    tile.pData = screen->pixels;
    
//  vres = vre_iostream_open_file ( &pStream, "tahoma.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "crazk.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "Vera.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "Abduction.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "Amazone BT.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "Aspastic.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "Baby Blocks.ttf" );
//  vres = vre_iostream_open_file ( &pStream, "dotum.ttf" );
  /*
    vres = vre_iostream_open_file ( &pStream, "uming.ttf" );
    
    vre_matrix_identity(&mat);
    
    vres = vre_create_font(&pFont);
    vres = vre_create_ttf ( &pTtf );

    peak_mem = vre_mem_peak();
    
    vres = vre_load_ttf ( pTtf, pStream );
    
    peak_mem = vre_mem_peak();
    
    vre_ttf_bind ( pTtf, pFont );
    
    pAttr = vre_font_get_attributes(pFont);
    
    pAttr->direction = VRE_FONT_DIRECTION_LR;
    
    vres = vre_text_create(&pText, pFont, &style, &mat, VRE_CHAR_UTF16);
    
   // vres = vre_text_add_string(pText, string, strlen(string));
    vres = vre_text_add_string(pText, strutf16, 4);
    
    peak_mem = vre_mem_peak();
    printf("peak mem: %d", peak_mem);

    // Draw bezier curves
    /*
    vre_polygon_create3 ( &pPolyOut, 1000, 1 );	

    pPolyOut->pContoursStart[0] = 0;

    vre_create_bezier ( &pBezier, 3 );
    vre_create_bezier ( &pBezier2, 4 );

    vre_bezier_set_points ( pBezier, bezier, 3 );
    vre_bezier_set_points ( pBezier2, bezier2, 4 );

    // vre_bezier_flatten ( pBezier, pPolyOut, 100, 0, 0 );
    // pPolyOut->pContoursEnd[0] = pPolyOut->num_vertices - 1;
    // vre_draw_polygon_concave2 ( pPolyOut, &tile, &style );  

//    vre_bezier_flatten ( pBezier2, pPolyOut, 50, 0 );
 //   pPolyOut->pContoursEnd[0] = pPolyOut->num_vertices - 1;
    // vre_draw_polygon_concave_aa ( pPolyOut, &tile, &style ); 
    */
    
    /*
    SDL_Flip(screen);
    
    
     SDL_LockSurface(screen);
       
     vre_memset(tile.pData, tile.scanline_width*tile.h, 0xff);
     
     SDL_UnlockSurface(screen);
     
     canvas.pData = tile.pData;
     canvas.width  = tile.w;
     canvas.height = tile.h;
     canvas.scanline_width = tile.scanline_width;
     
    while(done == 0)
    {
        while ( SDL_PollEvent(&event) )
        {
            if ( event.type == SDL_QUIT )  
            {  
                done = 1;
            }
        }
     
        SDL_LockSurface(screen);
        vre_memset(tile.pData, tile.scanline_width*tile.h, 0xff);
        
        style.fg_color = 0xff404040;
        (void) render_text ( pText, pAttr->upem, &tile, 20, 160, scale, angle );
        
        style.fg_color = 0x8000ff00;
        (void) render_text ( pText, pAttr->upem, &tile, 16, 155, scale, angle );
        SDL_UnlockSurface(screen);
                
        //scale--;
        angle-=100;
        
        SDL_Flip(screen);
       
        if ( angle < -50000 ) done = 1;
        
        //printf ("Peak Memory Usage: %d\n", vre_mem_peak());
    }
 */
    SDL_Quit();

    return 0;
}

//
// Render in a tile based manner.
//
vre_result vre_render_iterative ( vre_context *pContext,
                                  vre_tile *pTile )
{
    vre_result vres = VRE_ERR_OK;
   
    vre_tile_iter   *pIter;
    vre_size        canvas_size;
    vre_size        tile_size;
    vre_tile        tile;
    vre_rectangle   rect;
    vre_mat3x3      tmp_mat;
    vre_mat3x3      *pMat;
    vre_int32 cont = 0;
    
    pMat = &tmp_mat;
    
    canvas_size.w = pTile->w;
    canvas_size.h = pTile->h;
    
    tile_size.w = 8;
    tile_size.h = 8;
    
    tile.w = tile_size.w;
    tile.h = tile_size.h;
    
    tile.pData = vre_malloc( tile.w * tile.h * 4 );
    if ( tile.pData == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    tile.scanline_width = tile.w * 4;
    
    vres = vre_create_tile_iterator( &pIter, &canvas_size, &tile_size );
    
    vre_matrix_identity ( pMat );
    
    while ( vre_iterate_tile(pIter, &rect ) == VRE_ITER_STATUS_CONTINUE )
    {
        // Translate -rect
        tile.x = rect.x;
        tile.y = rect.y;
        tile.w = rect.w;
        tile.h = rect.h;
        
        vre_matrix_translate( pMat, -(rect.x<<16), -(rect.y<<16) );
        
        // clean tile
        vre_memset(tile.pData, tile.scanline_width*tile.h, 0xff);
        
        // Render to tile
        //vre_text_draw(pText, &tile, pMat);
        vre_context_render (pContext, &tile, pMat);

        // Copy tile to canvas
        vre_copy_tile_to_tile( pTile, &tile );
        
        // Back to origin
        vre_matrix_translate( pMat, (rect.x<<16), (rect.y<<16) );
        
        cont++;
        //if ( cont == 2 ) break;
    };
    
    vre_free( pIter);
    vre_free ( tile.pData );
    
    return VRE_ERR_OK;
}

vre_result render_text ( vre_text *pText, 
                         vre_context *pContext,
                         vre_uint32 upem,
                         vre_tile *pTile, 
                         vre_uint32 x,
                         vre_uint32 y,
                         vre_uint32 scale,
                         vre_uint32 angle )
{
    vre_mat3x3  mat;
    vre_int32   norm_scale;
    
    norm_scale = (65536 / upem) * 65536;
    
    vre_matrix_identity (&mat);
    vre_matrix_rotate( &mat, angle);
    
    vre_matrix_scale ( &mat, scale<<16, scale<<16 );
    
    vre_matrix_scale ( &mat,  norm_scale, -norm_scale );//;
    vre_matrix_translate ( &mat, x<<16, y<<16 );
        
    vre_text_set_cursor (pText, 0,0);
    
    vre_text_prepare(pText, &mat, pContext);
    
    return VRE_ERR_OK;
}


void fancy_demo ( SDL_Surface *screen )
{
    vre_result vres;
    int done = 0;

    vre_context *pContext;
    
    vre_style style;
    vre_tile  tile;
    
    vre_font *pFont;
    vre_text *pText;
    vre_ttf *pTtf;
    
    vre_font *pFont2;
    vre_text *pText2;
    vre_ttf *pTtf2;
    
    vre_font *pFont3;
    vre_text *pText3;
    vre_ttf *pTtf3;
    
    vre_font_attr *pAttr;
    vre_iostream *pStream;
    vre_iostream *pStream2;
    vre_iostream *pStream3;
    
    vre_uint32 scale = 185;
    vre_int32 angle = 0;//VRE_PI_4_28>>12;
    vre_int32 transx = 100;
        
    vre_uint32 scaleUp = 0;
    vre_uint32 moveRight = 1;
        
    vre_mat3x3 mat;
    
    VRE_CHAR strutf16[4] = {0x6147, 0x6148,0x6149, 0x6146};
    char *scaladoStr ="Scalado";
    //char *string3 = "This is a very interesting test!";
    char *string3 = "abcdefghij";

    vre_matrix_identity(&mat);
    
    // Load Fonts and create texts

    vres = vre_iostream_open_file ( &pStream, "uming.ttf" );  
    
    vres = vre_create_font(&pFont);
    vres = vre_create_ttf ( &pTtf );    
    vres = vre_load_ttf ( pTtf, pStream );
    
    vre_ttf_bind ( pTtf, pFont );
    
    pAttr = vre_font_get_attributes(pFont);
    
    pAttr->direction = VRE_FONT_DIRECTION_LR;
    
    vres = vre_text_create(&pText, pFont, &style, &mat, VRE_CHAR_UTF16);
    vres = vre_text_add_string(pText, strutf16, 4);

    //
    vres = vre_iostream_open_file ( &pStream2, "Amazone BT.ttf" );  
    vres = vre_create_font(&pFont2);
    vres = vre_create_ttf ( &pTtf2 );    
    
    vres = vre_load_ttf ( pTtf2, pStream2 );
    
    vre_ttf_bind ( pTtf2, pFont2 );

    pAttr = vre_font_get_attributes(pFont2);
    
    pAttr->direction = VRE_FONT_DIRECTION_LR;
    vres = vre_text_create(&pText2, pFont2, &style, &mat, VRE_CHAR_ASCII);

    vres = vre_text_add_string(pText2, scaladoStr, 7);
   
    //
    vres = vre_iostream_open_file ( &pStream3, "Tahoma.ttf" );  
    vres = vre_create_font(&pFont3);
    vres = vre_create_ttf ( &pTtf3 );    
        
    vres = vre_load_ttf ( pTtf3, pStream3 );
    
    vre_ttf_bind ( pTtf3, pFont3 );
    
    pAttr = vre_font_get_attributes(pFont3);
    
    pAttr->direction = VRE_FONT_DIRECTION_LR;
    vres = vre_text_create(&pText3, pFont3, &style, &mat, VRE_CHAR_ASCII);
    
    vres = vre_text_add_string(pText3, string3, strlen(string3));
    
    // Init buffers
    tile.x = 0;
    tile.y = 0;
    tile.w = SCREEN_WIDTH;
    tile.h = SCREEN_HEIGHT;
    tile.scanline_width = screen->pitch;
    tile.pData = screen->pixels;
    
    // Perform loop animation for every animated thing.
    while(done == 0)
    {
        while ( SDL_PollEvent(&event) )
        {
            if ( event.type == SDL_QUIT )  
            {  
                done = 1;
            }
        }
        
        vre_context_create ( &pContext );
     /*
        style.fg_color = 0xff404040;
        vre_context_set_style ( pContext, &style );
        (void) render_text ( pText, pContext, pAttr->upem, &tile, 20, 160, 128, angle );
      
        style.fg_color = 0xff00ff00;
        vre_context_set_style ( pContext, &style );
        (void) render_text ( pText, pContext, pAttr->upem, &tile, 16, 155, 128, angle );
        
        style.fg_color = 0x402010ff;
        vre_context_set_style ( pContext, &style );
        (void) render_text ( pText2, pContext, pAttr->upem, &tile, 0, 280, scale, 0 );
     
        style.fg_color = 0x80f0aa00;
        vre_context_set_style ( pContext, &style );
        (void) render_text ( pText2, pContext,  pAttr->upem, &tile, 0, 280, 100, 0 );
     */ 
        style.fg_color = 0x80ff4080;
        vre_context_set_style ( pContext, &style );
        (void) render_text ( pText3, pContext, pAttr->upem, &tile, transx, 350, 48, 0 );
       
        SDL_LockSurface(screen);
        vre_memset(tile.pData, tile.scanline_width*tile.h, 0xaa);

        //vre_context_render (pContext, &tile, 0);
        vre_render_iterative ( pContext, &tile );

        SDL_UnlockSurface(screen);

        vre_context_destroy (pContext);
        
        if ( moveRight == 0 )
        {
            transx --;
            if ( transx < -50 )
            {
                moveRight = 1;
            }
        }
        else
        {
            transx ++;
            if ( transx > 500 )
            {
                moveRight = 0;
            }
        }
        
        angle-=100;
        
        if ( scaleUp == 0 )
        {
            scale--;
            if ( scale < 100 )
            {
                scaleUp = 1;
            }
        }
        else
        {
            scale ++;
            if ( scale > 400 ) 
            {
                scaleUp = 0;
            }
        }
        
        SDL_Flip(screen);
        
        if ( angle < -190000 ) 
        {
        //    done = 1;
        }

//        printf ("Peak Memory Usage: %d\n", vre_mem_peak());        
    }
}
