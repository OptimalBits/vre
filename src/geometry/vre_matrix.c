

#include "vre_matrix.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"


/**
    Matrix functions for 2D linear geometry 
    using homogenous coordinates.
 
    Matrix is represented in column order.
*/

#define MATRIX_MULT( d, s1, s2 ) \
    d[0] = (( vre_int64 ) s1[0] * s2[0] + ( vre_int64 ) s1[1] * s2[3] + ( vre_int64 ) s1[2] * s2[6] + 32768) >> 16;\
    d[1] = (( vre_int64 ) s1[0] * s2[1] + ( vre_int64 ) s1[1] * s2[4] + ( vre_int64 ) s1[2] * s2[7] + 32768) >> 16;\
    d[2] = (( vre_int64 ) s1[0] * s2[2] + ( vre_int64 ) s1[1] * s2[5] + ( vre_int64 ) s1[2] * s2[8] + 32768) >> 16;\
\
    d[3] = (( vre_int64 ) s1[3] * s2[0] + ( vre_int64 ) s1[4] * s2[3] + ( vre_int64 ) s1[5] * s2[6]+ 32768) >> 16;\
    d[4] = (( vre_int64 ) s1[3] * s2[1] + ( vre_int64 ) s1[4] * s2[4] + ( vre_int64 ) s1[5] * s2[7]+ 32768) >> 16;\
    d[5] = (( vre_int64 ) s1[3] * s2[2] + ( vre_int64 ) s1[4] * s2[5] + ( vre_int64 ) s1[5] * s2[8]+ 32768) >> 16;\
\
    d[6] = (( vre_int64 ) s1[6] * s2[0] + ( vre_int64 ) s1[7] * s2[3] + ( vre_int64 ) s1[8] * s2[6]+ 32768) >> 16;\
    d[7] = (( vre_int64 ) s1[6] * s2[1] + ( vre_int64 ) s1[7] * s2[4] + ( vre_int64 ) s1[8] * s2[7]+ 32768) >> 16;\
    d[8] = (( vre_int64 ) s1[6] * s2[2] + ( vre_int64 ) s1[7] * s2[5] + ( vre_int64 ) s1[8] * s2[8]+ 32768) >> 16;


void vre_matrix_identity ( vre_mat3x3 *pMat )
{    
    pMat->m[0] = 65536;
    pMat->m[1] = 0;
    pMat->m[2] = 0;
    
    pMat->m[3] = 0;
    pMat->m[4] = 65536;
    pMat->m[5] = 0;
    
    pMat->m[6] = 0;
    pMat->m[7] = 0;
    pMat->m[8] = 65536;    
}

void vre_matrix_set (vre_mat3x3 *pMat, 
                     vre_fix16 a, vre_fix16 b, vre_fix16 c,
                     vre_fix16 d, vre_fix16 e, vre_fix16 f,
                     vre_fix16 g, vre_fix16 h, vre_fix16 i )
{
    pMat->m[0] = a;
    pMat->m[1] = b;
    pMat->m[2] = c;
    
    pMat->m[3] = d;
    pMat->m[4] = e;
    pMat->m[5] = f;
    
    pMat->m[6] = g;
    pMat->m[7] = h;
    pMat->m[8] = i;
}


void vre_matrix_rotate ( vre_mat3x3 *pMat, vre_fix16 angle )
{
    vre_fix16   c = vre_cos (angle);
    vre_fix16   s = vre_sin (angle);
 
    vre_mat3x3 rot;
    vre_mat3x3 tmp;
    
    rot.m[0] = c;  rot.m[1] = s;   rot.m[2] = 0;
    rot.m[3] = -s; rot.m[4] = c;   rot.m[5] = 0;
    rot.m[6] = 0;  rot.m[7] = 0;   rot.m[8] = 1<<16;
    
    MATRIX_MULT ( (tmp.m), (pMat->m), (rot.m) );
    
    *pMat = tmp;
}

void vre_matrix_scale ( vre_mat3x3 *pMat, vre_fix16 sx, vre_fix16 sy )
{
    vre_int64   a, b, d, e, g, h;
    
    a = ((vre_int64) pMat->m[0]) * sx;
    b = ((vre_int64) pMat->m[1]) * sy;
    
    d = ((vre_int64) pMat->m[3]) * sx;
    e = ((vre_int64) pMat->m[4]) * sy;
    
    g = ((vre_int64) pMat->m[6]) * sx;
    h = ((vre_int64) pMat->m[7]) * sy;
    
    pMat->m[0] = (vre_fix16) ((a+32768) >> 16);
    pMat->m[1] = (vre_fix16) ((b+32768) >> 16);
    
    pMat->m[3] = (vre_fix16) ((d+32768) >> 16);
    pMat->m[4] = (vre_fix16) ((e+32768) >> 16);
    
    pMat->m[6] = (vre_fix16) ((g+32768) >> 16);
    pMat->m[7] = (vre_fix16) ((h+32768) >> 16);        
}

void vre_matrix_translate ( vre_mat3x3 *pMat, 
                            vre_fix16 tx, vre_fix16 ty )
{  
    pMat->m[6] +=  tx;
    pMat->m[7] +=  ty;    
}

void vre_matrix_skew ( vre_mat3x3 *pMat, vre_fix16 fx, vre_fix16 fy )
{


}

void vre_matrix_mult ( vre_mat3x3  *pMatA, 
                       vre_mat3x3  *pMatB, 
                       vre_mat3x3  *pMatDst )
{
    MATRIX_MULT ( (pMatDst->m), (pMatA->m), (pMatB->m) );
}


void vre_matrix_copy (  vre_mat3x3 *pMatDst, 
                        vre_mat3x3 *pMatSrc )
{
    vre_assert ( pMatDst );
    vre_assert ( pMatSrc );

    *pMatDst = *pMatSrc;
}

vre_result vre_matrix_clone ( vre_mat3x3 **ppMatDst, vre_mat3x3 *pMatSrc )
{
    vre_assert ( ppMatDst );
    vre_assert ( pMatSrc );
    
    *ppMatDst = ( vre_mat3x3* ) vre_malloc(sizeof ( vre_mat3x3));
    if ( *ppMatDst == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    **ppMatDst = *pMatSrc;
    
    return VRE_ERR_OK;
}


static void vre_matrix_transform_point ( vre_mat3x3 *pMat,
                                         vre_point *pPoint,
                                         vre_point *pTPoint )
{   
    pTPoint->x = (vre_int32) ((( ((vre_int64) pMat->m[0]) * pPoint->x + 
                   ((vre_int64) pMat->m[1]) * pPoint->y )+32768) >> 16);
    pTPoint->x += pMat->m[6];               
                   
    pTPoint->y = (vre_int32) ((( ((vre_int64) pMat->m[3]) * pPoint->x + 
                   ((vre_int64) pMat->m[4]) * pPoint->y )+32768) >> 16);
    pTPoint->y += pMat->m[7];                   
}

void vre_matrix_transform ( vre_mat3x3 *pMat, 
                            vre_point *pSrcPoints, 
                            vre_point *pDstPoints, 
                            vre_uint32 num_points )
{
    vre_uint32  i;
    
    for ( i = 0; i < num_points; i++ )
    {
        vre_matrix_transform_point ( pMat, &pSrcPoints[i], &pDstPoints[i] );    
    }
}





