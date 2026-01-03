#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include "DirectOutput.h"

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

	std::string DevTypeToString(const GUID& dev_type) {
		if (dev_type == DeviceType_X52Pro) {
			return "X52 Pro";
		}
		if (dev_type == DeviceType_Fip) {
			return "Flight Instrument Panel";
		}
		return "Unknown";
	}

	const DWORD kButtons[] = {
		SoftButton_Select,
		SoftButton_Up,
		SoftButton_Down,
	};

	std::string ButtonToString(const DWORD button) {
		switch (button) {
		case SoftButton_Select:
			return "Select";
		case SoftButton_Up:
			return "Up";
		case SoftButton_Down:
			return "Down";
		default:
			return "Button" + button;
		}
	}
}
