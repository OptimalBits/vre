

#include "vre_defs.h"
#include "vre_math.h"
#include "vre_assert.h"


// TODO: convert to 16 bits fixed point
vre_int32 vre_vector_length ( vre_point *pPoint )
{
    return vre_sqrt ( ( (vre_int64) pPoint->x * pPoint->x + 
                        (vre_int64) pPoint->y * pPoint->y + 32768) >> 16 );
}

//
// Aproximation of sqrt distance, based on octagon.
//
// distance_approx (x, y) = 
//    (1 + 1/(4-2*√2))/2 * min((1 / √2)*(|x|+|y|), max (|x|, |y|))

// 1/√2
#define ONE_OVER_SQRT_2 46341
// (1 + 1/(4-2*√2))/2
#define DISTANCE_FACTOR (vre_uint32)(((1.0 + 1.0/(4-2*1.41421356237))/2.0) * 65536)

vre_ufix16 vre_vector_length_apprx ( vre_point *pPoint )
{
    vre_uint32 x_abs = VRE_ABS (pPoint->x);
    vre_uint32 y_abs = VRE_ABS (pPoint->y);
    
    vre_uint32 xy_sum = x_abs + y_abs;
    
    vre_uint32 m1 = MUL64SH (ONE_OVER_SQRT_2, xy_sum, 16);
    vre_uint32 m2 = VRE_MAX (x_abs, y_abs);
    
    vre_uint32 r1 = VRE_MIN (m1, m2);
    
    vre_uint32 r2 = MUL64SH( DISTANCE_FACTOR, r1, 16 );
    
    return r2;
}


void vre_vector_scale (vre_point *pPoint, vre_fix8 scale)
{
    vre_uint32 length = vre_vector_length ( pPoint );

    if ( length > 0 )
    {      
        pPoint->x = (((vre_int64) pPoint->x) << 16) / length;
        pPoint->y = (((vre_int64) pPoint->y) << 16) / length;

        pPoint->x = ((vre_int64) pPoint->x * scale + 32768) >> 16;
        pPoint->y = ((vre_int64) pPoint->y * scale + 32768) >> 16;
    }
}


vre_fix16 vre_vector_dot_product (vre_point *pU, vre_point *pV)
{
    return (((vre_int64) pU->x * pV->x + (vre_int64) pU->y * pV->y) + 32768 ) >> 16;
}



// Move to proper C file
// Check for parallels lines elsewhere!
/*
void vre_line_intersect ( vre_point *pNormal1, vre_point *pPoint1, 
                         vre_point *pNormal2, vre_point *pPoint2,
                         vre_point *pIntersection )
{
    vre_fix16 C1, C2;
    
    vre_fix16 determinant = ( pNormal1->x * pNormal2->y - 
                              pNormal2->x * pNormal1->y  );
    
    vre_assert ( determinant != 0 );
    
    C1 = ( ( vre_int64 ) pNormal1->x * pPoint1->x + pNormal1->y * pPoint1->y + 32768 ) >> 16;
    C2 = ( ( vre_int64 ) pNormal2->x * pPoint2->x + pNormal2->y * pPoint2->y + 32768 ) >> 16; 
    
    pIntersection->x = ( ( vre_int64 ) pNormal2->y * C1 - pNormal1->y * C2 + 32768 ) / determinant;
    pIntersection->y = ( ( vre_int64 ) pNormal2->x * C1 - pNormal1->x * C2 + 32768 ) / determinant;
}
*/


void vre_line_intersect ( vre_point *pNormal1, vre_point *pPoint1,
                          vre_point *pNormal2, vre_point *pPoint2,
                          vre_point *pIntersection )
{
    float x1, y1, x2, y2;
    float A1, B1, A2, B2;
    float C1, C2;
    float det;
    
    vre_assert ( !(( pNormal1->x == 0 ) && ( pNormal1->y == 0 ) ) );
    vre_assert ( !(( pNormal2->x == 0 ) && ( pNormal2->y == 0 ) ) );
    
    x1 = pPoint1->x / 65536.0f;
    y1 = pPoint1->y / 65536.0f;
    x2 = pPoint2->x / 65536.0f;
    y2 = pPoint2->y / 65536.0f;
    
    A1 = pNormal1->x / 65536.0f;
    B1 = pNormal1->y / 65536.0f;
    A2 = pNormal2->x / 65536.0f;
    B2 = pNormal2->y / 65536.0f;

    det =  A1*B2 - A2*B1;
    
    vre_assert ( det != 0 );
    
    C1 = A1*x1+B1*y1;
    C2 = A2*x2+B2*y2;
    
    pIntersection->x = ( vre_fix16) ((( B2 * C1 - B1 * C2 ) / det ) * 65536);
    pIntersection->y = ( vre_fix16) ((( A1 * C2 - A2 * C1 ) / det ) * 65536);
}





