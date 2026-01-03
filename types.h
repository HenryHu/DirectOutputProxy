#pragma once

#include <string>
#include <map>
#include <Windows.h>

namespace direct_output_proxy {
	struct PageData {
		std::wstring name;
		std::wstring top;
		std::wstring middle;
		std::wstring bottom;
	};

	using PagesData = std::map<DWORD, PageData>;

	enum LineIndex {
		kTopLine = 0,
		kMiddleLine = 1,
		kBottomLine = 2,
	};

	enum class DeviceType {
		kUnknown,
		kX52Pro,
		kFip,
	};
}
