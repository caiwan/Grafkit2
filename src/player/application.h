#pragma once

#include "fwzSetup.h"

#include <grafkit/core/system.h>

class Application : public Grafkit::System
{
public:
	explicit Application(fwzSettings & settings);

	int Initialize() override;
	int Mainloop() override;
	void Release() override;
};
