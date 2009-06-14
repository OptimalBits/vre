/*
 *  vre_container.h
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-17.
 *  Copyright 2008 . All rights reserved.
 *
 */

#ifndef VRE_CONTAINER_H
#define VRE_CONTAINER_H

#include "vre_defs.h"
#include "vre_iostream.h"

vre_result vre_container_get_chunk_id ( vre_iostream *pStream, vre_uint32 *pId );

vre_result vre_container_skip_chunk ( vre_iostream *pStream );

#endif
