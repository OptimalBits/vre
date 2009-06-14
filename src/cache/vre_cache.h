/*
 *  vre_cache.h
 *  vre_test
 *
 *  Created by Manuel  Astudillo on 2006-06-08.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_defs.h"

typedef struct vre_cache vre_cache;

vre_result vre_cache_create ( vre_cache **ppCache, vre_uint32 num_entries );

void vre_cache_destroy ( vre_cache *pCache );

void vre_cache_query ( vre_cache *pCache, vre_uint32 id, void **ppData );

void vre_cache_insert ( vre_cache *pCache, vre_uint32 id, 
                        void *pDataIn, 
                        void  **ppDataOut);

void vre_cache_begin_iter ( vre_cache *pCache );
vre_result vre_cache_iter ( vre_cache *pCache, void **ppData );


