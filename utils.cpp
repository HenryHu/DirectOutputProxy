#include "utils.h"
#include <Windows.h>
#include <DirectOutput.h>
#include <string>
#include <iostream>
#include <ios>
#include "types.h"

void CHECK_ERROR(const std::string& message, HRESULT result) {
	if (SUCCEEDED(result)) return;

	std::cerr << "Failed: " << message << ' ';
	switch (result) {
	case E_PAGENOTACTIVE:
		std::cerr << "Page not active";
		break;
	case E_INVALIDARG:
		std::cerr << "Invalid argument";
		break;
	case E_OUTOFMEMORY:
		std::cerr << "Out of memory";
		break;
	case E_HANDLE:
		std::cerr << "Invalid handle";
		break;
	default:
		std::cerr << std::hex << result;
	}
	std::cerr << std::endl;
}

namespace direct_output_proxy {
	DeviceType DeviceTypeGuidToDeviceType(const GUID& guid) {
		if (guid == DeviceType_X52Pro) {
			return DeviceType::kX52Pro;
		}
		if (guid == DeviceType_Fip) {
			return DeviceType::kFip;
		}
		return DeviceType::kUnknown;
	}

	std::wstring DevTypeToString(const DeviceType dev_type) {
		switch (dev_type) {
		case DeviceType::kX52Pro:
			return L"X52 Pro";
		case DeviceType::kFip:
			return L"Flight Instrument Panel";
		}
		return L"Unknown";
	}

	std::wstring ButtonToString(const DWORD button) {
		switch (button) {
		case SoftButton_Select:
			return L"Select";
		case SoftButton_Up:
			return L"Up";
		case SoftButton_Down:
			return L"Down";
		default:
			return L"Button" + button;
		}
	}
}
