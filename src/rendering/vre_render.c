/*
 *  vre_render.c
 *  vre_test
 *
 *  Created by Manuel  Astudillo on 2006-04-20.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_render.h"
#include "vre_polygon.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"



typedef struct
{
    vre_fix16   x;  ///< current x coordinate of intersection with y scanline
    vre_fix16   dx; ///< delta x, change of x with respect to y
    vre_uint    i;  ///< edge number
} vre_poly_edge;


typedef struct
{
    vre_poly_edge   *pList;
    vre_int32 num_edges;
} vre_active_list;


struct vre_render
{
    vre_uint32      max_num_vertices;
    vre_active_list *pActive;
    vre_int16       *pIndexes;
    vre_uint8       *pAlpha;
};

// Premultiplied alpha compositing
// B over A
// C = B + A - bA (where A, B and C are premultiplied alpha colors).
//

static vre_uint32 VRE_ALPHA_BLEND ( vre_uint32 A, vre_uint32 B )
{    
    vre_uint32 A1, A2, B1, B2;
    
    vre_uint32 beta;
    
    beta = (B >> 24) + 1;
    
    A1 = A & 0x00ff00ff;
    A2 = (A >> 8) & 0x00ff00ff;
    
    B1 = B & 0x00ff00ff;
    B2 = (B >> 8) & 0x00ff00ff;
    
    B1 += A1 - ((beta*A1) >> 8);
    B2 += A2 - ((beta*A2) >> 8); 
    
    return ((B2 & 0x00ff00ff) << 8) | ( B1 & 0x00ff00ff);
}


// ----------------------------------------------------------------------------
/**
Scan converts a convex polygon using the style parameters
 specified in vre_context
 
 @param pContext instance to a valid rendering engine.
 @param pPoints list of points ordered clockwise and 
 sortered according to y coordinates.
 @param num_points number of points in the polygon.
 @param first_point point with smallest y value.
 */
// ----------------------------------------------------------------------------
/*
void vre_polygon_convex (vre_context *pContext, vre_point *pPoints, 
                         vre_uint num_points, vre_uint first_point)
{
    vre_uint32 y;
    vre_point *pLeft;
    vre_point *pRight;
    vre_point *pNextLeft;
    vre_point *pNextRight;
    
    vre_int32 slope;
    
    vre_int32 left_inc, right_inc;
    vre_int32 x_left, x_right;
    
    // Indexes to current vertices
    vre_uint left, right;
	
	vre_canvas *pCanvas = vre_context_get_canvas ( pContext );
    
    vre_assert ( num_points >= 3 );
    
    // Setup initial values
    pLeft = &pPoints[first_point];
    pRight = &pPoints[first_point];
    
    left = (first_point - 1) % num_points;
    right = (first_point + 1) % num_points;
    
    pNextLeft = &pPoints[left];
    pNextRight = &pPoints[right];
    
    slope = (pNextLeft->y - pLeft->y);
    if ( slope != 0 )
    {
        left_inc  = ((pNextLeft->x - pLeft->x)<<8) / slope;
    }
    
    slope = (pNextRight->y - pRight->y);
    if ( slope != 0 )
    {
        right_inc = ((pNextRight->x - pRight->x)<<8) / slope;
    }
    
    // Iterate scanlines
    y = pLeft->y >> 5; // low precision to 3 bits.
    x_left = pLeft->x;
    x_right = pRight->x + 1; // add one to get correct first pixel.
    
    while (1)
    {       
        if (y == (pNextLeft->y>>5) )
        {
            left = (left - 1) % num_points;
            
            pLeft = pNextLeft;
            pNextLeft = &pPoints[left];
            slope =     (pNextLeft->y - pLeft->y);
            if (slope != 0 )
            {
                left_inc = ((pNextLeft->x - pLeft->x)<<8) / slope;
            }  
            x_left = pLeft->x;                   
        }
        
        if (y == (pNextRight->y>>5) )
        {
            right = (right + 1) % num_points;
            
            pRight = pNextRight;
            pNextRight = &pPoints[right];
            
            slope =      (pNextRight->y - pRight->y);
            if (slope != 0)
            {
                right_inc = ((pNextRight->x - pRight->x)<<8) / slope;
            }
            x_right = pRight->x; 
        }
        
        if (y > (pNextLeft->y>>5) || y > (pNextRight->y>>5) )
        {
            vre_render_scanline (pContext, pLeft, pRight, y >> 3);
            break;
        }
        
        if (( y & 0x7 ) == 0x07)
        {           
            vre_uint    offset = ( ((y >> 3)*(pCanvas->scanline_width))>>2) + (x_left >> 8);
            vre_uint    i;      
            
            for ( i = x_left >> 8; i < ((x_right+0xff) >> 8); i++ )
            {
                pCanvas->pData[offset] = 0xffffffff;
                offset++;
            }
            
            // three more bits are required to increment at the same time as y
            // (for antialias).
            x_left += left_inc;
            x_right+= right_inc;
        }
        
        y++;
    }
}
*/
// ----------------------------------------------------------------------------
/**
Scan converts a concave polygon using the style parameters
 specified in vre_context.
 
 @param pContext instance to a valid rendering engine.
 @param pPoints array with polygon vertices sorted in clockwise or
 counter clockwise order.
 */
// ----------------------------------------------------------------------------


static vre_result vre_create_active_list (vre_active_list **ppActive, 
                                          vre_uint num_edges)
{
    vre_active_list *pActive;
    
    pActive = (vre_active_list*) vre_malloc (sizeof (vre_active_list));
    
    if (pActive == 0) 
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pActive->pList = 
        (vre_poly_edge*) vre_malloc (sizeof(vre_poly_edge) * num_edges);
    
    if (pActive->pList == 0)
    {
        vre_free (pActive);
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pActive->num_edges = 0;
    
    *ppActive = pActive;
    
    return VRE_ERR_OK;
}

static void vre_active_list_ins ( vre_active_list *pActive, 
                                  vre_polygon *pPoly, 
                                  vre_int i, vre_fix8 y )
{
    vre_int j, p, q;
    vre_poly_edge *pList = pActive->pList;
    vre_int nact = pActive->num_edges;
    vre_point   *pPoints = vre_polygon_get_point ( pPoly, 0);
    
    vre_uint32 c;
    
    vre_int start_point, end_point;
    
    vre_int32 inc;
    
    c = vre_polygon_get_contour(pPoly, i);
    
    start_point = vre_polygon_get_contour_start (pPoly, c); 
    
    end_point = vre_polygon_get_contour_end (pPoly, c);
    
    j = ( i < end_point) ? i + 1 : start_point;
    
    if ( pPoints[i].y < pPoints[j].y ) 
    {
        p = i;
        q = j;
    }
    else               
    {
        p = j;
        q = i;
    }
    
    pList[nact].dx = (((vre_int64) (pPoints[q].x - pPoints[p].x)) << 16) / 
        (pPoints[q].y - pPoints[p].y);
    
    inc = ((((vre_int64) pList[nact].dx) * (y - pPoints[p].y)) + 32768) >> 16;
    pList[nact].x = pPoints[p].x + inc; 
    
    pList[nact].i = i;
    
    pActive->num_edges++;
}




static void vre_active_list_ins2 ( vre_active_list *pActive, 
                                   vre_polygon *pPoly, 
                                   vre_int i, vre_fix8 y)
{
    vre_int j, p, q;
    vre_poly_edge *pList = pActive->pList;
    vre_int nact = pActive->num_edges;
    vre_point   *pPoints = vre_polygon_get_point ( pPoly, 0);
    
    vre_uint32 c;
    
    vre_int start_point, end_point;
    
    vre_int32 inc;

    vre_int32 diff_y;
    
    // TODO: Pass c as a parameter.
    c = vre_polygon_get_contour(pPoly, i);
    
    start_point = vre_polygon_get_contour_start (pPoly, c);     
    end_point = vre_polygon_get_contour_end (pPoly, c);
    
    j = ( i < end_point) ? i + 1 : start_point;
    
    if ( pPoints[i].y < pPoints[j].y ) 
    {
        p = i;
        q = j;
    }
    else               
    {
        p = j;
        q = i;
    }
    
    diff_y = (pPoints[q].y - pPoints[p].y);

    pList[nact].dx = ((((vre_int64) (pPoints[q].x - pPoints[p].x)<<16) / diff_y))&0xffffffff;

    inc = ((((vre_int64) pList[nact].dx) * (y - pPoints[p].y)) + 32768) >> 16;
    pList[nact].x = pPoints[p].x + inc; 
    
    pList[nact].i = i;
    
    pList[nact].dx = ( pList[nact].dx + 4 ) / 8;
    
    pActive->num_edges++;
}

static void vre_active_list_del (vre_active_list *pActive, vre_int i)
{
    vre_int j, k;
    vre_poly_edge *pList = pActive->pList;
    
    for ( j = 0; j < pActive->num_edges; j++ )
    {
        if ( pList[j].i == i )
        {
            pActive->num_edges--;
            for (k=j; k < pActive->num_edges; k++)
            {
                pList[k].x  = pList[k+1].x;
                pList[k].dx = pList[k+1].dx;
                pList[k].i  = pList[k+1].i;
            }
            return;
        }
    }
    // edge not found
    vre_assert (0);    
}

// TODO: Convert to insert sort
// Insert sort will take advantage of the fact
// that most of the time the list is almost sorted.
static void vre_active_list_sort (vre_active_list *pActive)
{
    vre_int i, j, n;
    vre_poly_edge *pList = pActive->pList;
    
    vre_poly_edge tmpEdge;
    
    n = pActive->num_edges;
    
    for (i=0; i<n-1; i++) 
    {
        for (j=0; j<n-1-i; j++)
        {
            if (pList[j+1].x < pList[j].x) 
            { 
                // Swap edges
                tmpEdge.x = pList[j].x;
                tmpEdge.dx = pList[j].dx;
                tmpEdge.i = pList[j].i;
                
                pList[j].x = pList[j+1].x;
                pList[j].dx = pList[j+1].dx;
                pList[j].i = pList[j+1].i;
                
                pList[j+1].x = tmpEdge.x;
                pList[j+1].dx = tmpEdge.dx;
                pList[j+1].i = tmpEdge.i;
            }
        }
    }
}



static void vre_destroy_active_list (vre_active_list *pActive)
{
    vre_free (pActive->pList);
    vre_free (pActive);
}

/**

Note: points are expected in 16.16 format.

*/
vre_result vre_draw_polygon_concave ( vre_polygon *pPoly,
                                      vre_tile *pTile,
                                      vre_style *pStyle )                                       
{    
    vre_result  res;
    vre_int     k;
    vre_fix16   y0, y1, y;
    vre_int     i, j, xl, xr, t;
    vre_int16   *pInd;
    vre_uint32  num_points;
    vre_point   *pPoints;
    
    vre_uint32  scanline_offset;
    
    vre_uint32  c;
	
	vre_uint32 fg_color = pStyle->fg_color;
    
    vre_active_list *pActive;
    
    vre_assert ( pPoly != 0 );
    vre_assert ( pTile != 0 );
    vre_assert ( pStyle != 0 );
    
    num_points = vre_polygon_get_num_vertices ( pPoly );
    pPoints = vre_polygon_get_point (pPoly, 0);
    
    //
    // Alloc memory for some internal structures (TODO: handle externally)
    //
    
    pInd = (vre_int16*) vre_malloc ( num_points * sizeof (vre_uint16));
    res = vre_create_active_list ( &pActive, num_points );
    
    //
    // init structures
    //
    
    for ( i = 0; i < num_points; i++) 
    {
        pInd[i] = i;
    }
    
    k = 0;
    
    //
    // Sort points according to y. (a sorting index structure is produced)
    //
    
    for(i = 1; i < num_points; i++) 
    {
        vre_fix16 value = pPoints[pInd[i]].y;         
        vre_int32 ind = pInd[i];
        j = i - 1;
        while (j >= 0 && pPoints[pInd[j]].y > value) 
        { 
            pInd[j+1] = pInd[j];
            j--;
        }
        pInd[j+1] = ind;
    }
    
    //
    // Get largest and smallest y coords.
    //
    
    y0 = pPoints[pInd[0]].y;
    y1 = pPoints[pInd[num_points-1]].y;
    
    scanline_offset = (y0>>16) * pTile->scanline_width;
    
    //
    // Go thru all scanlines.
    //
    
    for ( y = y0; y < y1; y += 65536 )
    {
        
        //
        // Update active edge lists
        //
        
        //  while ( (k < num_points) && ( pPoints[pInd[k]].y <= y + 32768 ) )
        while ( (k < num_points) && ( pPoints[pInd[k]].y <= y ) )
        { 
            vre_int32 cur, prev, next;
            vre_uint32 start, end;
            cur = pInd[k];
            
            //
            // Check previous vertex
            //
            c = vre_polygon_get_contour(pPoly, cur);
            
            start = vre_polygon_get_contour_start(pPoly, c);
            end = vre_polygon_get_contour_end(pPoly, c);
            
            prev = ( cur > start ) ? cur-1 : end;
            if ( pPoints[prev].y <= y - 65536)
            {
                vre_active_list_del (pActive, prev);
            }
            else if (pPoints[prev].y > y) 
            {             
                vre_active_list_ins (pActive, pPoly, prev, y);
            }
            
            //
            // Check next vertex
            //
            
            next = ( cur < end ) ? cur+1 : start;
            if (pPoints[next].y <= y - 65536) 
            {
                vre_active_list_del (pActive, cur);
            }
            else if (pPoints[next].y > y) 
            {
                vre_active_list_ins (pActive, pPoly, cur, y);
            }                                   
            
            k++;
        }
        
        //
        // this sort should only be performed in case the active 
        // list has changed (Alt. introduce insert sort in vre_active_list_ins)
        //
        vre_active_list_sort ( pActive );
        
        //
        // Render scanline
        //
        
        for ( i = 0; i < pActive->num_edges; i += 2 )
        {
            vre_uint    offset;
            
            xl = (pActive->pList[i].x + 32768) >> 16;
            xr = (pActive->pList[i+1].x + 32768) >> 16;
            
            vre_assert ( xl <= xr );
            
            offset = (scanline_offset>>2) + xl;
            
            for ( t = xl; t <= xr; t++ )
            {
                pTile->pData[offset] = fg_color;
                offset++;
            }
            
            pActive->pList[i].x += pActive->pList[i].dx;
            pActive->pList[i+1].x += pActive->pList[i+1].dx;
        }
        
        scanline_offset += pTile->scanline_width;  
    }
    
    vre_free ( pInd );
    vre_destroy_active_list (pActive);
    
    return VRE_ERR_OK;
}

/*
static vre_uint32 computePixelCoverage ( vre_int32 pixel, 
                                         vre_int32 *pXLeft, 
                                         vre_int32 *pXRight)
{
    vre_int32 y;
    vre_uint32 area = 0;
    
    vre_int32 pixel_left = pixel << 16;
    vre_int32 pixel_right = pixel_left + 65535;
    
    for ( y = 0; y < 8; y++ )
    {
        vre_int32 partial_area = 0;
        
        if ( ( pXLeft[y] != -1 ) && ( pXRight[y] != -1 ) )
        {
            partial_area = VRE_MIN ( pXRight[y], pixel_right ) - 
            VRE_MAX ( pXLeft[y], pixel_left );
        }
        else if ( ( pXLeft[y] != -1 ) && ( pXRight[y] == -1) )
        {
            partial_area = pixel_right - VRE_MAX ( pXLeft[y], pixel_left );            
        }
        else if ( ( pXLeft[y] == -1 ) && ( pXRight[y] != -1) )
        {
            partial_area = VRE_MIN ( pXRight[y], pixel_right ) - pixel_left;
        }
        else
        {
            partial_area = 0;
        }
        
        
        if ( partial_area > 0 )
        {
            area += partial_area;
        }
    }
    
    return area;
}
*/

#define SUBPIXEL    8
 
static void vre_render_scanline ( vre_uint8 *pCoverage, 
                                  vre_uint32 length,
                                  vre_uint32 *pBuffer,
                                  vre_style *pStyle )
{
    vre_uint32 x;
    vre_uint8 alpha;
    vre_uint32 fg_color = pStyle->fg_color;
    
    // Render scanline
    for ( x = length+1; x > 0; x -- )
    {
        alpha = ((*pCoverage) * (fg_color >> 24))>>8;
        
        if ( alpha == 255 )
        {
            *pBuffer = fg_color;
        }
        else if ( alpha != 0 )
        {
            // TODO: use packed 32 bits mults (as in alpha_blend)
            // ( fg_color_pack_ag and fg_color_pack_rb precomputed).
            vre_uint32 r = (((fg_color & 0x00ff0000) >> 16) * alpha + 128) >> 8;
            vre_uint32 g = (((fg_color & 0x0000ff00) >> 8) * alpha + 128) >> 8;
            vre_uint32 b = (((fg_color & 0x000000ff) ) * alpha + 128) >> 8;
            
            vre_uint32 color;
            
            color = (alpha << 24) | (r<<16) | (g<<8) | b;
            *pBuffer = VRE_ALPHA_BLEND ( *pBuffer, color);
        }
        
        *pCoverage = 0;
        pCoverage++;
        pBuffer++;
    }
}

void heapsort(int arr[], unsigned int N)
{
    unsigned int n = N, i = n/2, parent, child;
    int t;
    
    for (;;) 
    {      //Loops until arr is sorted 
        if (i > 0) 
        { // First stage - Sorting the heap 
            i--;     // Save its index to i 
            t = arr[i];    // Save parent value to t 
        } 
        else 
        {     // Second stage - Extracting elements in-place 
            n--;           // Make the new heap smaller 
            if (n == 0) return; // When the heap is empty, we are done 
            t = arr[n];    // Save last value (it will be overwritten) 
            arr[n] = arr[0]; // Save largest value at the end of arr 
        }

        parent = i; // We will start pushing down t from parent 
        child = i*2 + 1; // parent's left child 

        // Sift operation - pushing the value of t down the heap 
        while (child < n) 
        {
            if (child + 1 < n  &&  arr[child + 1] > arr[child]) 
            {
                child++; /// Choose the largest child 
            }
    
            if (arr[child] > t) 
            {   // If any child is bigger than the parent 
                arr[parent] = arr[child]; // Move the largest child up 
                parent = child; // Move parent pointer to this child 
                child = parent*2 + 1; // Find the next child 
            } 
            else 
            {
                break; // t's place is found 
            }
        }
        
        arr[parent] = t; // We save t in the heap 
    }
}


vre_result vre_render_create ( vre_render **ppRender, 
                               vre_uint32 max_width )
{
    vre_result res;;
    vre_render *pRender;
    
    *ppRender = 0;
    
    pRender = VRE_TALLOC ( vre_render );
    if ( pRender == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    vre_memset( pRender, sizeof ( vre_render ), 0 );
    
    res = vre_create_active_list ( &(pRender->pActive), VRE_DEFAULT_NUM_POINTS );
    if ( res != VRE_ERR_OK )
    {
        vre_render_destroy ( pRender );
        return res;
    }
    
    pRender->pIndexes = (vre_int16*) vre_malloc ( VRE_DEFAULT_NUM_POINTS * 
                                                  sizeof (vre_uint16));
    if ( pRender->pIndexes == 0 )
    {
        vre_render_destroy ( pRender );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pRender->pAlpha = vre_malloc ( max_width );
    if ( pRender->pAlpha == 0 )
    {
        vre_render_destroy ( pRender );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pRender->max_num_vertices = VRE_DEFAULT_NUM_POINTS;
    
    *ppRender = pRender;
    
    return VRE_ERR_OK;
}


void vre_render_destroy ( vre_render *pRenderer )
{
    if ( pRenderer )
    {
        vre_destroy_active_list ( pRenderer->pActive );
        vre_free ( pRenderer->pIndexes );
        vre_free ( pRenderer );
    }
}

vre_result vre_render_realloc ( vre_render *pRender, vre_uint32 num_points )
{
    vre_result vres;
    
    vre_destroy_active_list ( pRender->pActive );
    vre_free ( pRender->pIndexes );
    
    vres = vre_create_active_list ( &(pRender->pActive), num_points );
    if ( vres != VRE_ERR_OK )
    {
        return vres;
    }
    
    pRender->pIndexes = (vre_int16*) vre_malloc ( num_points * sizeof (vre_uint16));
    if ( pRender->pIndexes == 0 )
    {
        vre_destroy_active_list ( pRender->pActive );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    return VRE_ERR_OK;
}

vre_result vre_draw_polygon_concave_aa ( vre_render *pRender,
                                         vre_polygon *pPoly,
                                         vre_tile *pTile,
                                         vre_style *pStyle )                                       
{    
    vre_result  res;
    vre_int     k;
    vre_fix16   y0, y1, y;
    vre_int32   i, j,  xl, xr;
    vre_int16   *pInd;
    vre_uint32  num_points;
    vre_point   *pPoints;
    
    vre_uint32  scanline_offset;
    
    vre_uint32  c;

    vre_int32   min_xleft;
    vre_int32   max_xleft;
    vre_int32   min_xright;
    vre_int32   max_xright;
    
    vre_active_list *pActive;

    vre_int32 ds_y = 0;
    
    vre_uint8 *pAlpha;
    vre_uint32 scan_min = 0xffffffff;
    vre_uint32 scan_max = 0;
    vre_uint32 offset;
    
    vre_assert ( pRender != 0 );
    vre_assert ( pPoly != 0 );
    vre_assert ( pTile != 0 );
    vre_assert ( pStyle != 0 );
    
    num_points = vre_polygon_get_num_vertices ( pPoly );
    pPoints = vre_polygon_get_point (pPoly, 0);
    
    //
    // Alloc memory for some internal structures if needed.
    //
    if ( pRender->max_num_vertices < num_points ) 
    {
        res = vre_render_realloc ( pRender, num_points );
        if ( res != VRE_ERR_OK )
        {
            return res;
        }
    }
    
    pInd = pRender->pIndexes;
    pActive = pRender->pActive;
    pAlpha = pRender->pAlpha;
    
    //
    // init structures
    //
    
    for ( i = 0; i < num_points; i++) 
    {
        pInd[i] = i;
    }
    
    k = 0;
    
    vre_memset(pAlpha, pTile->w, 0);
    
    pActive->num_edges = 0;
    
    //
    // Sort points according to y. (a sorting index structure is produced)
    //
    
    for(i = 1; i < num_points; i++) 
    {
        vre_fix16 value = pPoints[pInd[i]].y;         
        vre_int32 ind = pInd[i];
        j = i - 1;
        while (j >= 0 && pPoints[pInd[j]].y > value) 
        { 
            pInd[j+1] = pInd[j];
            j--;
        }
        pInd[j+1] = ind;
    }
    
    //
    // Get largest and smallest y coords.
    //
    
    y0 = pPoints[pInd[0]].y;
    y1 = pPoints[pInd[num_points-1]].y;
    
    scanline_offset = (y0>>16) * pTile->scanline_width;
    //
    // Go thru all scanlines.
    //
    
    for ( y = y0; y < y1; y += (65536/SUBPIXEL) )
    {
         ds_y = y >> 13;
        //
        // Update active edge lists
        //
        
        while ( (k < num_points) && ( pPoints[pInd[k]].y <= y ) )
        {
            vre_uint32 start, end;
            vre_int prev, next;
            vre_int cur = pInd[k];
            
            c = vre_polygon_get_contour(pPoly, cur);
            
            start = vre_polygon_get_contour_start ( pPoly, c );
            end = vre_polygon_get_contour_end ( pPoly, c );
            vre_assert ( start != end );
            
            prev = ( cur > start ) ? cur-1 : end;
            next = ( cur < end ) ? cur+1 : start;
            
            if ( pPoints[prev].y <= y - 65536/SUBPIXEL)
            {
                vre_active_list_del (pActive, prev);
            } 
            else  if (pPoints[prev].y > y ) 
            {             
                vre_active_list_ins2 (pActive, pPoly, prev, y);
            }
            
            if (pPoints[next].y <= y - 65536/SUBPIXEL) 
            {
                vre_active_list_del (pActive, cur);
            }
            else if (pPoints[next].y > y ) 
            {
                vre_active_list_ins2 (pActive, pPoly, cur, y);
            }
            
            k++;
        }
        
        //
        // this sort should only be performed in case the active 
        // list has changed (Alt. introduce insert sort in vre_active_list_ins,
        // but then it will only be compatible with non-self intersecting polygons)
        //
        if (  pActive->num_edges > 0 )
        {
            vre_active_list_sort ( pActive );
        }
        
        //
        // Render scanlines
        //
        offset = 0;
        for ( i = 0; i < pActive->num_edges; i += 2 )
        {
            vre_int32 area;
            vre_int32 pixel_left;
            vre_int32 pixel_right;
            
            xl = pActive->pList[i].x;
            xr = pActive->pList[i+1].x;
            
            vre_assert ( xl <= xr );
            
            min_xleft =  xl & 0xffff0000;
            max_xleft =  (xl + 65535) & 0xffff0000;
            
            min_xright =  xr & 0xffff0000;
            max_xright = (xr + 65535) & 0xffff0000;                
        
            offset = min_xleft >>16;
            
            scan_min = offset < scan_min ? offset : scan_min;
            
            vre_assert ( min_xleft <= max_xright );    
            
            // Left 
            for ( pixel_left = min_xleft; 
                  pixel_left < max_xleft; 
                  pixel_left += 65536 )
            {
                pixel_right = pixel_left + 65535;
                
                area = VRE_MIN ( xr, pixel_right ) - VRE_MAX ( xl, pixel_left );
                
                if ( area < 0 ) area = 0;
                area = (area + 1024) / 2048;
                
                area += pAlpha[offset];
                if ( area > 255 ) area = 255;
                pAlpha[offset] = area;
                
                vre_assert (offset < pTile->w );
                
                offset++;
            }
            
            // Middle
            while ( pixel_left < min_xright )
            {
                area = 256/8 + pAlpha[offset];
                
                if ( area > 255 ) area = 255;
                
                pAlpha[offset] = area;
                
                vre_assert (offset < pTile->w );
                
                offset++;
                pixel_left+= 65536;
            }
            
            // Right
            while ( pixel_left < max_xright )
            {
                pixel_right = pixel_left + 65535;
                
                area = VRE_MIN ( xr, pixel_right ) - VRE_MAX ( xl, pixel_left );
                
                if ( area < 0 ) area = 0;
                area = (area + 1024) / 2048;
                
                area += pAlpha[offset];
                if ( area > 255 ) area = 255;
                pAlpha[offset] = area;
                
                vre_assert (offset < pTile->w );
                
                offset++;
                pixel_left+= 65536;
            }
            
            scan_max = scan_max < offset ? offset : scan_max;
            
            pActive->pList[i].x += pActive->pList[i].dx;
            pActive->pList[i+1].x += pActive->pList[i+1].dx;
        }
        
        //
        // Render scanline
        //
        if ( ( ds_y & 0x07 ) == 7 ) 
        {   
            if ( scan_min != 0xffffffff)
            {
                vre_assert ( scan_min <= scan_max );
                vre_assert ( scan_max <= pTile->w );
            
                if ( scan_max == pTile->w )
                {
                    scan_max --;
                }
            
                offset = (scanline_offset>>2) + scan_min;
                vre_render_scanline( &pAlpha[scan_min], 
                                     scan_max - scan_min, 
                                     &pTile->pData[offset],
                                     pStyle);
                      
                scan_min = 0xffffffff;
                scan_max = 0;
            }
            scanline_offset += pTile->scanline_width;
        }
    }

    // Render Last Scanline if needed.
    if ( ( ( ds_y & 0x07 ) != 7 ) & ( scan_min != 0xffffffff) )
    {
        vre_assert ( scan_min <= scan_max );
        vre_assert ( scan_max <= pTile->w );
        
        if ( scan_max == pTile->w )
        {
            scan_max --;
        }
        
        offset = (scanline_offset>>2) + scan_min;
        vre_render_scanline( &pAlpha[scan_min], 
                             scan_max - scan_min, 
                             &pTile->pData[offset],
                             pStyle);
    }
    
    return VRE_ERR_OK;
}


