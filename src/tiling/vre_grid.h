

#ifndef VRE_GRID_H
#define VRE_GRID_H

typedef struct vre_grid vre_grid;

#include "vre_defs.h"
#include "vre_list.h"
#include "vre_polygon.h"

typedef enum vre_primitive_type
{
    VRE_POLYGON,
    VRE_POLYLINE,
    VRE_LINE,
    VRE_BOX,
    VRE_CIRCLE
} vre_primitive_type;


typedef struct vre_primitive
{
    vre_primitive_type  type;
    void    *data;
} vre_primitive;


struct vre_grid
{
    vre_list    *pPrimitivesList;
};





#endif
