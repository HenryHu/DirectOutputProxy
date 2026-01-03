#include "DirectOutputProxy.h"
#include <Windows.h>
#include <iostream>
#include "DirectOutputDevice.h"
#include "types.h"
#include "utils.h"

int main()
{
	direct_output_proxy::DirectOutputProxy proxy;
	if (!proxy.Init()) return 1;

	direct_output_proxy::DirectOutputDevice* device = proxy.GetDeviceByType(direct_output_proxy::DeviceType::kX52Pro);
	if (device == nullptr) return 2;

	device->AddPage(0, {
		.top = L"top1",
		.middle = L"middle1",
		.bottom = L"bottom1",
		}, true);

	device->AddPage(1, {
		.top = L"top2",
		.middle = L"middle2",
		.bottom = L"bottom2",
		}, false);

	device->RegisterButtonCallback([device](const DWORD button, const bool down, const DWORD page) {
		if (!down) return;
		device->SetLine(0, direct_output_proxy::kTopLine, L"Button: " + direct_output_proxy::ButtonToString(button));
	});

	std::cin.get();
	if (!proxy.Shutdown()) return 1;
	return 0;
}
