#include <stdint.h>
#include <memory.h>
#include "ImageUtils.h"
#include "bimg/include/bimg.h"

namespace Sapphire
{


    bimg::ImageContainer* load_tga_from_stream(InputStream* _inStream)
    {
        bimg::ImageContainer* image = NULL;

        Sapphire::TargaHeader tgaHeader;
        size_t bufferPos = 0;

        readBytesFromStream(_inStream, sizeof(tgaHeader), &bufferPos, (unsigned char*)&tgaHeader);

        int skipover = 0;
        skipover += tgaHeader.idlength;
        skipover += tgaHeader.colourmaptype * tgaHeader.colourmaplength;

        readBytesFromStream(_inStream, skipover, &bufferPos, NULL);

        // if uncompressed data
        if (tgaHeader.datatypecode == 2)
        {

        }
        else if (tgaHeader.datatypecode == 10) // compressed
        {
            // 
        }

        return image;
    }
}