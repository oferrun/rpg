#include "App.h"
#include "bx/include/bx.h"
#include "bx/include/thread.h"
#include "bx/include/os.h"

static volatile bool g_shut_down_logic = false;

///
/// app logic thread function
///
int32_t app_logic_thread(bx::Thread* _self, void* _userData)
{
	while (true)
	{
		if (g_shut_down_logic)
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

	thread.init(app_logic_thread, nullptr, 0, "app_logic");
}


#include <GL/glew.h>
#include <GL/wglew.h>

void App::do_frame()
{	
	glClearColor(1.0f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

}

void App::shut_down()
{
	// signal the logic thread to terminate
	g_shut_down_logic = true;
}
