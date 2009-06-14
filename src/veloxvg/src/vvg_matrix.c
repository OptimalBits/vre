/*
 *  vvg_matrix.c
 *  vre_test
 *
 *  Created by Manuel Astudillo on 2007-11-04.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "openvg.h"
#include "vvg_matrix.h"
#include "vvg_context.h"



VG_API_CALL void vgLoadIdentity(void)
{
    vre_matrix_identity ( veloxContext.matrix.pCurrent );
}

VG_API_CALL void vgLoadMatrix(const VGfloat * m)
{
    vre_matrix_set (  veloxContext.matrix.pCurrent,
                    m[0]*65536, m[1]*65536, m[2]*65536,
                    m[3]*65536, m[4]*65536, m[5]*65536,
                    m[6]*65536, m[7]*65536, m[8]*65536 );
}

VG_API_CALL void vgGetMatrix(VGfloat * m)
{
    vre_mat3x3 *pMat;
    
    pMat = veloxContext.matrix.pCurrent;
    m[0] = pMat->m[0] * 65536.0f;
    m[1] = pMat->m[0] * 65536.0f;
    m[2] = pMat->m[0] * 65536.0f;
    
    m[3] = pMat->m[0] * 65536.0f;
    m[4] = pMat->m[0] * 65536.0f;
    m[5] = pMat->m[0] * 65536.0f;
    
    m[6] = pMat->m[0] * 65536.0f;
    m[7] = pMat->m[0] * 65536.0f;
    m[8] = pMat->m[0] * 65536.0f;
}

VG_API_CALL void vgMultMatrix(const VGfloat * m)
{
    vre_mat3x3 matB;
    vre_mat3x3 matDst;
    
    vre_matrix_set (  &matB,
                    (long) (m[0]*65536), (long) m[1]*65536, (long) (m[2]*65536),
                    (long) (m[3]*65536), (long) m[4]*65536, (long) (m[5]*65536),
                    (long) (m[6]*65536), (long) m[7]*65536, (long) (m[8]*65536) );
    
    vre_matrix_mult ( veloxContext.matrix.pCurrent, &matB, &matDst );
    
    *(veloxContext.matrix.pCurrent) = matDst;
}

VG_API_CALL void vgTranslate(VGfloat tx, VGfloat ty)
{
    vre_matrix_translate ( veloxContext.matrix.pCurrent, 
                          (long) (tx * 65536),  (long) (ty * 65536) );
}

VG_API_CALL void vgScale(VGfloat sx, VGfloat sy)
{
    vre_matrix_scale ( veloxContext.matrix.pCurrent, 
                      (long) (sx * 65536),  (long) (sy * 65536) );
}

VG_API_CALL void vgShear(VGfloat shx, VGfloat shy)
{
    vre_matrix_skew (  veloxContext.matrix.pCurrent, 
                     (long) shx * 65536,  (long) shy * 65536 );
}

VG_API_CALL void vgRotate(VGfloat angle)
{
    vre_matrix_rotate ( veloxContext.matrix.pCurrent,  (long) (angle * 65536) );
}

