#pragma once
#include <string>
#include <Windows.h>

#include "DirectOutput.h"

void CHECK_ERROR(const std::string& message, HRESULT result);

namespace direct_output_proxy {

	constexpr DWORD kButtons[] = {
		SoftButton_Select,
		SoftButton_Up,
		SoftButton_Down,
	};

	std::string DevTypeToString(const GUID& dev_type);
	std::string ButtonToString(const DWORD button);
}
