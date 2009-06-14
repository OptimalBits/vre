

#include "vre_bezier.h"
#include "vre_mem.h"
#include "vre_vector.h"
#include "vre_math.h"
#include "vre_assert.h"


struct vre_bezier
{  
    vre_point   *pPoints;   // Control Points.
    vre_point   *pTPoints;  // Transformed Points. ( Obsolete ).
    vre_uint32  num_points;
    
    VRE_BEZIER_MAP_FUNC pMapFunc;
    void *pUser;
};

static vre_result vre_bezier_flatten2 ( vre_bezier *pBezier, 
                                        vre_polygon *pPolygon,
                                        vre_fix16 max_deviation,
                                        vre_bool include_last_point);

static vre_result vre_bezier_flatten3 ( vre_bezier *pBezier, 
                                        vre_polygon *pPolygon,
                                        vre_fix16 max_deviation,
                                        vre_bool include_last_point);
                                  
vre_result vre_bezier_create ( vre_bezier **ppBezier,                               
                               vre_uint32 num_points )
{    
    vre_result vres = VRE_ERR_OK;
    
    vre_bezier *pBezier;
    
    vre_assert ( num_points > 2 );
    
    for (;;)
    {
        pBezier = VRE_TALLOC ( vre_bezier );
        
        if ( pBezier == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }        
        
        pBezier->pPoints = vre_malloc ( sizeof(vre_point) * num_points );
        if ( pBezier->pPoints == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
        
        pBezier->pTPoints = vre_malloc ( sizeof(vre_point) * num_points );
        if ( pBezier->pTPoints == 0 )
        {
            vres = VRE_ERR_MEM_ALLOC_FAILED;
            break;
        }
                                       
        pBezier->num_points = num_points;
        
        break;
    }
    
    if ( vres != VRE_ERR_OK )
    {
        *ppBezier = 0;
    }
    else
    {
        *ppBezier = pBezier;
    }
    
   return vres;    
}


void vre_bezier_destroy ( vre_bezier *pBezier )
{
    vre_assert ( pBezier );
    
    vre_free ( pBezier->pTPoints );
    vre_free ( pBezier->pPoints );
    vre_free ( pBezier );

}

void vre_bezier_set_points ( vre_bezier *pBezier,
                             vre_point *pPoints,
                             vre_uint32 num_points )
{
    vre_assert ( pBezier->num_points == num_points );

    vre_memcopy8 ( pBezier->pPoints, pPoints, 
                   sizeof(vre_point) * num_points);                             
}

void vre_bezier_set_map_func ( vre_bezier *pBezier, 
                               VRE_BEZIER_MAP_FUNC pFunc, 
                               void *pUser )
{
    pBezier->pMapFunc = pFunc;
    pBezier->pUser = pUser;
}

// TODO: an optimized eval for cubic bezier
void vre_bezier_eval ( vre_bezier *pBezier, 
                       vre_fix16 t,
                       vre_point *pOutPoint )
{
	vre_uint64 t0, t1, fact, n_over_i, a;
	vre_int32 i, n;
	
	vre_assert ( ( t >= 0 ) && ( t <= 65536 ) );
	vre_assert ( pBezier->num_points > 0 );

    // degree
    n = pBezier->num_points - 1;
    t0 = t;
	t1 = 65536 - t0;
	
	// Horner-Bezier algorithm
	pOutPoint->x = (pBezier->pPoints[0].x * t1 + 32768) >> 16;
    pOutPoint->y = (pBezier->pPoints[0].y * t1 + 32768) >> 16;

  	fact = 65536;	
	n_over_i = 65536;
	i = 1;
	while ( i < n )
	{
		fact = (fact * t0 + 32768) >> 16;
		n_over_i = (n_over_i * (n - i + 1)) / i;
		
		a = (fact * n_over_i + 32768) >> 16;
		
		pOutPoint->x = ((pOutPoint->x + ((a * pBezier->pPoints[i].x+32768) >> 16)) * t1+32768) >> 16;		 
		pOutPoint->y = ((pOutPoint->y + ((a * pBezier->pPoints[i].y+32768) >> 16)) * t1+32768) >> 16;
				  
		i++;
	};

	pOutPoint->x = 
	   pOutPoint->x + ((((fact * t0+32768)>>16) * pBezier->pPoints[i].x+32768)>>16);
	pOutPoint->y = 
	   pOutPoint->y + ((((fact * t0+32768)>>16) * pBezier->pPoints[i].y+32768)>>16);

}                    
                       
                       
/**
    @param pPolygon polygon where the resulted flatten bezier will be put.
           This polygon is assumed to be big enought to hold the data.
           (TODO: polygons should be able to grow if required ).
*/
vre_result vre_bezier_flatten ( vre_bezier *pBezier, 
                                vre_polygon *pPolygon, 
                                vre_fix16 max_deviation,
                                vre_bool include_last_point)
{
    if (  pBezier->num_points == 3  )
    {
        // Flatten a quadric bezier
        return vre_bezier_flatten2 (pBezier, pPolygon, 
                                    max_deviation, 
                                    include_last_point);
    }
    else if ( pBezier->num_points == 4 )
    {
        // Flatten a cubic bezier
        return vre_bezier_flatten3 (pBezier, pPolygon, 
                                    max_deviation, 
                                    include_last_point);
    }
    
    return VRE_ERR_UNSUPORTED;
}

/**
    Flatten quadric bezier curves. 
    Based on amanith implementation: www.amanith.org
    
*/
static vre_result vre_bezier_flatten2 ( vre_bezier *pBezier, 
                                        vre_polygon *pPolygon,
                                        vre_fix16 max_deviation,
                                        vre_bool include_last_point)
{
    vre_point   tmp_bezier[3];
    vre_uint64  epsilon;
    vre_int64   t;

    vre_result  vres = VRE_ERR_OK;
    vre_assert  ( pBezier->num_points == 3 );
              
    vre_memcopy8 ( &tmp_bezier, pBezier->pPoints, sizeof(vre_point) * 3 );
     
    //epsilon = 2 * vre_pow (max_deviation, 65536 / 4 );
    epsilon = 2 * vre_sqrt (vre_sqrt (max_deviation));
    
    do
    {   
        vre_point   k;
        vre_ufix16  k_length;
                
        // Output point
        vres = vre_polygon_add_point( pPolygon, &tmp_bezier[0] ); // 
        VRE_RETURN_IF_ERR ( vres );
                   
        // Update squared chordal length
        k.x = 2 * tmp_bezier[1].x - tmp_bezier[0].x - tmp_bezier[2].x;                  
        k.y = 2 * tmp_bezier[1].y - tmp_bezier[0].y - tmp_bezier[2].y;
        
        // k_length = vre_vector_length( &k );
        k_length = vre_vector_length_apprx ( &k );
                  
        if ( k_length == 0 ) 
        {
            break;
        }                  
                  
        t = (epsilon<<16) / vre_sqrt( k_length );
        
        if ( t > 65536 )
        {
            break;
        }

        // Cut curve using De Casteljau algorithm
        tmp_bezier[0].x = 
           ( ((65536 - t) * tmp_bezier[0].x + t * tmp_bezier[1].x) + 32768)>> 16;
        tmp_bezier[0].y = 
           ( ((65536 - t) * tmp_bezier[0].y + t * tmp_bezier[1].y) + 32768)>> 16;

        tmp_bezier[1].x = 
           ( ((65536 - t) * tmp_bezier[1].x + t * tmp_bezier[2].x) + 32768)>> 16;
        tmp_bezier[1].y = 
           ( ((65536 - t) * tmp_bezier[1].y + t * tmp_bezier[2].y) + 32768)>> 16;
        
        tmp_bezier[0].x = 
           ( ((65536 - t) * tmp_bezier[0].x + t * tmp_bezier[1].x) + 32768)>> 16;
        tmp_bezier[0].y = 
           ( ((65535 - t) * tmp_bezier[0].y + t * tmp_bezier[1].y) + 32768)>> 16;
        
    } while ( t < 65536 ); 
    
    // Output last point (not always, for example in case of splines we do not want to do this!!)
    if ( include_last_point )
    {    
        return vre_polygon_add_point( pPolygon, &tmp_bezier[2] );
    }
    else
    {
        return vres;
    }        
        
}

/* 
==========================
 Cubic Bezier Drawing v1.1
 ==========================
 recursive quadratic approximation 
 with adjustable tolerance
 
 March 4, 2004
 
 Robert Penner
 www.robertpenner.com
 
	MovieClip.drawBezier()
	MovieClip.drawBezierPts()
	MovieClip.curveToCubic()
	MovieClip.curveToCubicPts()
 
 */

////////////////////////////////////////////////////
// override drawing methods to store pen location
//#include "drawing_api_core_extensions.as"

/////////////////////////////////////////////////////
/*
Math.intersect2Lines = function (p1, p2, p3, p4) 
 {
	var x1 = p1.x; var y1 = p1.y;
	var x4 = p4.x; var y4 = p4.y;
    
	var dx1 = p2.x - x1;
	var dx2 = p3.x - x4;
	if (!(dx1 || dx2)) return NaN;
	
	var m1 = (p2.y - y1) / dx1;
	var m2 = (p3.y - y4) / dx2;
	
	if (!dx1)
    {
		// infinity
		return { x:x1,
                 y:m2 * (x1 - x4) + y4 };
        
	} 
    else if (!dx2) 
    {
		// infinity
		return { x:x4,
                 y:m1 * (x4 - x1) + y1 };
	}
	var xInt = (-m2 * x4 + y4 + m1 * x1 - y1) / (m1 - m2);
	var yInt = m1 * (xInt - x1) + y1;
	return { x:xInt, y:yInt };
};

Math.midLine = function (a, b) 
{
	return { x:(a.x + b.x)/2, y:(a.y + b.y)/2 };
};

Math.bezierSplit = function (p0, p1, p2, p3) 
{
	var m = Math.midLine;
	var p01 = m (p0, p1);
	var p12 = m (p1, p2);
	var p23 = m (p2, p3);
	var p02 = m (p01, p12);
	var p13 = m (p12, p23);
	var p03 = m (p02, p13);
	return 
    {
            b0:{a:p0,  b:p01, c:p02, d:p03},
            b1:{a:p03, b:p13, c:p23, d:p3 }  
	};
};

var MCP = MovieClip.prototype;

MCP.$cBez = function (a, b, c, d, k) 
{
	// find intersection between bezier arms
	var s = Math.intersect2Lines (a, b, c, d);
	// find distance between the midpoints
	var dx = (a.x + d.x + s.x * 4 - (b.x + c.x) * 3) * .125;
	var dy = (a.y + d.y + s.y * 4 - (b.y + c.y) * 3) * .125;

	// split curve if the quadratic isn't close enough
	if (dx*dx + dy*dy > k) 
    {
		var halves = Math.bezierSplit (a, b, c, d);
		var b0 = halves.b0; var b1 = halves.b1;
		// recursive call to subdivide curve
		this.$cBez (a,     b0.b, b0.c, b0.d, k);
		this.$cBez (b1.a,  b1.b, b1.c, d,    k);
	} 
    else 
    {
		// end recursion by drawing quadratic bezier
		this.curveTo (s.x, s.y, d.x, d.y);
	}
};

MCP.drawBezierPts = function (p1, p2, p3, p4, tolerance) 
{
	if (tolerance == undefined) tolerance = 5;
	this.moveTo (p1.x, p1.y);
	this.$cBez (p1, p2, p3, p4, tolerance*tolerance);
};

MCP.drawBezier = function (x1, y1, x2, y2, x3, y3, x4, y4, tolerance) 
{
	this.drawBezierPts ({x:x1, y:y1},
						{x:x2, y:y2},
						{x:x3, y:y3},
						{x:x4, y:y4},
						tolerance);
};

MCP.curveToCubic = function (x1, y1, x2, y2, x3, y3, tolerance) 
{
	if (tolerance == undefined) tolerance = 5;
	this.$cBez (
                {x:this._xpen, y:this._ypen},
                {x:x1, y:y1},
                {x:x2, y:y2},
                {x:x3, y:y3},
                tolerance*tolerance );
};

MCP.curveToCubicPts = function (p1, p2, p3, tolerance) {
	if (tolerance == undefined) tolerance = 5;
	this.$cBez (
                {x:this._xpen, y:this._ypen},
                p1, p2, p3, tolerance*tolerance );
};

delete MCP;
*/





static vre_result vre_bezier_flatten3 ( vre_bezier *pBezier, 
                                        vre_polygon *pPolygon,
                                        vre_fix16 max_deviation,
                                        vre_bool include_last_point)
{
    vre_result  vres = VRE_ERR_OK;
    vre_fix16   t;
      
    for ( t = 0; t < 65536; t += max_deviation )
    {    
        vre_point point;

        vre_bezier_eval ( pBezier, t, &point );

        if ( pBezier->pMapFunc != 0 )
        {
            vres = pBezier->pMapFunc ( pBezier->pUser, point );
            VRE_BREAK_IF_ERR ( vres );
        }
        vres = vre_polygon_add_point( pPolygon, &point ); 
        VRE_BREAK_IF_ERR ( vres );
                                                    
    };
    
    return vres;
}

/*

void SplitBezier( Point     *bezPts,    // 4 points
                  double    t,
                  Point     *bLeft,     // 4 points 0-t
                  Point     *bRight )   // 4 points t-1.0
{
    int     i, r;

    for (i=0; i<=3; i++)
        bRight[i] = bezPtsData[i];

    for (r=1; r<=3; r++)
    {
        for (i=0; i<=3-r; i++)
        {
            bRight[i].x = (1.0-t)*bRight[i].x + t*bRight[i+1].x;
            bRight[i].y = (1.0-t)*bRight[i].y + t*bRight[i+1].y;
        }
    }

    for (i=0; i<=3; i++)
        bLeft[3-i] = bezPts[i];

    for (r=1; r<=3; r++)
    {
        for (i=0; i<=3-r; i++)
        {
            bLeft[i].x = t*bLeft[i].x + (1.0-t)*bLeft[i+1].x;
            bLeft[i].y = t*bLeft[i].y + (1.0-t)*bLeft[i+1].y;
        }
    } 
}

*/


// Investigate this algorithm...
/*
 Bezier Curve
 Elegant addition-only differencing method
 .
 WARNING: untested code.
 2002-04-13:DAV: Translated
 from lovely idiomatic PostScript
 to lowest-common-denominator C language.
 1998-02-14: Don Lancaster wrote Version 3.2 in PostScript
 and posted online at
 http://www.tinaja.com/text/bezgen3.html
 .
 
 How the method works:
 Given a point on the spline curve, the next
 point on the curve can be found
 by simple addition of running terms.
 
 The method is device and language independent,
 letting you generate
 a cubic spline **without** using the PostScript -curveto- operator!
 
 TODO: Is it more efficient to leave like this,
 breaking up a smooth Bezier curve into 30 or so straight line segments,
 or would it be cool to
 make this even more like the Bresenham circle algorithm,
 *only* doing putpixel() and directly sets the correct pixels
 without reference to a lineto() routine ?
 
 */

extern void moveto( int x, int y);

extern void lineto( int x, int y);

void
bezier(
       const int x0, const int y0, // start point
       const int x1, const int y1, // first control point
       const int x2, const int y2, // last control point
       const int x3, const int y3  // end point
       ){
    
    
	const int numincs = 100;
	int i = numincs;
    // % Plot the spline as a standard PostScript curveto...
    // x0 y0 moveto x1 y1 x2 y2 x3 y3 curveto line1 stroke
    
    // Find t-power coefficients a-d given control points. See
    //   HACK62.PDF http://www.tinaja.com/glib/hack62.pdf
    // for more details. This moves you from graph space to math space..
    
    const float ax = x3 - x2*3 + x1*3 - x0;
    const float ay = y3 - y2*3 + y1*3 - y0;
    
    const float bx = x2*3 - x1*6 + x0*3;
    const float by = y2*3 - y1*6 + y0*3;
    
    const float cx = x1*3 - x0*3;
    const float cy = y1*3 - y0*3;
    
    // /dx x0 def
    // /dy y0 def
    
    
    // Elegant addition-only differencing method
    
    
	// /numincs 100 def % number of steps to generate the spline
    
	// % next, precalculate delta-t increment, square, and cube
    
	const float dt1 = 1/(float)numincs; // delta t
	const float dt2 = dt1*dt1; // delta t squared
	const float dt3 = dt1*dt2; // delta t cubed
    
	// % initialize some stuff...
	float xx = (float)x0;
	float yy = (float)y0;
	float ux = 0;
	float uy = 0;
	float vx = 0;
	float vy = 0;
    
	float mx1 = ax * dt3;
	float my1 = ay * dt3;
    
	float lx1 = bx * dt2;
	float ly1 = by * dt2;
    
	float kx = mx1 + lx1 + cx * dt1;
	float ky = my1 + ly1 + cy * dt1;
    
	float mx = mx1 * 6;
	float my = my1 * 6;
    
	float lx = mx + 2*lx1;
	float ly = my + 2*ly1;
    
	// % now generate the fake curveto...
    
	//moveto( xx, yy ); // xx yy moveto
    
	for( i = numincs; i; i-- ){
		xx += ux + kx;
		yy += uy + ky;
		ux += vx + lx;
		uy += vy + ly;
		vx += mx;
		vy += my;
        
	//	lineto( xx, yy ); // xx yy lineto
        
	};
    
	/*
     Note that successive points along the curve require only *ten* repeated
     additions. There are no multiplies or other time-intensive operators.
     Note that this is **not** an approximation, but an exact solution!
     
     Try changing -numincs- to 3 to prove this to yourself.
     */
    
};

