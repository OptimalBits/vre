/*
 *  vre_bin.c
 *  vre_sdltest
 *
 *  Created by Manuel Astudillo on 2008-07-17.
 *  Copyright 2008 Vectech. All rights reserved.
 *
 *  TODO: Real group support, it is necessary for animation and bone support.
 *
 */

#include "vre_bin.h"
#include "vre_container.h"
#include "vre_path.h"
#include "vre_style.h"
#include "vre_iostream.h"
#include "vre_array.h"
#include "vre_mem.h"

typedef struct 
{
    vre_style style;
    vre_mat3x3 mat;
    vre_array *pPaths;
} vre_group;

struct vre_bin
{
    vre_array *pGroups;
};

//
// Static forward declarations
//

static vre_result create_group ( vre_group **ppGroup );
static void destroy_group ( vre_group *pGroup );

static vre_result parse_paint_attributes ( vre_iostream *pStream, vre_style *pStyle );

static vre_result parse_path ( vre_iostream *pStream, 
                              vre_group *pGroup,
                              vre_path **ppPath );

static vre_result parse_move_cmd ( vre_iostream *pStream, 
                                  vre_uint32 num_coords, 
                                  vre_path *pPath );

static vre_result parse_line_cmd ( vre_iostream *pStream, 
                                  vre_uint32 num_coords, 
                                  vre_path *pPath );

static vre_result parse_cubic_cmd ( vre_iostream *pStream, 
                                   vre_uint32 num_coords, 
                                   vre_path *pPath );

vre_result vre_bin_load ( vre_iostream *pStream, vre_bin **ppBin )
{
    vre_result vres;
    vre_uint32 id;
    
    vre_bin *pBin;
    
    vre_group *pGroup = 0;
    
    vres = vre_bin_create ( ppBin );
    VRE_RETURN_IF_ERR ( vres );
    
    pBin = *ppBin;
    
    //
    // Parse chunks
    //
    
    vres = vre_container_get_chunk_id ( pStream, &id );
    VRE_RETURN_IF_ERR ( vres );
        
    while ( id != 'CEOF' )
    {
        switch ( id )
        {
            case 'CGRP':
                create_group ( &pGroup );
                vres = vre_array_add(pBin->pGroups, &pGroup, sizeof (vre_group*));
                VRE_BREAK_IF_ERR ( vres );
                {
                    vre_uint32 dummy_length;
                    vres = vre_iostream_read_dword ( pStream, &dummy_length);
                }
                VRE_BREAK_IF_ERR ( vres );
                break;
            
            case 'PATH':
            {
                vre_path *pPath;
                vres = parse_path ( pStream, pGroup, &pPath );
                VRE_BREAK_IF_ERR ( vres );
                
                vres = vre_array_add ( pGroup->pPaths, &pPath, sizeof(vre_path*) );
                VRE_BREAK_IF_ERR ( vres );
            }
                break;
            
            case 'PNTA':
                vres = parse_paint_attributes ( pStream, &(pGroup->style) );
                VRE_BREAK_IF_ERR ( vres );
                
                break;
            default:
                vres = vre_container_skip_chunk (pStream);
                VRE_BREAK_IF_ERR ( vres );
        }
        VRE_BREAK_IF_ERR ( vres );
        
        vres = vre_container_get_chunk_id ( pStream, &id );
        VRE_RETURN_IF_ERR ( vres );
    }

    if ( vres != VRE_ERR_OK )
    {
        vre_bin_destroy ( pBin );
    }

    return VRE_ERR_OK;
}


vre_result vre_bin_create ( vre_bin **ppBin )
{
    vre_result vres = VRE_ERR_OK;
    
    vre_bin *pBin;
    
    *ppBin = VRE_TALLOC ( vre_bin );
    if ( *ppBin == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pBin = *ppBin;

    vres = vre_array_create( sizeof (vre_group*), &(pBin->pGroups) );
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pBin );
    }
    
    return vres;
}


void vre_bin_destroy ( vre_bin *pBin )
{
    if ( pBin )
    {
        //
        // Iter all groups and free all paths
        //
    }
}

vre_result vre_bin_render ( vre_bin *pBin, 
                            vre_render *pRender, 
                            vre_mat3x3 *pMat, 
                            vre_tile *pTile )
{
    
    vre_uint32 num_groups;
    vre_uint32 num_paths;
    vre_group *pGroup;
    vre_uint32 *dummy_ptr; // hack until array gets refactored.
    
    // Urgent TODO: Refactor array in order to
    vre_array_get_elems_start( pBin->pGroups, (void**) (&dummy_ptr), &num_groups );
   
    while (dummy_ptr )
    {
        vre_path *pPath;
        
        pGroup = (vre_group*) *dummy_ptr;
        
        vre_array_get_elems_start ( pGroup->pPaths, (void**)(&dummy_ptr), &num_paths );
        
        while ( dummy_ptr )
        {
            pPath = (vre_path*)*dummy_ptr;
            
            if ( pPath )
            {
                vre_path_render(pPath, pRender, pMat, pTile );
            }
                
            vre_array_get_elems_iter (pGroup->pPaths, (void**)(&dummy_ptr), &num_paths );
        }
        
        vre_array_get_elems_iter (pBin->pGroups,  (void**) (&dummy_ptr), &num_paths);
    }
    
    
    return VRE_ERR_OK;
}


/**
    Return an array of all paths objects created from the bin file content.
 
 */
vre_result vre_bin_get_paths ( vre_iostream *pStream, vre_array *pPaths )
{
    return VRE_ERR_OK;
}


/**
    Create a group of primitives.
 */
static vre_result create_group ( vre_group **ppGroup )
{
    vre_result vres;
    
    vre_group *pGroup;
    
    
    
    *ppGroup = VRE_TALLOC ( vre_group );
    if ( *ppGroup == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pGroup = *ppGroup;
    
    vres = vre_array_create ( sizeof ( vre_path* ), &pGroup->pPaths );
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pGroup );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    vre_style_init_defaults ( &pGroup->style );

    vre_matrix_identity( &pGroup->mat );
    
    return VRE_ERR_OK;
}

/**
    Destroy group.
 
 */
static void destroy_group ( vre_group *pGroup )
{
   if ( pGroup )
   {
       vre_array_destroy ( pGroup->pPaths );
       vre_free ( pGroup );
   }
}


/**
    Parse paint attributes.
 */

enum  
{
    PAINT_ATTR_FILL = 1,
    PAINT_ATTR_FILL_SOLID = 2,
    PAINT_ATTR_FILL_GRADIENT = 4,
    PAINT_ATTR_FILL_OPACITY = 8,
    PAINT_ATTR_STROKE = 256,
    PAINT_ATTR_STROKE_WIDTH = 512,
    PAINT_ATTR_STROKE_OPACITY = 1024
} VRE_BIN_PAINT_ATTR;

static vre_result parse_paint_attributes ( vre_iostream *pStream, vre_style *pStyle )
{
    vre_result vres;
    vre_int32 length;
    vre_uint32 attrs;
    
    vres = vre_iostream_read_dword( pStream, &length );
    VRE_RETURN_IF_ERR ( vres );
    
    if ( length < 4 )
    {
        return VRE_ERR_OK;
    }
    vres = vre_iostream_read_dword( pStream, &attrs );
    VRE_RETURN_IF_ERR ( vres );
    
    length -= 4;
    if ( length < 0 )
    {
        return VRE_ERR_FILE_FORMAT_CORRUPT;
    }
    
    if ( PAINT_ATTR_FILL & attrs )
    {
        vre_uint32 fill_color;
        vres = vre_iostream_read_dword( pStream, &fill_color );
        VRE_RETURN_IF_ERR ( vres );
        
        pStyle->fg_color = (0xff << 24) | fill_color;
        pStyle->fill_type = VRE_FILL_TYPE_SOLID;
        
        
        length -= 4;
        if ( length < 0 )
        {
            return VRE_ERR_FILE_FORMAT_CORRUPT;
        }
    }
    
    if ( PAINT_ATTR_FILL_OPACITY & attrs )
    {
        vre_uint32 opacity;
        vres = vre_iostream_read_dword( pStream, &opacity );
        VRE_RETURN_IF_ERR ( vres );
        
        length -= 4;
        if ( length < 0 )
        {
            return VRE_ERR_FILE_FORMAT_CORRUPT;
        }
    }
    
    if ( PAINT_ATTR_STROKE & attrs )
    {
        vre_uint32 stroke_color;
        vres = vre_iostream_read_dword( pStream, &stroke_color );
        VRE_RETURN_IF_ERR ( vres );
        
        pStyle->stroke_type =  VRE_STROKE_TYPE_SOLID;
        pStyle->stroke_color = (0xff << 24) | stroke_color;
        
        length -= 4;
        if ( length < 0 )
        {
            return VRE_ERR_FILE_FORMAT_CORRUPT;
        }
    }

    if ( PAINT_ATTR_STROKE_WIDTH & attrs )
    {
        vre_uint32 stroke_width;
        vres = vre_iostream_read_dword( pStream, &stroke_width );
        VRE_RETURN_IF_ERR ( vres );

        pStyle->stroke_width = stroke_width;
        
        length -= 4;
        if ( length < 0 )
        {
            return VRE_ERR_FILE_FORMAT_CORRUPT;
        }
     
    }    
    
    return VRE_ERR_OK;
}


/**
    Creates a path based on the parsed path data.
 
 */
static vre_result parse_path ( vre_iostream *pStream, 
                               vre_group *pGroup,
                               vre_path **ppPath )
{
    vre_result vres = VRE_ERR_OK;
    vre_int32 num_elements;
    vre_uint32 cmd_and_num_coords;
    vre_path *pPath;
    
    //
    // Create path
    //
    
    vres = vre_path_create( ppPath, &pGroup->style );
    VRE_RETURN_IF_ERR ( vres );
    
    pPath = *ppPath;
    
    for (;;)
    {
         vre_bool closed_path;
        
        //
        // Read chunk size
        //
        
        vres = vre_iostream_read_dword( pStream, &num_elements );
        VRE_BREAK_IF_ERR ( vres );
        
        num_elements /= 4;
        
        while ( num_elements != 0 )
        {
            vre_uint32 num_coords;
            vre_uint32 cmd;
           
            //
            // Get command
            //
            
            vres = vre_iostream_read_dword( pStream, &cmd_and_num_coords );
            VRE_BREAK_IF_ERR ( vres );
        
            num_coords = (cmd_and_num_coords >> 16) & 0xffff;
            cmd = cmd_and_num_coords & 0xfff;
            
            switch ( cmd )
            {
                case 'M': // TODO: Handle subpaths.
                    closed_path = VRE_FALSE;
                    vres = parse_move_cmd ( pStream, num_coords, pPath );
                    VRE_BREAK_IF_ERR ( vres );
                    break;
                case 'L':
                    vres = parse_line_cmd ( pStream, num_coords, pPath );
                    VRE_BREAK_IF_ERR ( vres );
                    break;
                case 'C':
                    vres = parse_cubic_cmd ( pStream, num_coords, pPath );
                    VRE_BREAK_IF_ERR ( vres );
                    break;
                case 'z':
                    vres = vre_path_close(pPath);
                    closed_path = VRE_TRUE;
                    break;
            }
            
            VRE_BREAK_IF_ERR ( vres );
            
            num_elements -= num_coords * 2 + 1;
            if ( num_elements < 0 )
            {
                vres = VRE_ERR_FILE_FORMAT_CORRUPT;
                break;
            }
        }
        
        if ( closed_path == VRE_FALSE )
        {
            vre_path_end ( pPath );
        }
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_path_destroy(pPath);
    }
    
    return vres;
}

static vre_result parse_move_cmd ( vre_iostream *pStream, 
                                   vre_uint32 num_coords, 
                                   vre_path *pPath )
{
    vre_result vres;
    
    if ( num_coords > 0 ) 
    {
        vre_fix16 x, y;
    
        vres = vre_iostream_read_dword( pStream, (vre_uint32*) &x );
        VRE_RETURN_IF_ERR ( vres );
    
        vres = vre_iostream_read_dword( pStream, (vre_uint32*) &y );
        VRE_RETURN_IF_ERR ( vres );
    
        vres = vre_path_add_move_to( pPath, x,y );
        VRE_RETURN_IF_ERR ( vres );
    
        num_coords --;
    
        while ( num_coords != 0 )
        {
            vres = vre_iostream_read_dword( pStream, (vre_uint32*) &x );
            VRE_RETURN_IF_ERR ( vres );
        
            vres = vre_iostream_read_dword( pStream, (vre_uint32*) &y );
            VRE_RETURN_IF_ERR ( vres );
        
            vres = vre_path_add_line( pPath, VRE_FALSE, x,y );
            VRE_RETURN_IF_ERR ( vres );
        
            num_coords --;
        }
        
        return VRE_ERR_OK;
    }
    else
    {
       return VRE_ERR_FILE_FORMAT_CORRUPT;
    }
}

static vre_result parse_line_cmd ( vre_iostream *pStream, 
                                   vre_uint32 num_coords, 
                                   vre_path *pPath )
{
    vre_result vres;
    vre_fix16 x, y;
    
    while ( num_coords != 0 )
    {
        vres = vre_iostream_read_dword( pStream, (vre_uint32*) &x );
        VRE_RETURN_IF_ERR ( vres );
        
        vres = vre_iostream_read_dword( pStream, (vre_uint32*) &y );
        VRE_RETURN_IF_ERR ( vres );
        
        vres = vre_path_add_line( pPath, VRE_FALSE, x,y );
        VRE_RETURN_IF_ERR ( vres );
        
        num_coords --;
    }
    
    return VRE_ERR_OK;
}


static vre_result parse_cubic_cmd ( vre_iostream *pStream, 
                                    vre_uint32 num_coords, 
                                    vre_path *pPath )
{
    vre_result vres;
    
    if ( ( num_coords >= 3 ) && !( num_coords % 3 ) ) 
    {
        vre_fix16 coords[6];
        
        while ( num_coords != 0 )
        {
            vres = vre_iostream_read_dwords( pStream, (vre_uint32*) coords, 6 );
            VRE_RETURN_IF_ERR ( vres );
            
            vres = vre_path_add_cubic ( pPath, VRE_FALSE, ( vre_point*) coords );
            VRE_RETURN_IF_ERR ( vres );
            
            num_coords -= 3;
        }
        return VRE_ERR_OK;
    }
    else
    {
        return VRE_ERR_FILE_FORMAT_CORRUPT;
    }
}


