#include <stdio.h>

#include "vre_file.h"

vre_result vre_file_open ( vre_char *pFilename, vre_uint32 flags, 
                           vre_file *pFile )
{
    char mode[8];
    vre_uint32 index = 0;
    
    if ( flags & VRE_FILE_READ )
    {
        mode[index] = 'r';
        index++;
    }
    
    if ( flags & VRE_FILE_WRITE )
    {
        mode[index] = 'w';
        index++;
    }

    if ( flags & VRE_FILE_BINARY )
    {
        mode[index] = 'b';
        index++;
    }
   
    if ( flags & VRE_FILE_APPEND )
    {
        mode[index] = 'a';
        index++;   
    }
   
    if ( flags & VRE_FILE_UPDATE )
    {
        mode[index] = '+';
        index++;   
    }

    mode[index] = 0;       
    *pFile = (vre_file *) fopen(pFilename, mode);
    
    if ( *pFile == 0 )
    {
        return VRE_ERR_FILE_OPEN;
    }
    else
    {
        return VRE_ERR_OK;
    }
}

void vre_file_close ( vre_file file )
{
    fclose ( (FILE*) file );
}

vre_result vre_file_read ( vre_file file, vre_uint8 *pData, 
                           vre_uint32 num_bytes )
{
    vre_uint32  bytes_read;

    bytes_read = fread( pData, 1, num_bytes, file );
    
    if ( bytes_read != num_bytes )
    {
        return VRE_ERR_FILE_READ;
    }
    else
    {
        return VRE_ERR_OK;
    }
}

vre_result vre_file_seek ( vre_file file, vre_uint32 pos )
{
     if ( fseek( file, pos, SEEK_SET ) != 0 )
     {
        return VRE_ERR_FILE_SEEK;
     }
     else
     {
        return VRE_ERR_OK;
     }
}


vre_result vre_file_size ( vre_file file, vre_uint32 *pSize )
{
    long currentPos;
    long size;
    int stat;
    
    currentPos = ftell ( file );
    if ( currentPos < 0 )
    {
        return VRE_ERR_FILE_SEEK;
    }
    
    stat = fseek ( file, 0, SEEK_END );
    if ( stat < 0 )
    {
        return VRE_ERR_FILE_SEEK;
    }
    
    size = ftell ( file );
    if ( size < 0 )
    {
        return VRE_ERR_FILE_SEEK;
    }
    
    *pSize = size;
    
    stat = fseek ( file, currentPos, SEEK_SET );
    if ( stat < 0 )
    {
        return VRE_ERR_FILE_SEEK;
    }
    
    return VRE_ERR_OK;
}


