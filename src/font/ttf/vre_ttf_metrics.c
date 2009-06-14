
#include "vre_ttf_metrics.h"
#include "vre_ttf_max_profile.h"
#include "vre_defs.h"
#include "vre_ttf.h"
#include "vre_mem.h"
#include "vre_assert.h"

vre_result vre_read_hhea_table ( vre_iostream *pStream, 
                                 ttf_hhea_table **ppHeader )
{
    vre_result vres;
    ttf_hhea_table *pHeader;
    
    *ppHeader = 0;
    
    pHeader = (ttf_hhea_table *) vre_malloc ( sizeof ( ttf_hhea_table ) );
    if ( pHeader == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;    
    }
    
    for (;;)
    {
        vres = vre_iostream_read_dword ( pStream, &pHeader->version );
        VRE_BREAK_IF_ERR ( vres ); 
    
        vres = vre_iostream_read_sword ( pStream, &pHeader->ascender );
        VRE_BREAK_IF_ERR ( vres ); 

        vres = vre_iostream_read_sword ( pStream, &pHeader->descender );
        VRE_BREAK_IF_ERR ( vres ); 

        vres = vre_iostream_read_sword ( pStream, &pHeader->line_gap );
        VRE_BREAK_IF_ERR ( vres ); 

        vres = vre_iostream_read_word ( pStream, &pHeader->advance_width_max);
        VRE_BREAK_IF_ERR ( vres );        
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->min_lsb );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->min_rsb );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->x_max_extent );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->caret_slope_rise );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->caret_slope_run );
        VRE_BREAK_IF_ERR ( vres ); 
    
        vres = vre_iostream_read_sword ( pStream, &pHeader->caret_offset );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_skip ( pStream, 8 );
        VRE_BREAK_IF_ERR ( vres ); 
        
        vres = vre_iostream_read_sword ( pStream, &pHeader->metric_data_format );
        VRE_BREAK_IF_ERR ( vres ); 
    
        vres = vre_iostream_read_word ( pStream, &pHeader->num_metrics );
        VRE_BREAK_IF_ERR ( vres ); 
    
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pHeader );
        return vres;
    }
    
    *ppHeader = pHeader;
    
    return vres;
}


vre_result vre_read_htmx_table ( ttf_hhea_table *pHhea,
                                 ttf_max_profile_table *pMaxP,
                                 vre_iostream *pStream, 
                                 ttf_htmx_table **ppTable )
{
    vre_result vres = VRE_ERR_OK;
    ttf_htmx_table *pTable;
    vre_uint32  num_lsbs;
    
    vre_assert ( pStream != 0 );

    if ( pHhea == 0 )
    {
        return VRE_ERR_FILE_FORMAT_CORRUPT;
    }
    
    if ( pMaxP == 0 )
    {
        return VRE_ERR_FILE_FORMAT_CORRUPT;
    }
    
    pTable = (ttf_htmx_table *) vre_malloc ( sizeof (ttf_htmx_table) );
    if ( pTable == 0 )
    {
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    pTable->pHMetrics = (ttf_hmetrics *) vre_malloc ( sizeof (ttf_hmetrics) * 
                                                      pHhea->num_metrics);
    if ( pTable->pHMetrics == 0 )
    {
        vre_free ( pTable );
        return VRE_ERR_MEM_ALLOC_FAILED;
    }
    
    num_lsbs = pMaxP->num_glyphs - pHhea->num_metrics;
    
    if ( num_lsbs > 0 )
    {
        pTable->pLsbs = (vre_int16*) vre_malloc ( sizeof ( vre_int16 ) * num_lsbs );
        if ( pTable->pLsbs == 0 )
        {
            vre_free ( pTable );
            vre_free ( pTable->pHMetrics );                    
        }
    }
    else
    {
        pTable->pLsbs = 0;
    }
    
    for (;;)
    {            
        vre_uint32 i;
        
        for ( i = 0; i < pHhea->num_metrics; i++ )
        {
            ttf_hmetrics *pMetrics = pTable->pHMetrics;
        
            vres = vre_iostream_read_word ( pStream, &pMetrics[i].advance_width);
            VRE_BREAK_IF_ERR ( vres );

            vres = vre_iostream_read_sword ( pStream, &pMetrics[i].lsb );
            VRE_BREAK_IF_ERR ( vres );
        }
        
        for ( i = 0; i < num_lsbs; i++ )
        {
            vres = vre_iostream_read_sword ( pStream, &pTable->pLsbs[i] );
            VRE_BREAK_IF_ERR ( vres );           
        }
        
        break;
    }

    if ( vres != VRE_ERR_OK )
    {
        vre_free ( pTable->pHMetrics );
        vre_free ( pTable->pLsbs );
        vre_free ( pTable );
    }
    
    pTable->pHhea = pHhea;
    
    *ppTable = pTable;
    
    return vres;
}

vre_int32 vre_ttf_get_advance_width ( vre_ttf *pTtf, 
                                      vre_uint16 index )
{
    ttf_htmx_table *pTable;
    vre_int32 advance_width;

    vre_assert ( pTtf != 0);
    vre_assert ( pTtf->pHtmx != 0);
    vre_assert ( pTtf->pHhea != 0);
    
    pTable = pTtf->pHtmx;
    
    if ( index < pTtf->pHhea->num_metrics )
    {
        advance_width = pTable->pHMetrics[index].advance_width;
    }
    else
    {
        advance_width = pTable->pHMetrics[pTable->pHhea->num_metrics-1].advance_width;
    }

    return advance_width;
}




