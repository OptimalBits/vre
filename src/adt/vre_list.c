
#include "vre_list.h"
#include "vre_mem.h"
#include "vre_assert.h"

typedef struct vre_list_node vre_list_node;

// TODO: optimize to use only one pointer per node.
// Use this: http://www.aggregate.org/MAGIC/#Dual-Linked%20List%20with%20One%20Pointer%20Field

struct vre_list_node
{
    vre_list_node   *pNext;
    vre_list_node   *pPrev;
    void            *pData;
    vre_uint32      key;    // Key to be used for sorting purposes.
};


struct vre_list
{   
    vre_list_node   *pFirst;
    vre_list_node   *pLast;
    vre_int32       num_nodes;
};

vre_result vre_list_create ( vre_list **ppList )
{
    vre_list    *pList;
    
    vre_assert ( ppList );

    pList = VRE_TALLOC ( vre_list );
    
    if ( pList == 0 )
    {
        *ppList = 0;
        return VRE_ERR_MEM_ALLOC_FAILED;   
    }

    pList->pFirst = 0;
    pList->pLast = 0;

    pList->num_nodes = 0;
    
    *ppList = pList;
	
	return VRE_ERR_OK;
}

void vre_list_destroy ( vre_list *pList )
{
    if ( pList != 0 )
    {
        while ( pList->num_nodes > 0 )
        {
            (void) vre_list_get_first ( pList );
        }
  
        vre_free ( pList );
    }
}

vre_result vre_list_put_last ( vre_list *pList, void *data )
{
    vre_list_node *pNode;
    
    vre_assert ( pList );
     
    pNode = VRE_TALLOC ( vre_list_node );
    if ( pNode == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }

    pNode->pNext = 0;
    pNode->pPrev = pList->pLast;
    pNode->pData = data;

    if ( pList->num_nodes != 0 )
    {                       
        pList->pLast->pNext = pNode;
    }
    else 
    {
        pList->pFirst = pNode;
    }
    
    pList->pLast = pNode;
    pList->num_nodes ++;
    
    return VRE_ERR_OK;
}

vre_result vre_list_put_first (vre_list *pList, void *data )
{
    vre_list_node *pNode;
    
    vre_assert ( pList );
     
    pNode = VRE_TALLOC ( vre_list_node );
    if ( pNode == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }

    pNode->pNext = pList->pFirst;
    pNode->pPrev = 0;
    pNode->pData = data;

    if ( pList->num_nodes != 0 )
    {               
        pList->pFirst->pPrev = pNode;
    }
    else 
    {
        pList->pLast = pNode;
    }
    
    pList->pFirst = pNode;
    
    pList->num_nodes ++;
    
    return VRE_ERR_OK;
}

static void insert_node ( vre_list *pList,
                          vre_list_node *pNodePrev, 
                          vre_list_node *pNode )
{
    vre_list_node *pNodeNext;

    vre_assert ( pNode );

    if ( pNodePrev != 0 )
    {
        pNodeNext = pNodePrev->pNext;

        pNode->pNext = pNodeNext;
        pNode->pPrev = pNodePrev;

        pNodePrev->pNext = pNode;
    }
    else
    {
        pNodeNext = pList->pFirst;
        pNode->pNext = pNodeNext;
        pList->pFirst = pNode;
        pNode->pPrev = 0;
        pNodeNext->pPrev = pNode;
    }

     if ( pNodeNext == 0 )
     {
        pList->pLast = pNode;
     }

     pList->num_nodes ++;
}

static vre_list_node *find_node_key ( vre_list *pList, vre_uint32 key )
{
    vre_list_node *pNode;
    vre_list_node *pNodePrev;

    vre_assert ( pList );

    pNodePrev = 0;
    pNode = pList->pFirst;

    while ( pNode != 0 )
    {
        if ( pNode->key >= key )
        {
            return pNodePrev;
        }

        pNodePrev = pNode;
        pNode = pNode->pNext;
    }

    return pNodePrev;
}


vre_result vre_list_put_sorted ( vre_list *pList, 
                                 void *pData, 
                                 vre_uint32 key )
{
    vre_list_node *pNode;

    pNode = VRE_TALLOC ( vre_list_node );
    if ( pNode == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }

     pNode->pData = pData;
     pNode->key = key;

    if ( pList->num_nodes > 0 )
    {
        vre_list_node *pInsertNode;

        pInsertNode = find_node_key ( pList, key );

        insert_node ( pList, pInsertNode, pNode );
    }
    else
    {
        pList->pFirst = pNode;
        pList->pLast  = pNode;
        pNode->pNext = 0;
        pNode->pPrev = 0;
        pList->num_nodes ++;
    }

    return VRE_ERR_OK;
}


void *vre_list_get_last (vre_list *pList)
{
    vre_list_node   *pNode;
    void *data = 0;
    
    vre_assert ( pList );
    
    if ( pList->num_nodes > 0 )
    {
        vre_assert ( pList->pLast );
        
        pNode = pList->pLast;
        
        data = pNode->pData;
    
        pList->pLast = pNode->pPrev;
        
        if ( pList->pLast )
        {
            pList->pLast->pNext = 0;
        }
        
        pList->num_nodes--;
    
        vre_free ( pNode );
        
        return data;
    }
    else
    {
        return 0;
    }
}

void *vre_list_get_first (vre_list *pList)
{
    vre_list_node   *pNode;
    void *data = 0;
    
    vre_assert ( pList );
    
    if ( pList->num_nodes > 0 )
    {
        vre_assert ( pList->pFirst );
        
        pNode = pList->pFirst;
        
        data = pNode->pData;
    
        pList->pFirst = pNode->pNext;
        
        if ( pList->pFirst ) 
        {
            pList->pFirst->pPrev = 0;
        }
        
        pList->num_nodes--;
        
        vre_free ( pNode );
        
        return data;
    }
    else
    {
        return 0;
    }
    
}

void *vre_list_peek (vre_list *pList, vre_int pos)
{
    vre_list_node   *pNode;
    vre_int count = 0;
    
    vre_assert ( pList );
    
    pNode = pList->pFirst;
    while ( pNode != 0 )
    {
        if ( count == pos )
        {
            return pNode->pData;
        }    
        pNode = pNode->pNext;
        count ++;
    }
    
    return 0;    
}

void *vre_list_peek_last (vre_list *pList)
{
    void *data = 0;
    
    vre_assert ( pList );
    
    data = 0;
   
    if ( pList->pLast != 0 )
    {
        data = pList->pLast->pData;
    }
    
    return data;
}

void *vre_list_peek_first (vre_list *pList)
{
    void *data = 0;
    
    vre_assert ( pList );
    
    data = 0;
    
    if ( pList->pFirst != 0 )
    {
        data = pList->pFirst->pData;
    }
    
    return data;
}

vre_int vre_list_num_elems (vre_list *pList)
{
    vre_assert ( pList );
    
    return pList->num_nodes;
}
