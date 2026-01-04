#include "utils.h"
#include <Windows.h>
#include <DirectOutput.h>
#include <string>
#include <iostream>
#include <ios>
#include "types.h"
#include <cstdlib>
#include <optional>
#include <sstream>

namespace direct_output_proxy {
	void ReportError(const std::wstring& message) {
		MessageBoxW(0, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
	}

	void ReportError(const std::string& message) {
		MessageBoxA(0, message.c_str(), "Error", MB_OK | MB_ICONERROR);
	}

	std::string ResultToString(const HRESULT result) {
		switch (result) {
		case S_OK:
			return "OK";
		case E_PAGENOTACTIVE:
			return "Page not active";
		case E_INVALIDARG:
			return "Invalid argument";
		case E_OUTOFMEMORY:
			return "Out of memory";
		case E_HANDLE:
			return "Invalid handle";
		case -ERROR_ALREADY_EXISTS:
			return "Already exists";
		case -ERROR_NOT_FOUND:
			return "Not Found";
		default:
			std::stringstream ss;
			ss << std::hex << result;
			return ss.str();
		}
	}

	HRESULT CHECK_ERROR(const std::string& context, HRESULT result) {
		if (SUCCEEDED(result)) return result;

		std::stringstream ss;
		ss << "Failed: " << context << ' ' << ResultToString(result) << std::endl;
		std::string msg = ss.str();

#ifdef _CONSOLE
		std::cerr << msg;
#else
		ReportError(msg);
#endif
		return result;
	}

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

	std::optional<std::string> WstrToStr(const std::wstring& wstr) {
		char buf[1024];
		if (wcstombs_s(nullptr, buf, sizeof(buf), wstr.c_str(), sizeof(buf) - 1) != 0) return std::nullopt;
		return std::string(buf);
	}

	std::string WstrToStrOrDie(const std::wstring& wstr) {
		std::optional<std::string> ret = WstrToStr(wstr);
		return ret.value();
	}

	std::wostream& DebugW() {
		return std::wcerr;
	}

	std::ostream& Debug() {
		return std::cerr;
	}

	int ConvertHresultToHttpCode(const HRESULT result) {
		switch (result) {
		case -ERROR_NOT_FOUND:
			return 404;
		case -ERROR_ALREADY_EXISTS:
			return 409;
		case E_INVALIDARG:
			return 400;
		case E_OUTOFMEMORY:
			return 413;
		default:
			return 500;
		}
	}
}
