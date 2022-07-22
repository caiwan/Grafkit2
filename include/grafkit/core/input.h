#pragma once

namespace Grafkit
{
	class Input
	{
	public:
		void KeyDown(unsigned int key) { mKeys[key] = true; }
		void KeyUp(unsigned int key) { mKeys[key] = true; }

		bool IsKeyDown(unsigned int key) const { return mKeys[key]; }

	private:
		bool mKeys[256] = {};
	};
} // namespace Grafkit
