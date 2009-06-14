/*
	Vectorial Rendering Engine 
	(c) 2005, Synthetica Software Ltd.

    Polygon renderers.
*/

#include "vre_defs.h"
#include "vre_context.h"
#include "vre_assert.h"
#include "vre_polygon.h"
#include "vre_mem.h"


#define VARRAY_INC  256
#define MAX_NUM_CONTOURS 32

struct vre_polygon
{
    vre_point 	*pVertices;    /// Vertices
    vre_point   *pTVertices;   /// Transformed vertices ( TO BE REMOVED; USE ANOTHR POLYGON TO SAVE TRANSFORMED VERTEXES)
    vre_uint32   num_vertices; // Number of used vertices.
    vre_uint32   size;         // Number of total vertices.
    
    //
    // To render glyphs, it is required extra structures.
    //
    vre_uint32  *pContoursStart; // First vertex position in this contour.
    vre_uint32  *pContoursEnd;   // Last vertex position in this contour.
    vre_uint32  num_contours;
    vre_uint32  max_num_contours;
    
    // RENDER_FUNC_PTR // Function pointer to the renderer av this polygon.
    // The engine will automatically generate a lot of different 
    // polygon fillers.
};




vre_result vre_polygon_create ( vre_polygon **ppPoly,
								vre_point *pVertices,
								vre_uint32 num_points )
{
	vre_polygon *pPoly;
	
	vre_assert ( num_points >= 3 );
	
	pPoly = VRE_TALLOC ( vre_polygon );
	
	if ( pPoly == 0 )
	{
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
    
    vre_memset(pPoly, sizeof(vre_polygon), 0);
	
	pPoly->pVertices = 
		(vre_point*) vre_malloc ( sizeof (vre_point) * num_points );
	
	if ( pPoly->pVertices == 0 )
	{
		vre_free ( pPoly );
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
	
	pPoly->pTVertices = 
		(vre_point*) vre_malloc ( sizeof (vre_point) * num_points );
	
	if ( pPoly->pTVertices == 0 )
	{
		vre_free ( pPoly );
		vre_free ( pPoly->pVertices );
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
	
	vre_memcopy8 (pPoly->pVertices, pVertices, sizeof (vre_point) * num_points);

   	pPoly->size = num_points;	
	pPoly->num_vertices = num_points;
	
	*ppPoly = pPoly;
	
	return VRE_ERR_OK;	
}


vre_result vre_polygon_createEx ( vre_polygon **ppPoly )
{
  	vre_polygon *pPoly;
	
	pPoly = VRE_TALLOC ( vre_polygon );
	
	if ( pPoly == 0 )
	{
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
    
    vre_memset(pPoly, sizeof(vre_polygon), 0);
    
    pPoly->max_num_contours = MAX_NUM_CONTOURS;
    pPoly->pContoursStart = vre_malloc ( sizeof (vre_uint32)*MAX_NUM_CONTOURS );
    
    if ( pPoly->pContoursStart == 0 )
    {
        vre_free ( pPoly );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pPoly->pContoursEnd = vre_malloc ( sizeof (vre_uint32)*MAX_NUM_CONTOURS );
    
    if ( pPoly->pContoursEnd == 0 )
    {
        vre_free ( pPoly->pContoursStart );
        vre_free ( pPoly );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }

    *ppPoly = pPoly; 
    
    return VRE_ERR_OK;
}



vre_result vre_polygon_create2 ( vre_polygon **ppPoly, 								
								 vre_uint32 num_points )
{
	vre_polygon *pPoly;
	
	vre_assert ( num_points >= 3 );
	
	pPoly = VRE_TALLOC ( vre_polygon );
	
	if ( pPoly == 0 )
	{
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
    
    vre_memset(pPoly, sizeof(vre_polygon), 0);
	
	pPoly->pVertices = 
		(vre_point*) vre_malloc ( sizeof (vre_point) * num_points );
	
	if ( pPoly->pVertices == 0 )
	{
		vre_free ( pPoly );
		return VRE_ERR_MEM_ALLOC_FAILED;
	}	
	
	pPoly->pTVertices = 
		(vre_point*) vre_malloc ( sizeof (vre_point) * num_points );
	
	if ( pPoly->pTVertices == 0 )
	{
		vre_free ( pPoly );
		vre_free ( pPoly->pVertices );
		return VRE_ERR_MEM_ALLOC_FAILED;
	}
	
	pPoly->num_vertices = 0;
	pPoly->size = num_points;
	
	*ppPoly = pPoly;
	
	return VRE_ERR_OK;	
}


vre_result vre_polygon_create3 ( vre_polygon **ppPoly,
								 vre_uint32 num_points,
								 vre_uint16 num_contours )
{
    vre_polygon *pPoly;

    vre_result vres = vre_polygon_create2 (&pPoly, num_points);

    if ( vres != VRE_ERR_OK )
    {
        return vres;
    }

    for (;;)
    {            
        pPoly->max_num_contours = num_contours;
        pPoly->pContoursStart = vre_malloc ( sizeof (vre_uint32)*num_contours );
    
        if ( pPoly->pContoursStart == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;    
            break;
        }
    
        pPoly->pContoursEnd = vre_malloc ( sizeof (vre_uint32)*num_contours );
    
        if ( pPoly->pContoursEnd == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;    
            break;
        }
    
        break;
    }
    
    if ( vres == VRE_ERR_OK )
    {
        *ppPoly = pPoly;
    }
    else
    {
        vre_polygon_destroy ( pPoly );
        *ppPoly = 0;
    }
            
    
    return VRE_ERR_OK;
}


void vre_polygon_destroy ( vre_polygon *pPolygon )
{
    if ( pPolygon != 0 )
    {
        vre_free ( pPolygon->pTVertices );
        vre_free ( pPolygon->pVertices );
        vre_free ( pPolygon->pContoursStart );
        vre_free ( pPolygon->pContoursEnd );
        vre_free ( pPolygon );
    }
}


vre_result 
vre_polygon_add_point ( vre_polygon *pPoly, vre_point *pPoint )
{
    vre_assert ( pPoly != 0 );
    vre_assert ( pPoint != 0 );
    vre_assert ( pPoly->size >= pPoly->num_vertices );

    if ( pPoly->size == pPoly->num_vertices )
    {
        vre_uint32 new_size = pPoly->num_vertices + VARRAY_INC;
        vre_uint32 num_bytes = sizeof(vre_point) * new_size;
            
        vre_point *pPoints = vre_malloc( num_bytes );

        if ( pPoints == 0 )
        {
            return VRE_ERR_MEM_ALLOC_FAILED;
        }
        else
        {
            if ( pPoly->pVertices != 0 )
            {
                vre_memcopy8( pPoints, pPoly->pVertices, 
                             sizeof(vre_point) * pPoly->size );

                vre_free( pPoly->pVertices );
            }
            pPoly->pVertices = pPoints;
            pPoly->size = new_size;            
        }        
    }
        
    pPoly->pVertices[pPoly->num_vertices].x = pPoint->x;
    pPoly->pVertices[pPoly->num_vertices].y = pPoint->y;
    
    pPoly->num_vertices++;
    
    return VRE_ERR_OK;
}


vre_result
vre_polygon_add_points ( vre_polygon *pPoly, vre_point *pPoint, vre_uint32 n )
{
    vre_result vres = VRE_ERR_OK;
    vre_uint32 i;

    vre_assert ( pPoly != 0 );
    vre_assert ( pPoint != 0 );
    vre_assert ( pPoly->size >= pPoly->num_vertices );
    
    for ( i = 0; i < n; i++ )
    {
        vres = vre_polygon_add_point( pPoly, &pPoint[i] );
        if ( vres != VRE_ERR_OK )
        {
            break;
        }
    }
    
    return vres;
}

vre_point *vre_polygon_get_point ( vre_polygon *pPoly, vre_uint32 point )
{
    vre_assert ( point < pPoly->num_vertices );
    
    return &pPoly->pVertices [point];
}


vre_result vre_polygon_clone ( vre_polygon **ppDst, vre_polygon *pSrc )
{
    vre_result vres;
    
    vre_polygon *pDst;
    
    vres = vre_polygon_create(ppDst, pSrc->pVertices, pSrc->num_vertices);
    VRE_RETURN_IF_ERR ( vres );
    
    pDst = *ppDst;
    
    pDst->max_num_contours = pSrc->max_num_contours;
    pDst->num_contours = pSrc->num_contours;
    
    pDst->pContoursStart = vre_malloc( pDst->max_num_contours * 
                                       sizeof ( vre_uint32 ));
    if ( pDst->pContoursStart == 0 )
    {
        vre_polygon_destroy(pDst);
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memcopy8( pDst->pContoursStart, 
                  pSrc->pContoursStart, 
                  pDst->num_contours * sizeof ( vre_uint32 ));
    
    pDst->pContoursEnd = vre_malloc( pDst->max_num_contours * 
                                      sizeof ( vre_uint32 ));
    if ( pDst->pContoursEnd == 0 )
    {
        vre_polygon_destroy(pDst);
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memcopy8( pDst->pContoursEnd, 
                  pSrc->pContoursEnd, 
                  pDst->num_contours * sizeof ( vre_uint32 ));
    
    return VRE_ERR_OK;
}

void vre_polygon_copy ( vre_polygon *pSrc, vre_polygon *pDst )
{
// To be implented

}

vre_uint32 
vre_polygon_get_contour ( vre_polygon *pPoly, vre_uint32 point_index )
{
    vre_uint32 c;
    vre_uint32 *pContoursStart;
    vre_uint32 *pContoursEnd;
    
    vre_assert ( pPoly != 0 );
    vre_assert ( point_index < pPoly->num_vertices );
    
    pContoursStart = pPoly->pContoursStart;
    pContoursEnd = pPoly->pContoursEnd;
    
    // TODO: use binary search
    for ( c = 0; c < pPoly->num_contours; c++ )
    {
        if ( ( point_index >= pContoursStart[c] ) &&
             ( point_index <= pContoursEnd[c] ) )
        {
            break;
        }
    }
    
    vre_assert ( c < pPoly->num_contours );
    
    return c;
}


vre_uint32 vre_polygon_get_contour_start ( vre_polygon *pPoly, vre_uint32 contour )
{
    vre_assert ( contour < pPoly->num_contours );
    return pPoly->pContoursStart[contour];
}

vre_uint32 vre_polygon_get_contour_end ( vre_polygon *pPoly, vre_uint32 contour )
{
    vre_assert ( contour < pPoly->num_contours );
    return pPoly->pContoursEnd[contour];
}


void vre_polygon_swap_coords ( vre_polygon *pPoly )
{
    vre_point*  pVertices;
    
    vre_assert ( pPoly != 0 );
    
    pVertices = pPoly->pVertices;
    pPoly->pVertices = pPoly->pTVertices;
    pPoly->pTVertices = pVertices;
}


void vre_polygon_transform ( vre_polygon *pPoly, vre_mat3x3 *pMatrix )
{
    vre_assert ( pPoly != 0 );
    vre_assert ( pMatrix != 0 );
    
    vre_matrix_transform ( pMatrix, 
                           pPoly->pVertices, 
                           pPoly->pTVertices, 
                           pPoly->num_vertices );
}

vre_result vre_polygon_start_contour ( vre_polygon *pPoly )
{
    pPoly->pContoursStart[pPoly->num_contours] = pPoly->num_vertices;
    
    return VRE_ERR_OK;
}

vre_result vre_polygon_end_contour ( vre_polygon *pPoly )
{
    pPoly->pContoursEnd[pPoly->num_contours] = pPoly->num_vertices - 1;
    pPoly->num_contours ++;
    
    return VRE_ERR_OK;
}

vre_uint32 vre_polygon_get_num_vertices ( vre_polygon *pPoly )
{
    return pPoly->num_vertices;
}

vre_uint32 vre_polygon_get_num_contours ( vre_polygon *pPoly )
{
    return pPoly->num_contours;
}


vre_result vre_polygon_set_num_contours ( vre_polygon *pPoly, 
                                          vre_uint32 num_contours )
{
    vre_free ( pPoly->pContoursStart );
    vre_free ( pPoly->pContoursEnd );
    
    pPoly->pContoursStart = 0;
    pPoly->pContoursEnd = 0;
    
    pPoly->max_num_contours = num_contours;
    
    pPoly->pContoursStart = vre_malloc ( sizeof (vre_uint32) * num_contours);
    if ( pPoly->pContoursStart == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    vre_memset ( pPoly->pContoursStart, 
                 sizeof (vre_uint32) * num_contours,
                 0 );

    pPoly->pContoursEnd = vre_malloc ( sizeof (vre_uint32) * num_contours);
    if ( pPoly->pContoursEnd == 0 )
    {
        vre_free ( pPoly->pContoursStart );
        pPoly->pContoursStart = 0;
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset ( pPoly->pContoursEnd,  
                 sizeof (vre_uint32) * num_contours, 
                 0 );
    
    pPoly->num_contours = 0;
    
    return VRE_ERR_OK;
}

void vre_polygon_reset ( vre_polygon *pPoly )
{
    vre_assert ( pPoly );
    
    vre_free( pPoly->pVertices );
    pPoly->num_vertices = 0;
    pPoly->num_contours = 0;
    pPoly->pVertices = 0;
    pPoly->size = 0;       
}

vre_result vre_polygon_copy_points ( vre_polygon *pDst, vre_polygon *pSrc )
{
    vre_result vres = VRE_ERR_OK;
  //  vre_assert ( pDst->size - pDst->num_vertices > pSrc->num_vertices );
    if ( pSrc->num_vertices > 0 )
    {
        vres = vre_polygon_add_points( pDst, pSrc->pVertices, 
                                       pSrc->num_vertices );
    }
    
    return vres;
}

// Just a hack function...
vre_result vre_polygon_trasform_points ( vre_polygon *pDst, 
                                         vre_polygon *pSrc,
                                         vre_mat3x3 *pMat )
{
   // vre_result vres = VRE_ERR_OK;
 //   vre_assert ( pDst->num_vertices != pSrc->num_vertices );
    
    if ( pSrc->num_vertices > 0 )
    {        
        vre_matrix_transform( pMat, pSrc->pVertices, 
                              pDst->pVertices, 
                              pDst->num_vertices );
    }
    
    return VRE_ERR_OK;
}

vre_result
vre_polygon_add_contour ( vre_polygon *pPoly, vre_polygon *pContour )
{
    vre_result vres;
    
    vres = vre_polygon_start_contour ( pPoly );
    VRE_RETURN_IF_ERR ( vres );
    
    // vres =
    vre_polygon_copy_points ( pPoly, pContour );
    VRE_RETURN_IF_ERR ( vres );
    
    vres = vre_polygon_end_contour ( pPoly );
    VRE_RETURN_IF_ERR ( vres );
    
    return VRE_ERR_OK;
}

