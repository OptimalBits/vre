
/*
 *  vvg_matrix.h
 *  Velox VG
 *
 *  Created by Manuel Astudillo on 2007-11-04.
 *  Copyright 2007  Software. All rights reserved.
 *
 */

#ifndef VVG_MATRIX_H
#define VVG_MATRIX_H

#include "openvg.h"
#include "vre_matrix.h"

typedef struct
{
    VGMatrixMode mode;
    
    vre_mat3x3 *pCurrent;
    
    vre_mat3x3 pathMatrix;
    vre_mat3x3 imageMatrix;
    vre_mat3x3 fillPaintMatrix;
    vre_mat3x3 strokePaintMatrix;
} veloxvg_matrix;

#endif
