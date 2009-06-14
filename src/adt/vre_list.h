/*
 *  vre_list.h
 *
 *  Created by Manuel  Astudillo on 2006-07-10.
 *  Copyright 2006 Polygoniq AB. All rights reserved.
 *
 */

#ifndef VRE_LIST_H
#define VRE_LIST_H

#include "vre_defs.h"

typedef struct vre_list vre_list;


/**
    Create a list object.
 
 */
vre_result vre_list_create ( vre_list **ppList );


/**
    Destroy a list object.
*/
void vre_list_destroy ( vre_list *pList );

/**
    Put an element last in the list.
 
 */
vre_result vre_list_put_last ( vre_list *pList, void *data );

/**
 Put an element first in the list.
 
 */
vre_result vre_list_put_first (vre_list *pList, void *data );

/**
  Put an element sorted in ascentent way according to some given key.
 
 */
vre_result vre_list_put_sorted ( vre_list *pList, void *pData, vre_uint32 key );

/**
 Get last element in the list ( removing it from the list ).
 
 */
void *vre_list_get_last (vre_list *pList);

/**
  Get first element in the list ( removing it from the list ).

 */
void *vre_list_get_first (vre_list *pList);


/**
 Peek some element in the list.
 
 */
void *vre_list_peek (vre_list *pList, vre_int pos);

/**
 Peek last element in the list.
 
 */
void *vre_list_peek_last (vre_list *pList);

/**
 Peek first element in the list.
 
 */
void *vre_list_peek_first (vre_list *pList);

/**
 Get the total number of elements in the list.
 
 */
vre_int vre_list_num_elems (vre_list *pList);


#endif
