#ifndef VRE_FILE_H
#define VRE_FILE_H

#include "vre_defs.h"

typedef void* vre_file;

#define VRE_FILE_READ   1
#define VRE_FILE_WRITE  2
#define VRE_FILE_BINARY 4
#define VRE_FILE_APPEND 8
#define VRE_FILE_UPDATE 16

vre_result vre_file_open ( vre_char *pFilename, vre_uint32 flags, 
                           vre_file *pFile );

void vre_file_close ( vre_file file );

vre_result vre_file_read ( vre_file file, vre_uint8 *pData, 
                           vre_uint32 num_bytes );

vre_result vre_file_seek ( vre_file file, vre_uint32 pos );

vre_result vre_file_size ( vre_file file, vre_uint32 *pSize );



#endif
















