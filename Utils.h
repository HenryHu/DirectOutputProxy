#pragma once
#include <string>
#include <Windows.h>

#include "DirectOutput.h"
#include "types.h"
#include <optional>

void CHECK_ERROR(const std::string& message, HRESULT result);

namespace direct_output_proxy {

	constexpr DWORD kButtons[] = {
		SoftButton_Select,
		SoftButton_Up,
		SoftButton_Down,
	};

	DeviceType DeviceTypeGuidToDeviceType(const GUID& device_type);
	std::wstring DevTypeToString(const DeviceType dev_type);
	std::wstring ButtonToString(const DWORD button);

	std::optional<std::string> WstrToStr(const std::wstring& wstr);
	std::string WstrToStrOrDie(const std::wstring& wstr);
}
