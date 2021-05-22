#pragma once
class App
{
public:
	void start(void* _nwh, unsigned int _width, unsigned int _height);
	void do_frame();
	void shut_down();

private:
	unsigned int m_windowWidth;
	unsigned int m_windowHeight;
};

