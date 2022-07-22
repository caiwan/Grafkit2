#include "application.h"
#include "fwzSetup.h"

#ifdef _DEBUG
constexpr int isWindowed = 1;
#else
constexpr int isWindowed = 0;
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	fwzSettings setup;
	setup.hInstance = hInstance;

	setup.scrBPP = 32;
	setup.nVsync = 1;
	setup.nMultisample = 0;

	setup.nAlwaysOnTop = 1;
	setup.scrWidth = GetSystemMetrics(SM_CXSCREEN);
	setup.scrHeight = GetSystemMetrics(SM_CYSCREEN);
	setup.nWindowed = isWindowed;

	if (!OpenSetupDialog(&setup)) return -1;

	Application app(setup);
	return app.Execute();
}

int main() { WinMain(nullptr, nullptr, nullptr, 0); }
