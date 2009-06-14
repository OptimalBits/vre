
#include <math.h>
#include "vre_math.h"
#include "vre_defs.h"
#include "vre_assert.h"

vre_fix16 vre_pow ( vre_fix16 a, vre_fix16 b )
{
    double x = a / 65536.0;
    double y = b / 65536.0;
    
    double p = pow ( x, y );

    return (vre_fix16) (p * 65536);
}


#define iter_sqrt(n, root, delta, N)   \
(delta) = ( (root) + (1 << (N)) ); \
if ( (n) >= ( (delta) << (N)) )    \
{   (n) -= ( (delta) << (N) );     \
    (root) |= ( 2 << (N) );        \
}                                  

vre_uint32 vre_sqrt(vre_uint32 n)
{
    vre_uint32 root  = 0;
    vre_uint32 delta = 0;
    
    iter_sqrt(n, root, delta, 15);
    iter_sqrt(n, root, delta, 14);    
    iter_sqrt(n, root, delta, 13);
    iter_sqrt(n, root, delta, 12);
    iter_sqrt(n, root, delta, 11);
    iter_sqrt(n, root, delta, 10);    
    iter_sqrt(n, root, delta,  9);
    iter_sqrt(n, root, delta,  8);
    iter_sqrt(n, root, delta,  7);
    iter_sqrt(n, root, delta,  6);    
    iter_sqrt(n, root, delta,  5);
    iter_sqrt(n, root, delta,  4);
    iter_sqrt(n, root, delta,  3);
    iter_sqrt(n, root, delta,  2);    
    iter_sqrt(n, root, delta,  1);
    iter_sqrt(n, root, delta,  0);
    
    root <<= 7;
    
    return root;
}

vre_fix16 vre_sin (vre_fix16 a)
{
    vre_int32 px = 1 << 16;
    vre_int32 py = 0;
    
    vre_cordic_rotate( &px, &py, (a<<12)  );
    
    return py;
}

vre_fix16 vre_cos (vre_fix16 a)
{
    vre_int32 px = 1 << 16;
    vre_int32 py = 0;
    
    vre_cordic_rotate( &px, &py, (a<<12) );
    
    return px;
}

void vre_sin_cos (vre_fix16 *pSin, vre_fix16 *pCos, vre_fix16 a)
{    
    *pCos = 1 << 16;
    *pSin = 0;
    vre_cordic_rotate( pCos, pSin, a<<12 );
}

vre_fix16 vre_arccos (vre_fix16 a)
{
    return (vre_ufix16) (acos ( a / 65536.0f)*65536);
}

/*
vre_uint32 vre_most_significant_bit ( vre_uint32 x )
{
    // Binary search
    if ( 0xff )
    
}
*/

vre_fix16 vre_atan2 ( vre_fix16 x, vre_fix16 y )
{
    float xf, yf;
    float result;
    
    xf = x / 65536.0f;
    yf = y / 65536.0f;
    
    result = ( atan2 ( xf, yf ) );
    
    if ( result < 0.0f ) result += 2*3.1416;
    
    return result * 65536;
    /*
    float xf, yf;
    float pi = 3.1416;
    float coeff_1 = pi/4;
    float coeff_2 = 3*coeff_1;
    float abs_y;
    float r;
    float angle;
    
    xf = x / 65536.0f;
    yf = y / 65536.0f;
    
    abs_y = fabs(yf)+1e-10;     // kludge to prevent 0/0 condition
    if (xf>=0)
    {
        r = (xf - abs_y) / (xf + abs_y);
        angle = coeff_1 - coeff_1 * r;
    }
    else
    {
        r = (xf + abs_y) / (abs_y - xf);
        angle = coeff_2 - coeff_1 * r;
    }
    if (y < 0)
        return (vre_fix16) (-angle * 65536.0f);     // negate if in quad III or IV
    else
        return (vre_fix16) (angle * 65536.0f);
*/
}


//
// CORDIC For trigonometric functions. (2:30)
// Borrowed from Ken Turkowski
//
//#define COSCALE ((vre_int32)(0.6072529350088813 * (1<<30)))   //0x22C2DD1C // 0.271572  
#define COSCALE ((vre_int32)(0.2715717684432241 * (1<<30)))
#define QUARTER ((vre_int32)(3.141592654 / 2.0 *(1 << 28))) 

static const vre_uint32 arctantab[32] = 
{   // MS 4 integral bits for radians  
    297197971, 210828714, 124459457, 65760959, 33381290, 16755422, 8385879, 
    4193963, 2097109, 1048571, 524287, 262144, 131072, 65536, 32768, 16384, 
    8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1, 0, 0, 
}; 

void vre_cordic_rotate(vre_fix16 *px, vre_fix16 *py, vre_fix16 theta) 
{ 
    vre_uint32 i; 
    vre_fix16  x = *px;
    vre_fix16  y = *py;
    vre_fix16  xtemp; 
    const vre_uint32 *pArctan = arctantab;
    vre_uint32 change_sign;
    
    // vre_assert ( theta >= 0 );
    // vre_assert ( theta <= 2*VRE_PI_4_28 );
    
    // Valid range -pi <= theta <= pi
    if ( theta > VRE_PI_4_28/2 )
    {
        theta = VRE_PI_4_28 - theta;
        change_sign = 1;
    }
    else if ( theta < -VRE_PI_4_28/2 )
    {
        theta = - (VRE_PI_4_28 + theta);
        change_sign = 1;
    }
    else
    {
        change_sign = 0;
    }
    
    if (theta < 0) 
    { 
        xtemp = x + (y << 1); // Left shift when i = -1 
        y     = y - (x << 1); 
        x = xtemp; 
        theta += *pArctan++; 
    } 
    else 
    { 
        xtemp = x - (y << 1); 
        y     = y + (x << 1); 
        x = xtemp; 
        theta -= *pArctan++; 
    }
    
    
    for (i = 0; i < 30; i++) 
    { 
        if (theta < 0) 
        { 
            xtemp = x + (y >> i); 
            y     = y - (x >> i); 
            x = xtemp; 
            theta += *pArctan++; 
        } 
        else 
        { 
            xtemp = x - (y >> i); 
            y     = y + (x >> i); 
            x = xtemp; 
            theta -= *pArctan++; 
        } 
    } 
    *px = FPMULSH(x, COSCALE, 30); // Compensate for CORDIC enlargement  
    *py = FPMULSH(y, COSCALE, 30); // FPMULSH(a,b)=(a*b)>>31, high part of 64-bit product  

    if ( change_sign == 1 )
    {
        *px = -*px;
    }
}


