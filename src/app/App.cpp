#include "App.h"
#include <bx/bx.h>
#include <bx/thread.h>
#include <bx/os.h>
#include "platform/common/Platform.h"


static volatile bool g_shut_down_rendering = false;

int runEventLoop()
{
	PlatformEvent event;

	while (true)
	{
		event = platformGetEvent();
		// if no more events are available
		if (event.eventType == PlatformEventType::PE_NONE) {
			return 0;
		}
		//ProcessEvent(ev);
	}
	return 0;
}

///
/// app rendering thread function
///
int32_t app_rendering_thread(bx::Thread* _self, void* _userData)
{
	while (true)
	{
		if (g_shut_down_rendering)
		{
			break;
		}
		bx::yield();
	}
	return 0;
}

void App::start()
{
	// spawn the logic thread
	static bx::Thread thread;

	thread.init(app_rendering_thread, nullptr, 0, "app_rendering");
}


///#include <GL/glew.h>
//#include <GL/wglew.h>

void App::do_frame()
{	
	//glClearColor(1.0f, 0.f, 0.f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

}

void App::shut_down()
{
	// signal the logic thread to terminate
	g_shut_down_rendering = true;
}
