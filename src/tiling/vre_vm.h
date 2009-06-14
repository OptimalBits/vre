

#ifndef VRE_VM_H
#define VRE_VM_H

typedef struct vre_vm vre_vm;

#include "vre_defs.h"
#include "vre_list.h"

/**
    vre virtual machine.
    The machine is executed everytime rendering is required.
    It performs transformation and tesselation of all the primitives.

    The execution of the machine will produce a quadtree with all the
    polygons to be rendered. 
*/

// fwd declarations


typedef enum vre_opcode
{
    // Matrix operations
    VRE_OP_PUSH_MATRIX,
    VRE_OP_POP_MATRIX,
    VRE_OP_ROTATE,
    VRE_OP_SCALE,
    VRE_OP_TRANSLATE,
    VRE_OP_SKEW,
    VRE_OP_MULT_MATRIX,
    
    // Style operations
    VRE_OP_SET_FG_COLOR,
    VRE_OP_SET_BG_COLOR,
    VRE_OP_SET_STROKE_WIDTH,
    VRE_OP_SET_STROKE_COLOR,
    VRE_OP_SET_FILL,

    // Primitives
    VRE_OP_POLYGON,
    VRE_OP_POLYLINE,
    VRE_OP_CIRCLE,
    VRE_OP_ARC,
    VRE_OP_ELIPSE,
    VRE_OP_CUBIC_BEZIER,
    VRE_OP_SPLINE
    
} vre_opcode;

/*
typedef struct vre_vm_atom 
{
    void *pPrimitive;
    vre_mat3x3 *pMat;
    vre_draw_func *pDrawFunc;
    vre_bbox_func *pBboxFunc;
} vre_vm_atom;
*/

vre_result vre_vm_create ( vre_vm **ppVm );

void vre_vm_destroy ( vre_vm *pVm );

vre_result vre_vm_add_cmd ( vre_vm *pVm, vre_opcode op, void *data );

vre_result vre_vm_execute ( vre_vm *pVm, vre_list *pPolyList );

// interface
/*
vre_result vre_draw_primitive ( void *pPrimitive, vre_mat3x3 *pMat );
vre_result vre_get_bounding_box ( void *pPrimitive, vre_mat3x3 *pMat );
*/


#endif



