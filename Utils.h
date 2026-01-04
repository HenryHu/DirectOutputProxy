#pragma once
#include <string>
#include <Windows.h>

#include "DirectOutput.h"
#include "types.h"
#include <optional>

#define RETURN_IF_ERROR(result) \
  { \
    const HRESULT __result = result; \
    if (FAILED(__result)) { \
      return __result; \
    } \
  }

#define CHECK_RETURN(context, result) RETURN_IF_ERROR(CHECK_ERROR(context, result))

namespace direct_output_proxy {
	HRESULT CHECK_ERROR(const std::string& context, HRESULT result);

	std::wostream& DebugW();
	std::ostream& Debug();

	constexpr DWORD kButtons[] = {
		SoftButton_Select,
		SoftButton_Up,
		SoftButton_Down,
	};

	void ReportError(const std::string& message);
	void ReportError(const std::wstring& message);

	DeviceType DeviceTypeGuidToDeviceType(const GUID& device_type);
	std::wstring DevTypeToString(const DeviceType dev_type);
	std::wstring ButtonToString(const DWORD button);

	std::optional<std::string> WstrToStr(const std::wstring& wstr);
	std::string WstrToStrOrDie(const std::wstring& wstr);

	std::string ResultToString(const HRESULT result);
	int ConvertHresultToHttpCode(const HRESULT result);
}
