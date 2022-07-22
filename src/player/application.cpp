#include "application.h"

Application::Application(fwzSettings & settings) : System() { InitializeWindows(settings.scrWidth, settings.scrHeight); }

int Application::Initialize() { return 0; }

int Application::Mainloop() { return IsEscPressed(); }

void Application::Release() {}
