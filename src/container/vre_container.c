/*
 *  vre_container.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-17.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "vre_container.h"


vre_result vre_container_get_chunk_id ( vre_iostream *pStream, vre_uint32 *pId )
{
    vre_result vres;
    
    vre_iostream_set_endian ( pStream, VRE_BIG_ENDIAN );
    
    vres = vre_iostream_read_dword( pStream, pId );
    
    vre_iostream_set_endian ( pStream, VRE_LITTLE_ENDIAN );

    VRE_RETURN_IF_ERR ( vres );
    
    return VRE_ERR_OK;
}


vre_result vre_container_skip_chunk ( vre_iostream *pStream )
{
    vre_result vres;
    vre_uint32 length;
    
    //
    // Read chunk length
    //
    
    vres = vre_iostream_read_dword( pStream, &length );
    VRE_RETURN_IF_ERR ( vres );
    
    //
    // Skip chunk data ( including subchunks )
    //
    
    vres = vre_iostream_skip( pStream, length );
    VRE_RETURN_IF_ERR ( vres );
    
    return VRE_ERR_OK;
}
