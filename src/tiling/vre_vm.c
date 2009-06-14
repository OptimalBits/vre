
#include "vre_vm.h"
#include "vre_matrix.h"
#include "vre_polygon.h"
#include "vre_mem.h"

#define MAX_NUM_OPCODES 256

typedef struct vre_vm_cmd
{
    vre_opcode  opcode;
    void        *data;
} vre_vm_cmd;


struct vre_vm
{
    vre_vm_cmd  *pCmds;
    vre_int     num_cmds;
    
    vre_list    *pMatrixStack;
    vre_mat3x3  *pCurMatrix;
};

vre_result vre_vm_create ( vre_vm **ppVm )
{
    vre_result res;
    vre_vm *pVm;
    
    pVm = VRE_TALLOC ( vre_vm );
    
    if ( pVm == 0 ) 
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pVm->pCmds = ( vre_vm_cmd *) vre_malloc ( sizeof ( vre_vm_cmd) *
                                              MAX_NUM_OPCODES );
    
    if ( pVm->pCmds == 0 )
    {
        vre_free ( pVm );   
    }
    
    pVm->num_cmds = 0;
                    
    res = vre_list_create ( &pVm->pMatrixStack );
    
    if ( res != VRE_ERR_OK )
    {
        vre_free ( pVm->pCmds );
        vre_free ( pVm );
        return res;
    }
       
    
    *ppVm = pVm;
    
    return VRE_ERR_OK;        
}


void vre_vm_destroy ( vre_vm *pVm )
{
    if ( pVm != 0 )
    {
        
        vre_free ( pVm->pCmds );
        vre_list_destroy ( pVm->pMatrixStack );
        vre_free ( pVm );        
    }
}

vre_result vre_vm_add_cmd ( vre_vm *pVm, vre_opcode op, void *data )
{
    vre_mat3x3  *pMat;
    vre_result  res = VRE_ERR_OK;
    
    vre_int num_cmds = pVm->num_cmds;

    pVm->pCmds[num_cmds].opcode = op;
    
    switch ( op )
    {
        case VRE_OP_PUSH_MATRIX: 
            res = vre_matrix_clone ( &pMat, (vre_mat3x3*) data );
            pVm->pCmds[num_cmds].data =  ( void* ) pMat;
            break;
        case VRE_OP_POP_MATRIX:
            pVm->pCmds[num_cmds].data = 0;
            break;
        case VRE_OP_ROTATE:
            {
                pVm->pCmds[num_cmds].data =  ( void* ) data;
            }
            break;
        case VRE_OP_SCALE:
            {
                vre_point *scale = VRE_TALLOC ( vre_point );
                scale->x = (( vre_point*) data)->x;
                scale->y = (( vre_point*) data)->y;
                pVm->pCmds[num_cmds].data =  ( void* ) scale;
            }
            break;
        case VRE_OP_TRANSLATE:
            {
                pVm->pCmds[num_cmds].data =  data;
            }            
            break;
        case VRE_OP_SKEW:
            {
                vre_point *skew = VRE_TALLOC ( vre_point );
                skew->x = (( vre_point*) data)->x;
                skew->y = (( vre_point*) data)->y;
                pVm->pCmds[num_cmds].data =  ( void* ) skew;
            }
            break;
        case VRE_OP_MULT_MATRIX:
            res = vre_matrix_clone ( (vre_mat3x3*) data, &pMat );
            pVm->pCmds[num_cmds].data =  ( void* ) pMat;
            break;
    
        // Style operations
        case VRE_OP_SET_FG_COLOR:
        case VRE_OP_SET_BG_COLOR:
        case VRE_OP_SET_STROKE_WIDTH:
        case VRE_OP_SET_STROKE_COLOR:
        case VRE_OP_SET_FILL:

        // Primitives
        case VRE_OP_POLYGON:
        {          
            pVm->pCmds[num_cmds].data = data;
            break;        
        }
        break;
        case VRE_OP_CIRCLE:
        case VRE_OP_ARC:
        case VRE_OP_ELIPSE:
        case VRE_OP_CUBIC_BEZIER:
        case VRE_OP_SPLINE:
        break;
    }
    
    if ( res != VRE_ERR_OK )
    {
        return res;
    }    

    pVm->num_cmds ++;
    
    return VRE_ERR_OK;
}


vre_result vre_vm_execute ( vre_vm *pVm, vre_list *pPolyList )
{
    vre_int i;
    vre_result  res;
    
    vre_vm_cmd  *pCmds = pVm->pCmds;
    
    vre_mat3x3  pCurMatrix;
    
    vre_matrix_identity ( &pCurMatrix );
    
    for ( i = 0; i < pVm->num_cmds; i++ )
    {        
        switch ( pCmds[i].opcode )
        {
            case VRE_OP_PUSH_MATRIX:
                {
                    vre_mat3x3 *pMat = 
                        ( vre_mat3x3*) vre_malloc ( sizeof ( vre_mat3x3 ) );
                    
                    vre_matrix_copy ( pMat, &pCurMatrix );
                    vre_list_put_first ( pVm->pMatrixStack, 
                                         (void*) &pCurMatrix );
                                        
                }
                break;
            case VRE_OP_POP_MATRIX:
                {
                    vre_mat3x3 *pMat;
                    
                    pMat = vre_list_get_first ( pVm->pMatrixStack );
                                                
                    vre_matrix_copy ( pVm->pCurMatrix, pMat );
                    vre_free ( pMat );                    
                }
                break;
            case VRE_OP_ROTATE:
                vre_matrix_rotate ( &pCurMatrix, (vre_fix16) pCmds[i].data );
                break;
            
            case VRE_OP_SCALE:
                vre_matrix_scale ( &pCurMatrix, 
                                   ((vre_point*) pCmds[i].data)->x,
                                   ((vre_point*) pCmds[i].data)->y );
                break;
            case VRE_OP_TRANSLATE:
                vre_matrix_translate ( &pCurMatrix, 
                                      ((vre_point*) pCmds[i].data)->x,
                                      ((vre_point*) pCmds[i].data)->y );
                break;

            case VRE_OP_POLYGON:
                {
                    vre_polygon *pPolygon;
                    
                    pPolygon =  (vre_polygon*) pCmds[i].data;
                             
                    // Transfornm polygon
                    vre_polygon_transform ( pPolygon,
                                            &pCurMatrix );

                    // Add to grip structure                                            
                    vre_list_put_first ( pPolyList, (void*) pPolygon );
                }
                break;               
        }
    }
}


