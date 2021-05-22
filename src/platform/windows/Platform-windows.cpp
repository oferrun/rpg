#include "pch.h"
#include "framework.h"
#include "platform/common/Platform.h"
#include <windowsx.h>
#include "app/App.h"



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
static unsigned int g_mainWindowWidth = 1024;
static unsigned int g_mainWindowHeight = 768;


#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

static PlatformEvent	g_eventQue[MAX_QUED_EVENTS];
static int			g_eventHead = 0;
static int			g_eventTail = 0;


void platformQueEvent(PlatformEventType type, int value, int value2) {
    PlatformEvent* ev = &g_eventQue[g_eventHead & MASK_QUED_EVENTS];

    if (g_eventHead - g_eventTail >= MAX_QUED_EVENTS) {
       // common->Printf("Sys_QueEvent: overflow\n");
        // we are discarding an event, but don't leak memory
       /* if (ev->evPtr) {
            Mem_Free(ev->evPtr);
        }*/
        g_eventTail++;
    }

    g_eventHead++;

    ev->eventType = type;
    ev->value1 = value;
    ev->value2 = value2;
  /*  ev->evPtrLength = ptrLength;
    ev->evPtr = ptr;
    ev->inputDevice = inputDeviceNum;*/
}

PlatformEvent platformGetEvent()
{
    PlatformEvent	ev;

    // return if we have data
    if (g_eventHead > g_eventTail) {
        g_eventTail++;
        return g_eventQue[(g_eventTail - 1) & MASK_QUED_EVENTS];
    }

    // return the empty event 
    memset(&ev, 0, sizeof(ev));

    return ev;
}

void platformClearEvents()
{
    g_eventHead = g_eventTail = 0;
}

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

   
    case WM_INPUT:
    {
        UINT dwSize;

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
            sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL)
        {
            return 0;
        }

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize,
            sizeof(RAWINPUTHEADER)) != dwSize)
            OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            //hResult = StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH, TEXT(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n"),
            //    raw->data.keyboard.MakeCode,
            //    raw->data.keyboard.Flags,
            //    raw->data.keyboard.Reserved,
            //    raw->data.keyboard.ExtraInformation,
            //    raw->data.keyboard.Message,
            //    raw->data.keyboard.VKey);
            //if (FAILED(hResult))
            //{
            //    // TODO: write error handler
            //}
            OutputDebugString(L"key event\n");
        }
        else if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            //hResult = StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH, TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"),
            //    raw->data.mouse.usFlags,
            //    raw->data.mouse.ulButtons,
            //    raw->data.mouse.usButtonFlags,
            //    raw->data.mouse.usButtonData,
            //    raw->data.mouse.ulRawButtons,
            //    raw->data.mouse.lLastX,
            //    raw->data.mouse.lLastY,
            //    raw->data.mouse.ulExtraInformation);

            //if (FAILED(hResult))
            //{
            //    // TODO: write error handler
            //}
            //OutputDebugString(szTempOutput);
           // OutputDebugString(L"mouse event\n");
        }

        delete[] lpb;
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE1, 1);


        return 0;
    }
    case WM_LBUTTONUP:
    {
       
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE1, 0);

        return 0;
    }

    case WM_RBUTTONDOWN:
    {
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE2, 1);
        break;
    }
        

    case WM_RBUTTONUP:
    {
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE2, 0);
        break;
    }
    case WM_MBUTTONDOWN:
    {
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE3, 1);
        break;
    }
    case WM_MBUTTONUP:
    {
        platformQueEvent(PlatformEventType::PE_KEY, (int)PlatformKeys::K_MOUSE3, 0);
        break;
    }
    case WM_MOUSEMOVE:
    {
        const int x = GET_X_LPARAM(lParam);
        const int y = GET_Y_LPARAM(lParam);

        // Generate an event
        platformQueEvent(PlatformEventType::PE_MOUSE_ABSOLUTE, x, y);
      
        break;
    }

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        int key = delta < 0 ? (int)PlatformKeys::K_MWHEELDOWN : (int)PlatformKeys::K_MWHEELUP;
        delta = abs(delta);
        while (delta-- > 0) {
            platformQueEvent(PlatformEventType::PE_KEY, key, true);
            platformQueEvent(PlatformEventType::PE_KEY, key, false);
        }


        break;
    }
        

    case WM_KEYDOWN:
    {
        int key;
        key = ((lParam >> 16) & 0xFF) | (((lParam >> 24) & 1) << 7);
        if (key == (int)PlatformKeys::K_PRINTSCREEN) {
            // don't queue printscreen keys.  Since windows doesn't send us key
            // down events for this, we handle queueing them with DirectInput
            break;
        }
        //else if (key == K_LCTRL || key == K_LALT || key == K_RCTRL || key == K_RALT) {
        //    // let direct-input handle this because windows sends Alt-Gr
        //    // as two events (ctrl then alt)
        //    break;
        //}
        platformQueEvent(PlatformEventType::PE_KEY, key, true);

        break;
    }
    case WM_KEYUP:
    {
        int key;
        key = ((lParam >> 16) & 0xFF) | (((lParam >> 24) & 1) << 7);
        if (key == (int)PlatformKeys::K_PRINTSCREEN) {
            // don't queue printscreen keys.  Since windows doesn't send us key
            // down events for this, we handle queueing them with DirectInput
            break;
        }
        //else if (key == K_LCTRL || key == K_LALT || key == K_RCTRL || key == K_RALT) {
        //    // let direct-input handle this because windows sends Alt-Gr
        //    // as two events (ctrl then alt)
        //    break;
        //}
        platformQueEvent(PlatformEventType::PE_KEY, key, false);
        break;
    }
    case WM_CHAR:
    {
        platformQueEvent(PlatformEventType::PE_CHAR, (int)wParam, 0);
        break;
    }
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

   
    params.rect.right = g_mainWindowWidth;
    params.rect.bottom = g_mainWindowHeight;
    params.resizable = true;

    if (!createWindow(&params, &hwnd, &hdc))
    {
        goto error;
    }

    g_hMainWindow = hwnd;
    g_hdc = hdc;



   
    // Show the window.
    ShowWindow(g_hMainWindow, SW_SHOW);



    g_theApp = app;
    g_appIsInitialized = true;
    
    return platform;
error:

    exit(0);
    return NULL;
    
}

static void systemPumpEvents()
{
    MSG msg;

    // pump the message loop
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        if (!GetMessage(&msg, NULL, 0, 0)) {
            g_theApp->shut_down();
            ExitProcess(0);
        }


        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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
  
    g_theApp->start(g_hMainWindow, g_mainWindowWidth, g_mainWindowHeight);

    RAWINPUTDEVICE Rid[2];

    Rid[0].usUsagePage = 0x01;
    Rid[0].usUsage = 0x02;
    Rid[0].dwFlags = RIDEV_INPUTSINK;   // adds HID mouse and also ignores legacy mouse messages
    Rid[0].hwndTarget = g_hMainWindow;

    Rid[1].usUsagePage = 0x01;
    Rid[1].usUsage = 0x06;
    Rid[1].dwFlags = RIDEV_INPUTSINK;   // adds HID keyboard and also ignores legacy keyboard messages
    Rid[1].hwndTarget = g_hMainWindow;

    if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
        //registration failed. Call GetLastError for the cause of the error
        int ofer = 4;
    }


    while (true)
    {
        systemPumpEvents();
        
           
        g_theApp->do_frame();

       
    }
    return 0;
}