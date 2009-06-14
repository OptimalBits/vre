
#ifndef VRE_DEFS_H
#define VRE_DEFS_H

#define VRE_LITTLE_ENDIAN   1
#define VRE_BIG_ENDIAN      2

#define VRE_FALSE	0
#define VRE_TRUE	1

#define VRE_DEBUG

//#define VRE_BIG_ENDIAN

#ifdef LITTLE_ENDIAN
#define VRE_PLATFORM_BYTE_ORDER VRE_LITTLE_ENDIAN
#endif

#ifdef BIG_ENDIAN
#define VRE_PLATFORM_BYTE_ORDER VRE_BIG_ENDIAN
#endif

// GCC and RVCT restrict
#define VRE_RESTRICT __restrict


//
// Basic Data types.
//

typedef unsigned long long vre_uint64;
typedef long long vre_int64;
typedef unsigned long vre_uint32;
typedef long vre_int32;
typedef unsigned short vre_uint16;
typedef short vre_int16;
typedef unsigned char vre_uint8;
typedef char vre_int8;
typedef unsigned int vre_uint;
typedef int vre_int;

typedef unsigned long vre_ufix16;
typedef long vre_fix16;

typedef unsigned long vre_ufix8;
typedef long vre_fix8;

#ifndef VRE_UNICODE
    typedef char vre_char;
#else
    typedef vre_int16 vre_char;
#endif

typedef int vre_bool;

typedef struct vre_point
{
    vre_fix16    x;
    vre_fix16    y;
} vre_point;

typedef struct vre_size
{
    vre_uint32  w;
    vre_uint32  h;
} vre_size;

typedef struct vre_color
{
    vre_uint32  a;
    vre_uint32  r;
    vre_uint32  g;
    vre_uint32  b;
} vre_color;

typedef struct vre_tile
{
    vre_int32  x;
    vre_int32  y;
    vre_int32  w;
    vre_int32  h;
    vre_uint32 *pData;
    vre_uint32 scanline_width;
} vre_tile;

/**
    Canvas structure.
    
    Only argb32 modes are supported.
*/
typedef struct vre_canvas 
{    
    vre_uint32  width;
    vre_uint32  height;
    vre_uint32  scanline_width;
    vre_uint32  *pData;
    vre_uint    color_mode; 
} vre_canvas;


typedef enum vre_result
{
    VRE_ERR_OK = 0,
    VRE_ERR_MEM_ALLOC_FAILED = -1,
    VRE_ERR_FILE_OPEN = -2,
    VRE_ERR_FILE_READ = -3,
    VRE_ERR_FILE_SEEK = -4,
    VRE_ERR_UNSUPORTED_FORMAT = -5,
    VRE_ERR_FILE_FORMAT_CORRUPT = -6,
    VRE_ERR_UNSUPORTED = -7
} vre_result;

//
// Usefull Macros
//

#define VRE_REVERSE_WORD( word ) (((word) >> 8) | ((word) << 8 )&0xff00)

#define VRE_REVERSE_DWORD( dword) (((dword) >> 24) | ((dword) << 24 ) | \
                                  (((dword) & 0x00ff0000 ) >> 8 )  | \
                                  (((dword) & 0x0000ff00 ) << 8 ))
                              
                              
#define VRE_BREAK_IF_ERR( a ) if ( (a) != VRE_ERR_OK ) break
#define VRE_RETURN_IF_ERR( a ) if ( (a) != VRE_ERR_OK ) return (a)
#define VRE_GOTO_EXIT_IF_ERR( a ) if ( (a) != VRE_ERR_OK ) goto exit 

#define VRE_IS_ERROR( a ) ( (a) < 0 )

#endif
