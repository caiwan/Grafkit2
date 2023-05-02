#include "application.h"
#include "fwzSetup.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	fwzSettings setup;
	setup.hInstance = hInstance;

	setup.scrBPP = 32;
	setup.nVsync = 1;
	setup.nMultisample = 0;

	setup.nAlwaysOnTop = 1;

#ifndef _DEBUG
	setup.scrWidth = GetSystemMetrics(SM_CXSCREEN);
	setup.scrHeight = GetSystemMetrics(SM_CYSCREEN);
	setup.nWindowed = 0;
#else
	setup.scrWidth = 640;
	setup.scrHeight = 480;
	setup.nWindowed = 1;
#endif

	if (!OpenSetupDialog(&setup))
		return -1;

	Application app(setup);
	return app.Execute();
}

int main() { return WinMain(nullptr, nullptr, nullptr, 0); }
