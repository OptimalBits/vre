
#include "vre_ttf_max_profile.h"
#include "vre_iostream.h"
#include "vre_mem.h"

vre_result vre_read_max_profile_table ( vre_iostream *pStream,
                                        ttf_max_profile_table **ppTable )
{
    vre_result vres = VRE_ERR_OK;
    ttf_max_profile_table *pTable;

    pTable = (ttf_max_profile_table*) 
             vre_malloc ( sizeof (ttf_max_profile_table) );
    
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    *ppTable = pTable;
    
    for (;;) 
    {
        vres = vre_iostream_read_dword ( pStream, 
                                         (vre_uint32*)&pTable->table_version );
        VRE_BREAK_IF_ERR ( vres );
    
        vres = vre_iostream_read_word ( pStream, &pTable->num_glyphs );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_points );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_contours );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_composite_points);
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_zones );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_twilight_points );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_storage );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_function_defs );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_instruction_defs);
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_stack_elements );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, 
                                        &pTable->max_size_of_instructions );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, 
                                        &pTable->max_component_elements );
        VRE_BREAK_IF_ERR ( vres );

        vres = vre_iostream_read_word ( pStream, &pTable->max_component_depth );
        VRE_BREAK_IF_ERR ( vres );
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTable );
        *ppTable = 0;
    }
    
    return vres;   

}

