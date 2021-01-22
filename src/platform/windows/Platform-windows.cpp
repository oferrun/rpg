#include "pch.h"
#include "framework.h"
#include "platform/common/Platform.h"
#include <windowsx.h>
#include "app/App.h"

#include <GL/glew.h>
#include <GL/wglew.h>

struct WindowCreationParams
{
    RECT rect;
    bool fullscreen;
    bool resizable;
    int samples;
};


static HINSTANCE g_hInstance = NULL;
static HWND g_hMainWindow = NULL;
static HDC g_hdc = NULL;

static bool createWindow(WindowCreationParams* params, HWND* hwnd, HDC* hdc)
{
    bool fullscreen = false;
    bool resizable = false;
    RECT rect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT };

    if (params)
    {

        memcpy(&rect, &params->rect, sizeof(RECT));
        fullscreen = params->fullscreen;
        resizable = params->resizable;
    }

    // Set the window style.
    DWORD style, styleEx;
    if (fullscreen)
    {
        style = WS_POPUP;
        styleEx = WS_EX_APPWINDOW;
    }
    else
    {
        if (resizable)
            style = WS_OVERLAPPEDWINDOW;
        else
            style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU;
        styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    }
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    // Adjust the window rectangle so the client size is the requested size.
    AdjustWindowRectEx(&rect, style, FALSE, styleEx);

    // Create the native Windows window.
    *hwnd = CreateWindowEx(styleEx, L"rpg", L"rpg", style, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, g_hInstance, NULL);
    if (*hwnd == NULL)
    {

        return false;
    }

    // Get the drawing context.
    *hdc = GetDC(*hwnd);
    if (*hdc == NULL)
    {

        return false;
    }

    // Center the window
    GetWindowRect(*hwnd, &rect);
    const int screenX = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
    const int screenY = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
    SetWindowPos(*hwnd, *hwnd, screenX, screenY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    return true;
}


//////////////

#define DEFAULT_COLOR_BUFFER_SIZE 32
#define DEFAULT_DEPTH_BUFFER_SIZE 24
#define DEFAULT_STENCIL_BUFFER_SIZE 8
static HGLRC g_hrc = 0;


bool initRenderer(WindowCreationParams* params, HDC hdc, HWND hwnd)
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = DEFAULT_COLOR_BUFFER_SIZE;
    pfd.cDepthBits = DEFAULT_DEPTH_BUFFER_SIZE;
    pfd.cStencilBits = DEFAULT_STENCIL_BUFFER_SIZE;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0)
    {
        DestroyWindow(hwnd);
        // GP_ERROR("Failed to choose a pixel format.");
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd))
    {
        DestroyWindow(hwnd);
        //GP_ERROR("Failed to set the pixel format.");
        return false;
    }

    HGLRC tempContext = wglCreateContext(hdc);
    if (!tempContext)
    {
        DestroyWindow(hwnd);
        //GP_ERROR("Failed to create temporary context for initialization.");
        return false;
    }
    wglMakeCurrent(hdc, tempContext);


    // Initialize GLEW
    if (GLEW_OK != glewInit())
    {
        wglDeleteContext(tempContext);
        DestroyWindow(hwnd);
        //GP_ERROR("Failed to initialize GLEW.");
        return false;
    }

#if 1
    if (wglChoosePixelFormatARB && wglCreateContextAttribsARB)
    {
        // Choose pixel format using wglChoosePixelFormatARB, which allows us to specify
        // additional attributes such as multisampling.
        //
        // Note: Keep multisampling attributes at the start of the attribute lists since code below
        // assumes they are array elements 0 through 3.
        int attribList[] = {
            WGL_SAMPLES_ARB,  0,
            WGL_SAMPLE_BUFFERS_ARB,  0,
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, DEFAULT_COLOR_BUFFER_SIZE,
            WGL_DEPTH_BITS_ARB, DEFAULT_DEPTH_BUFFER_SIZE,
            WGL_STENCIL_BITS_ARB, DEFAULT_STENCIL_BUFFER_SIZE,
            0
        };


        UINT numFormats;
        if (!wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats) || numFormats == 0)
        {


            wglDeleteContext(tempContext);
            DestroyWindow(hwnd);

            return false;

        }

        // Create new/final window if needed
        if (g_hMainWindow)
        {
            DestroyWindow(g_hMainWindow);

            if (!createWindow(params, &g_hMainWindow, &g_hdc))
            {
                wglDeleteContext(tempContext);
                return false;
            }
        }

        // Set final pixel format for window
        if (!SetPixelFormat(g_hdc, pixelFormat, &pfd))
        {
            // GP_ERROR("Failed to set the pixel format: %d.", (int)GetLastError());
            return false;
        }

        // Create our new GL context
        int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
            0
        };

        if (!(g_hrc = wglCreateContextAttribsARB(g_hdc, 0, attribs)))
        {
            wglDeleteContext(tempContext);
            // GP_ERROR("Failed to create OpenGL context.");
            return false;
        }

        // Delete the old/temporary context and window
        wglDeleteContext(tempContext);

        // Make the new context current
        if (!wglMakeCurrent(g_hdc, g_hrc) || !g_hrc)
        {
            //GP_ERROR("Failed to make the window current.");
            return false;
        }
    }


    //// Vertical sync.
    //if (wglSwapIntervalEXT)
    //    wglSwapIntervalEXT(__vsync ? 1 : 0);

#endif
    return true;
}

///////////


static bool g_appIsInitialized = false;

static App* g_theApp = nullptr;





LRESULT CALLBACK __WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   

    if (g_appIsInitialized == false || hwnd != g_hMainWindow)
    {
        // Ignore messages that are not for our game window.
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    static bool shiftDown = false;
    static bool capsOn = false;

    switch (msg)
    {
    case WM_CLOSE:

        DestroyWindow(hwnd);

        return 0;

    case WM_DESTROY:
       
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

      
        return 0;
    }
    case WM_LBUTTONUP:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

       
        return 0;
    }
    case WM_RBUTTONDOWN:
       
        break;

    case WM_RBUTTONUP:
      
        break;

    case WM_MBUTTONDOWN:
       
        break;

    case WM_MBUTTONUP:
        
        break;

    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

      
        break;
    }

    case WM_MOUSEWHEEL:
        tagPOINT point;
        point.x = GET_X_LPARAM(lParam);
        point.y = GET_Y_LPARAM(lParam);
      //  ScreenToClient(__hwnd, &point);
       
        break;

    case WM_KEYDOWN:
        if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT)
            shiftDown = true;

        if (wParam == VK_CAPITAL)
            capsOn = !capsOn;

       
        break;

    case WM_KEYUP:
        if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT)
            shiftDown = false;

       
        break;

    case WM_CHAR:
       
        break;

    case WM_UNICHAR:
     
        break;

    case WM_SETFOCUS:
        break;

    case WM_KILLFOCUS:
        break;

    case WM_SIZE:
        // Window was resized.
     
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


Platform* Platform::create(App* app)
{
    // Get the application module handle.
    g_hInstance = ::GetModuleHandle(NULL);

    // Register our window class.
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)__WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;  // No brush - we are going to paint our own background
    wc.lpszMenuName = NULL;  // No default menu
    wc.lpszClassName = L"rpg";

    // Read window settings from config.
    WindowCreationParams params;
    params.fullscreen = false;
    params.resizable = false;
    params.rect.left = 0;
    params.rect.top = 0;
    params.rect.right = 0;
    params.rect.bottom = 0;
    params.samples = 0;

    HWND hwnd = NULL;
    HDC hdc = NULL;

    Platform* platform = new Platform();

    if (!::RegisterClassEx(&wc))
    {

        goto error;
    }

   
    params.rect.right = 1024;
    params.rect.bottom = 768;
    params.resizable = true;

    if (!createWindow(&params, &hwnd, &hdc))
    {
        goto error;
    }

    g_hMainWindow = hwnd;
    g_hdc = hdc;

    initRenderer(&params, hdc, hwnd);

   
    // Show the window.
    ShowWindow(g_hMainWindow, SW_SHOW);

    g_theApp = app;
    g_appIsInitialized = true;
    
    return platform;
error:

    exit(0);
    return NULL;
    
}




/**
 * Begins processing the platform messages.
 *
 * This method handles all OS window messages and drives the game loop.
 * It normally does not return until the application is closed.
 *

 * @return The platform message pump return code.
 */
int Platform::enter_message_pump()
{
  
    g_theApp->start();

    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                g_theApp->shut_down();
                return (int)msg.wParam;
            }
        }
        else
        {
            g_theApp->do_frame();
            SwapBuffers(g_hdc);
        }
    }
    return 0;
}