    

#include "vre_defs.h"

/**
    Canvas structure.
    
    Only argb32 modes are supported.
*/
typedef struct vre_canvas 
{    
    vre_uint32  width;
    vre_uint32  height;
    vre_uint32  scanline_width;
    vre_uint32  *pData;
    vre_uint    color_mode; 
} vre_canvas;
    
