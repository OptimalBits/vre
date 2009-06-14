

#ifndef VRE_TTF_METRIC_H
#define VRE_TTF_METRIC_H

#include "vre_defs.h"
#include "vre_ttf_max_profile.h"
#include "vre_iostream.h"

/**
    Horizontal and Vertical font metrics.
*/

typedef struct ttf_hhea_table
{
vre_uint32  version;
vre_int16 	ascender;
vre_int16 	descender; 	
vre_int16	line_gap; 	
vre_uint16 	advance_width_max;
vre_int16	min_lsb;
vre_int16 	min_rsb;      // Min(aw - lsb - (xMax - xMin)).
vre_int16	x_max_extent; // Max(lsb + (xMax - xMin)).
vre_int16	caret_slope_rise; // Used to calculate the slope of the cursor (rise/run); 1 for vertical.
vre_int16 	caret_slope_run;  // 	0 for vertical.
vre_int16	caret_offset; // best appearance. Set to 0 for non-slanted fonts
vre_int16 	metric_data_format; // 0 for current format.
vre_uint16 	num_metrics; // Number of hMetric entries in 'hmtx' table

} ttf_hhea_table;


typedef struct ttf_hmetrics
{
    vre_uint16 advance_width;
    vre_int16  lsb;
} ttf_hmetrics;

typedef struct ttf_htmx_table
{
    ttf_hmetrics   *pHMetrics;
    vre_int16      *pLsbs;
    ttf_hhea_table *pHhea;
}ttf_htmx_table;

vre_result vre_read_hhea_table ( vre_iostream *pStream, 
                                 ttf_hhea_table **ppHeader );

vre_result vre_read_htmx_table ( ttf_hhea_table *pHhea,
                                 ttf_max_profile_table *pMaxP,
                                 vre_iostream *pStream, 
                                 ttf_htmx_table **ppTable );




#endif
