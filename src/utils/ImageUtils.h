#pragma once

namespace foundation
{
    class Allocator;
}

namespace bimg
{
    struct ImageContainer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// Targa file loading

namespace Sapphire
{



#pragma pack(push)
#pragma pack(1)
    struct TargaHeader {
        char  idlength;
        char  colourmaptype;
        char  datatypecode;
        short int colourmaporigin;
        short int colourmaplength;
        char  colourmapdepth;
        short int x_origin;
        short int y_origin;
        short width;
        short height;
        char  bitsperpixel;
        char  imagedescriptor;
    };
#pragma pack(pop)


    struct InputStream
    {
        const uint8_t* data;
    };

    inline void readBytesFromStream(InputStream* _inStream, size_t _size, size_t* _inout_currPos, unsigned char* outData)
    {
        size_t currPos = *_inout_currPos;
        if (outData)
        {
            memcpy(outData, &_inStream->data[currPos], _size);
        }

        *_inout_currPos = currPos + _size;
    }

    bimg::ImageContainer* load_tga_from_stream(InputStream* _inStream);
  
}