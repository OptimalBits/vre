

#ifndef VRE_MATRIX_H
#define VRE_MATRIX_H

#include "vre_defs.h"

typedef struct vre_mat3x3 
{
    vre_fix16   m[9];
} vre_mat3x3;


void vre_matrix_identity ( vre_mat3x3 *pMat );

void vre_matrix_set (vre_mat3x3 *pMat, 
                     vre_fix16 a, vre_fix16 b, vre_fix16 c,
                     vre_fix16 d, vre_fix16 e, vre_fix16 f,
                     vre_fix16 g, vre_fix16 h, vre_fix16 i );


/**
    Post-concatenation matrix operations.
*/
void vre_matrix_rotate ( vre_mat3x3 *pMat, vre_fix16 angle );

void vre_matrix_scale ( vre_mat3x3 *pMat, vre_fix16 sx, vre_fix16 sy );

void vre_matrix_translate ( vre_mat3x3 *pMat, 
                            vre_fix16 tx, vre_fix16 ty );
                            
void vre_matrix_skew ( vre_mat3x3 *pMat, vre_fix16 fx, vre_fix16 fy);

void vre_matrix_mult ( vre_mat3x3 *pMatA, vre_mat3x3 *pMatB, 
                       vre_mat3x3 *pMatDst );

void vre_matrix_copy ( vre_mat3x3 *pMatDst, vre_mat3x3 *pMatSrc );

void vre_matrix_transform ( vre_mat3x3 *pMat, 
                            vre_point *pSrcPoints, 
                            vre_point *pDstPoints, 
                            vre_uint32 num_points );

#endif

