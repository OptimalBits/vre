
#include "vre_rtree.h"



int main (void)
{
    vre_result vres;
    vre_rtree *pRTree;
    vre_rectangle rect;
    void *pNode;
    
    vres = vre_rtree_create ( &pRTree );
    
    rect.x1 = 100;
    rect.x2 = 200;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 1 );
    
    rect.x1 = 50;
    rect.x2 = 75;
    rect.y1 = 150;
    rect.y2 = 257;
    
    vre_rtree_insert ( pRTree, &rect, 1 );

    //    vre_rtree_print ( pRTree );
    
    rect.x1 = 300;
    rect.x2 = 360;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 2 );
    
    rect.x1 = 100;
    rect.x2 = 200;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 3 );
    
    rect.x1 = 100;
    rect.x2 = 200;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 4 );
    
    rect.x1 = 100;
    rect.x2 = 200;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 5 );
    
    rect.x1 = 100;
    rect.x2 = 200;
    rect.y1 = 150;
    rect.y2 = 500;
    
    vre_rtree_insert ( pRTree, &rect, 6 );
    vre_rtree_insert ( pRTree, &rect, 7 );
    
    rect.x1 = 50;
    rect.x2 = 100;
    rect.y1 = 50;
    rect.y2 = 75;
    
    vre_rtree_print ( pRTree );
    
 //   vre_rtree_start_search ( pRTree, &rect );
    
 //   vre_rtree_search ( pRTree, &pNode );
    
    vres = vre_rtree_destroy ( pRTree );
    
    return 0;
}


