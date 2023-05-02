#pragma once

#include "fwzSetup.h"

//#include <grafkit/core/GraphicsCore.h>
#include <grafkit/core/system.h>

class Application : public Grafkit::Core::System
{
public:
	explicit Application(fwzSettings& settings);
	~Application() override;

	int Initialize() override;
	int Mainloop() override;
	void Release() override;

private:
	// Grafkit::Core::Graphics mGraphicsContext;

	fwzSettings mSettings;
};
