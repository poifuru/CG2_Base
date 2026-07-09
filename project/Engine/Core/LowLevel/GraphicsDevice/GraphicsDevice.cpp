#include "PCH.h"
#include "GraphicsDevice.h"
#include "GraphicsDeviceCreator.h"

void MyEngine::LowLevel::GraphicsDevice::Initialize() {
	MyEngine::LowLevel::GraphicsDeviceCreator::CreateAndSetup(
		device_,
		dxgiFactory_
	);
}