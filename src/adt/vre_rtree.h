/*
 *  vre_rtree.h
 *
 *  Created by Manuel  Astudillo on 2006-07-10.
 *  Copyright 2006 Polygoniq AB. All rights reserved.
 *
 */

#ifndef VRE_RTREE_H
#define VRE_RTREE_H

typedef struct vre_rtree vre_rtree;

#include "vre_defs.h"
#include "vre_rectangle.h"
#include "vre_list.h"
#include "vre_render.h"


typedef void (*VRE_RTREE_NODE_DESTROY) ( void *pData );


/**
    Creates an RTree object.
 
    @params ppTree holder for the created RTree.
    @params node_destroy function pointer to a function capable of
            destroying the node data.
    
    @return appropiate error code.
 
 */
vre_result vre_rtree_create ( vre_rtree **ppTree,
                              VRE_RTREE_NODE_DESTROY node_destroy );


/**
    Destroys an RTree object.
 
 */
vre_result vre_rtree_destroy ( vre_rtree *pTree );

/**
    Searchs in the RTree for all the objects intersecting the given 
    rectangle. The returned list does not preserve the order of the elements
    in any way.
 
    @params pTree pointer to RTree object.
    @params pRect pointer to rectangle object.
    @params pOutlist list containing intersecting elements.
 
    @return appropiate error code.
 
 */
vre_result vre_rtree_search ( vre_rtree *pTree, 
                              vre_rectangle *pRect, 
                              vre_list *pOutList );

/**
    Inserts an element in the RTree. The element must be accompanied of
    a bounding box rectangle.
 
    @params pTree pointer to RTree object.
    @params pRect rectangle holding the bounding box of the input object.
    @params pData data to be stored in the RTree.
 
    @return appropiate error code.
 */
vre_result vre_rtree_insert ( vre_rtree *pTree, 
                              vre_rectangle *pRect,
                              void *pData );


#ifdef VRE_DEBUG
void vre_rtree_print ( vre_rtree *pTree );
void vre_rtree_render_boxes ( vre_rtree *pTree, vre_render *pRender, vre_tile *pTile );
#endif


#endif






