// rpg.cpp : Defines the entry point for the application.
//

// for uint8_t, uint16_t etc ..
#include <stdint.h>
#include <stdio.h>
#include "pch.h"
#include "framework.h"
#include "foundation/memory.h"
#include "platform/common/Platform.h"
#include "utils/ImageUtils.h"
#include "app/App.h"
#include <bx/bx.h>
#include <bimg/bimg.h>



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // init memory allocators
    foundation::memory_globals::init();

    unsigned char* buff = (unsigned char*)malloc(1024);
    FILE* f;
    errno_t  res = fopen_s(&f, "C:/Users/Ofer/Desktop/test.tga", "rb");
    size_t numRead = fread(buff, 1, 18, f);
    fclose(f);
    
    Sapphire::InputStream inStream;
    inStream.data = buff;

    foundation::Allocator& allocator = foundation::memory_globals::default_allocator();


   
    Sapphire::load_tga_from_stream(&inStream);
    
    App theApp;

    Platform* platform = Platform::create(&theApp);
    int retval = platform->enter_message_pump();

    return retval;

}

