/*
 *  vre_rtree.c
 *  
 *
 *  Created by Manuel Astudillo on 2006-07-17.
 *  Copyright 2006 Polygoniq AB. All rights reserved.
 *
 */

#include "vre_rtree.h"
#include "vre_list.h"
#include "vre_rectangle.h"
#include "vre_math.h"
#include "vre_mem.h"
#include "vre_assert.h"

#define VRE_RTREE_NUM_SLOTS 2

typedef struct vre_rtree_node vre_rtree_node;

struct vre_rtree_node
{
    vre_rtree_node *pParent;
    vre_rectangle  *pRects;
    void           **ppData;
    vre_uint32      num_entries;
    vre_uint32      is_leaf; 
    vre_uint32      parent_index;
};

struct vre_rtree
{
    vre_rtree_node *pRoot;
    vre_rectangle  rect;
    
    vre_list *pNodeStack;
    vre_list *pTraverseStack;
    VRE_RTREE_NODE_DESTROY node_free;
};

static vre_result 
create_node (  vre_rtree_node **ppNode );

static void 
destroy_node ( vre_rtree_node *pNode );

static void 
add_element_to_node ( vre_rtree_node *pNode, vre_rectangle *pRect, void *pData );

static void
add_child_to_node ( vre_rtree_node *pNode, 
                    vre_rectangle *pRect, 
                    vre_rtree_node *pChild );

static void 
node_union_rectangles ( vre_rtree_node *pNode, vre_rectangle *pRectOut );


vre_result vre_rtree_create ( vre_rtree **ppTree,
                              VRE_RTREE_NODE_DESTROY node_free )
{
    vre_result  vres;
    vre_rtree *pTree;
    
    *ppTree = 0;
    
    pTree = VRE_TALLOC ( vre_rtree );
    
    if ( pTree == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_memset (pTree, sizeof ( vre_rtree ), 0);

    for (;;)
    {
        pTree->node_free = node_free;
        
        vres = vre_list_create ( &pTree->pNodeStack );
        VRE_BREAK_IF_ERR (vres);
    
        vres = vre_list_create ( &pTree->pTraverseStack );
        VRE_BREAK_IF_ERR (vres);   
        break;
    }

    if ( vres != VRE_ERR_OK )
    {
        vre_rtree_destroy ( pTree );
        pTree = 0;
    }
    
    *ppTree = pTree;
    
    return vres;
}

// Redesign so that no errors can occour during destruction ( it will inevitable
// involve a memory leak.
vre_result vre_rtree_destroy ( vre_rtree *pTree )
{
    vre_result vres;
    
    vre_rtree_node *pNode;
    
    if ( pTree != 0 )
    {    
        vre_uint32 i;
        VRE_RTREE_NODE_DESTROY node_free;
        
        pNode = pTree->pRoot;
        
        node_free = pTree->node_free;
    
        vres = vre_list_put_first( pTree->pNodeStack, pNode );
        VRE_RETURN_IF_ERR (vres);
    
        while ( pNode != 0 )
        {
            for ( i = 0; i < pNode->num_entries; i++ )
            {
                vre_rtree_node *pChild;
                
                if ( ! pNode->is_leaf )
                {
                    pChild = pNode->ppData[i];
                
                    vres = vre_list_put_first( pTree->pNodeStack, pChild );
                    VRE_RETURN_IF_ERR (vres);
                
                    vres = vre_list_put_first( pTree->pTraverseStack, pChild );
                    VRE_RETURN_IF_ERR (vres);
                }
                else
                {
                    if ( node_free != 0 )
                    {
                        node_free (pNode->ppData[i]);
                    }
                }
            }
        
            pNode = (vre_rtree_node *) 
                    vre_list_get_first ( pTree->pTraverseStack );
        }
    
        for ( i = vre_list_num_elems ( pTree->pNodeStack ); i != 0; i-- )
        {
            pNode = (vre_rtree_node *) 
                    vre_list_get_first ( pTree->pNodeStack );
    
            destroy_node( pNode );
        }
    
        vre_list_destroy( pTree->pNodeStack );
        vre_list_destroy( pTree->pTraverseStack );
        
        vre_free( pTree );
    }
    
    return VRE_ERR_OK;
}


vre_result vre_rtree_search ( vre_rtree *pTree, 
                              vre_rectangle *pRect, 
                              vre_list *pOutList )
{
    vre_result vres;

    vre_rtree_node *pNode;
    vre_uint32     index;
    vre_list       *pNodeStack;
    
    vre_assert ( pOutList );   
    vre_assert ( pTree );
    vre_assert ( pRect );

    pNode = pTree->pRoot;
    pNodeStack = pTree->pNodeStack;

    if ( vre_rectangle_are_intersecting ( pRect, &pTree->rect ) )
    {
        while ( pNode != 0 ) 
        {
            index = 0;
            while ( index < pNode->num_entries )
            {
                vre_rectangle *pNodeRect = &pNode->pRects[index];
                if ( vre_rectangle_are_intersecting ( pRect, pNodeRect ) )
                {
                    if ( pNode->is_leaf )
                    {
                        vres = vre_list_put_first ( pOutList, pNode->ppData[index] );                    
                        VRE_RETURN_IF_ERR (vres);
                    }
                    else
                    {
                        vres = vre_list_put_first( pNodeStack, pNode->ppData[index] );
                        VRE_RETURN_IF_ERR (vres);
                    }
                }
                index ++;
            }

            pNode =  (vre_rtree_node *) vre_list_get_first ( pNodeStack );
        }
    }

    return VRE_ERR_OK;
}

//-----------------------------------------------------------------------------
/**
    Create Node
*/
static vre_result create_node ( vre_rtree_node **ppNode )
{
    for (;;)
    {
        *ppNode = VRE_TALLOC ( vre_rtree_node );
        
        if ( *ppNode == 0 )
        {
            break;
        }
        
        (*ppNode)->ppData = vre_malloc ( sizeof ( void*) * 
                                         (VRE_RTREE_NUM_SLOTS + 1));
        if ( (*ppNode)->ppData == 0 )
        {
            break;
        }
    
        (*ppNode)->pRects = vre_malloc ( sizeof (vre_rectangle) * 
                                         (VRE_RTREE_NUM_SLOTS + 1));
        
        if ( (*ppNode)->pRects == 0 )
        {
            break;
        }
        
        (*ppNode)->is_leaf = 1;
        (*ppNode)->num_entries = 0;
        (*ppNode)->pParent = 0;
        (*ppNode)->parent_index = 0;

        return VRE_ERR_OK;
    }
    
    destroy_node ( *ppNode );
    return VRE_ERR_MEM_ALLOC_FAILED;
}

/**
    Destroy Node
 */
static void destroy_node ( vre_rtree_node *pNode )
{
    if ( pNode != 0 )
    {
        vre_free ( pNode->ppData );
        vre_free ( pNode->pRects );
        vre_free ( pNode );
    }
}

/**
    Choose a leaf where to perform the insertion.

*/
static vre_rtree_node *choose_leaf ( vre_rtree_node *pNode, 
                                     vre_rectangle *pRect )
{
    vre_rectangle rect;
    vre_uint32 min_enlargement;
    vre_uint32 i;
    vre_uint32 index = 0;
    
    vre_assert ( pNode != 0 );
    vre_assert ( pNode != 0 );
    
    if ( pNode->is_leaf )
    {
        return pNode;
    }
    
    while ( !pNode->is_leaf )
    {
        //
        // Choose node that requires less enlarment area
        //
        min_enlargement = 0xffffffff;
        for ( i = 0; i < pNode->num_entries; i++ )
        {
            vre_uint32 enlargement_area, area_rect1, area_rect2;
            vre_rectangle_union ( pRect, &pNode->pRects[i], &rect );
    
            area_rect1 = vre_rectangle_area ( &rect );
            area_rect2 = vre_rectangle_area ( &pNode->pRects[i] );
            
            vre_assert ( area_rect1 >= area_rect2 );
            
            enlargement_area = area_rect1 - area_rect2;
                       
            if ( enlargement_area < min_enlargement )
            {
                min_enlargement = enlargement_area;
                index = i;
            }
        }
        
        pNode = pNode->ppData[index];
        
        vre_assert ( pNode != 0 );
    }
    
    return pNode;
}


// TODO: implement some good splitting algo.
static vre_result 
split_node ( vre_rtree_node *pNode, 
             vre_rtree_node **ppSplitedNode )
{
    vre_result vres;
    vre_uint32 i;
   
    vre_rtree_node *pSplitedNode;
    vre_uint32 num_entries;
    
    vre_assert ( pNode );
    vre_assert ( ppSplitedNode );
    
    vres = create_node ( ppSplitedNode );
    VRE_RETURN_IF_ERR ( vres );

    num_entries = pNode->num_entries;
    
    pSplitedNode = *ppSplitedNode;
    
    // Split into 2 groups
    // very loosy split
    for ( i = num_entries/2; i < num_entries; i++ )
    {
        add_element_to_node (pSplitedNode, &pNode->pRects[i], pNode->ppData[i]);
        
        pNode->num_entries --;
    }
    
    pSplitedNode->is_leaf = pNode->is_leaf;
    
    return VRE_ERR_OK;
}


/**

 Adjust tree so that all the rectangles are up-to-date.
 
 */
static vre_result 
adjust_tree ( vre_rtree *pTree, 
              vre_rtree_node *pLeaf, 
              vre_rtree_node *pSplitedNode )
{
    vre_result      vres = VRE_ERR_OK;
    vre_rtree_node  *pNode;
    vre_rtree_node  *pParent;
    vre_uint32      index;
    vre_rtree_node *pSecondSplit = 0;
    
    vre_assert ( pLeaf != 0 );
    vre_assert ( pLeaf->is_leaf );
        
    pNode = pLeaf;
    pParent = pNode->pParent;
    
    while ( pParent != 0 )
    {
        vre_rectangle rect;
        
        index = pNode->parent_index;
        node_union_rectangles ( pNode, &rect ); 
        
        vre_rectangle_copy ( &pParent->pRects[index], &rect );
        
        if ( pSplitedNode != 0 )
        {
            vre_uint32 num_entries = pParent->num_entries;
            
            vre_assert ( num_entries <= VRE_RTREE_NUM_SLOTS );

            node_union_rectangles ( pSplitedNode, &rect );
            add_child_to_node (pParent, &rect, pSplitedNode );

            if ( num_entries >= VRE_RTREE_NUM_SLOTS )
            {
                //
                // Not enough room for splited node in parent. Split again!.
                //
                vres = split_node ( pParent, &pSecondSplit );
                VRE_RETURN_IF_ERR ( vres );
            }
            else
            {
                pSecondSplit = 0;
            }
            
            pSplitedNode = pSecondSplit;
        }
        
        pNode = pParent;
        pParent = pNode->pParent;
    }
    
    //
    // Grow tree taller if root became splited.
    // ( create a new root, and add two splited nodes to it )
    //
    
    if ( pSplitedNode != 0 )
    {
        vre_rectangle rect;
        vre_rtree_node *pRoot;
        
        vres = create_node ( &pRoot );
        if ( vres != VRE_ERR_OK )
        {
            destroy_node ( pSecondSplit );
            return vres;
        }
        
        pRoot->is_leaf = 0;
            
        node_union_rectangles ( pNode, &rect );
        add_child_to_node ( pRoot, &rect, pNode );
        
        node_union_rectangles ( pSplitedNode, &rect );
        add_child_to_node ( pRoot, &rect, pSplitedNode ); // update 
        
        pTree->pRoot = pRoot;
    }
    
    return VRE_ERR_OK;
}

                                    
vre_result 
vre_rtree_insert ( vre_rtree *pTree, vre_rectangle *pRect, void *pData )
{
    vre_result vres = VRE_ERR_OK;
    vre_rtree_node *pLeaf = 0;
    vre_rtree_node *pLL = 0;
    
    vre_assert ( pTree != 0 );
    vre_assert ( pRect != 0 );
    vre_assert ( pData != 0 );
    
    if ( pTree->pRoot ) 
    {
        pLeaf = choose_leaf (  pTree->pRoot, pRect );
    
        vre_assert ( pLeaf->num_entries <= VRE_RTREE_NUM_SLOTS );
    
        add_element_to_node ( pLeaf, pRect, pData );
    
        if ( pLeaf->num_entries > VRE_RTREE_NUM_SLOTS )
        {
            vres = split_node ( pLeaf, &pLL );
            VRE_RETURN_IF_ERR ( vres );
        }
    
        vres = adjust_tree ( pTree, pLeaf, pLL );
        VRE_RETURN_IF_ERR ( vres );
    }
    else
    {
        vres = create_node ( &pTree->pRoot );
        VRE_RETURN_IF_ERR ( vres );
        
        add_element_to_node ( pTree->pRoot, pRect, pData );
    }
     
    node_union_rectangles (pTree->pRoot, &pTree->rect );

    return VRE_ERR_OK;
}

//-----------------------------------------------------------------------------
//
// Static functions
//
//-----------------------------------------------------------------------------
static void 
add_element_to_node ( vre_rtree_node *pNode, 
                      vre_rectangle *pRect, 
                      void *pData )
{
    vre_assert ( pNode != 0 );
    vre_assert ( pRect != 0 );
    vre_assert ( pNode->num_entries <= VRE_RTREE_NUM_SLOTS );
    
    vre_rectangle_copy ( &pNode->pRects[pNode->num_entries], pRect );

    pNode->ppData[pNode->num_entries] = (void*) pData;
    pNode->num_entries ++;
}


static void
add_child_to_node ( vre_rtree_node *pNode, 
                    vre_rectangle *pRect, 
                    vre_rtree_node *pChild )
{
    vre_assert ( pNode );
    vre_assert ( pRect );
    vre_assert ( pChild );
    
    pChild->parent_index = pNode->num_entries;
    add_element_to_node ( pNode, pRect, pChild );
    pChild->pParent = pNode;
}



static void 
node_union_rectangles ( vre_rtree_node *pNode, vre_rectangle *pRectOut )
{
    vre_uint32 i;
    
    vre_assert ( pNode != 0 );
    vre_assert ( pRectOut != 0 );

    if ( pNode->num_entries > 0 )
    {
        pRectOut->x1 = pNode->pRects[0].x1;
        pRectOut->x2 = pNode->pRects[0].x2;
        pRectOut->y1 = pNode->pRects[0].y1;
        pRectOut->y2 = pNode->pRects[0].y2;
        
        for ( i = 1; i < pNode->num_entries; i++ )
        {
            vre_rectangle_union ( &pNode->pRects[i], pRectOut, pRectOut );
        }
    }
    else
    {
        pRectOut->x1 = 0;
        pRectOut->x2 = 0;
        pRectOut->y1 = 0;
        pRectOut->y2 = 0;        
    }
}

#ifdef VRE_DEBUG
#include <stdio.h>
#include <vre_box.h>

static void print_nodes_recur ( vre_rtree_node *pNode, vre_uint32 depth );
static void render_nodes_recur ( vre_rtree_node *pNode,
                                vre_render *pRender,
                                vre_tile *pTile,
                                vre_mat3x3 *pMat,
                                vre_uint32 depth );

void vre_rtree_print ( vre_rtree *pTree )
{
    vre_rtree_node *pNode;
    
    pNode = pTree->pRoot;
    
    print_nodes_recur ( pNode, 0 );
}

static const vre_uint32 color_lut[16] = 
{ 
  0x80000000, 0x80000080, 0x80008080, 0x80800080, 0x80000000, 0x80000000, 
  0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000,
  0x80000000, 0x80000000, 0x80000000, 0x80000000 
};

void vre_rtree_render_boxes ( vre_rtree *pTree, vre_render *pRender, vre_tile *pTile )
{
    vre_rtree_node *pNode;
    vre_mat3x3 mat;
    
    pNode = pTree->pRoot;
    
    vre_matrix_identity(&mat);
    
    render_nodes_recur ( pNode, pRender, pTile, &mat, 0 );
}

static void print_nodes_recur ( vre_rtree_node *pNode, vre_uint32 depth )
{
    vre_rtree_node **pNodeList;
    vre_rectangle *pRects;
    vre_rectangle *pRect;
    vre_uint32 num_entries;
    vre_uint32 i, j;
    
    if ( ( pNode ) )
    {
        num_entries = pNode->num_entries;
        
        pNodeList = (vre_rtree_node**) pNode->ppData;
        pRects = pNode->pRects;
        
        for ( i = 0; i < num_entries; i++ )
        {
            pRect = &pRects[i];
            
            for ( j = 0; j < depth; j++ )
            {
                printf("  ");
            }
            
            printf ("Node rect(%f, %f)-(%f,%f)\n", 
                    pRect->x1/65536.0f, pRect->y1/65536.0f,
                    pRect->x2/65536.0f, pRect->y2/65536.0f );
            
            if ( !(pNode->is_leaf) )
            {
                print_nodes_recur ( pNodeList[i], depth+1 );
            }
        }
    }
}

static void render_nodes_recur ( vre_rtree_node *pNode,
                                 vre_render *pRender,
                                 vre_tile *pTile,
                                 vre_mat3x3 *pMat,
                                 vre_uint32 depth )
{
    vre_rtree_node **pNodeList;
    vre_rectangle *pRects;
    vre_rectangle *pRect;
    vre_uint32 num_entries;
    vre_uint32 i;
    
    if ( ( pNode ) )
    {
        num_entries = pNode->num_entries;
        
        pNodeList = (vre_rtree_node**) pNode->ppData;
        pRects = pNode->pRects;
        
        for ( i = 0; i < num_entries; i++ )
        {
            vre_box *pBox;
            vre_style style;
            
            pRect = &pRects[i];
            
            //
            // Get color depending on depth
            //
            
            style.fill_type = VRE_FILL_TYPE_NONE;
            style.stroke_type = VRE_STROKE_TYPE_SOLID;
            style.stroke_color = color_lut[depth];
            style.stroke_width = 1<<16;
              
            //
            // Render box
            //
            
            pRect->x = pRect->x1;
            pRect->y = pRect->y1;
            pRect->w = pRect->x2 - pRect->x1;
            pRect->h = pRect->y2 - pRect->y1;
            
            (void) vre_box_create( &pBox, &style, pRect );
            (void) vre_box_render( pBox, pRender, pMat, pTile );

            vre_box_destroy ( pBox );
            
            if ( !(pNode->is_leaf) )
            {
                render_nodes_recur ( pNodeList[i], pRender, pTile, pMat, depth+1 );
            }
        }
    }
}

#endif





