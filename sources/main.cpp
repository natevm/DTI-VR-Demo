#include "gldk.hpp"

int main() {
	GLDK::InitializationParameters GLDKParams = {
		1920, 1440, "Desktop Volume Renderer",
		false, true, 4, 5
	};

	if (GLDK::Initialize(GLDKParams) != true) return -1;
}