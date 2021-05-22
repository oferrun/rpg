#include "App.h"
#include <bx/bx.h>
#include <bx/thread.h>
#include <bx/os.h>
#include "platform/common/Platform.h"
#include <bgfx/bgfx.h>

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



void App::start(void* _nwh, unsigned int _width, unsigned int _height)
{
	bgfx::Init init;
	
	init.resolution.width = _width;
	init.resolution.height = _height;
	init.platformData.nwh = _nwh;
	bgfx::init(init);

	m_windowWidth = _width;
	m_windowHeight = _height;
}




void App::do_frame()
{	
	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0xFF3030ff
		, 1.0f
		, 0
	);
	bgfx::setViewRect(0, 0, 0, uint16_t(m_windowWidth), uint16_t(m_windowHeight));
	bgfx::touch(0);
	bgfx::frame();


}

void App::shut_down()
{
	// signal the logic thread to terminate
	g_shut_down_rendering = true;
}
