#pragma once

#include <string>
#include <map>
#include <intsafe.h>

namespace direct_output_proxy {
	struct PageData {
		std::wstring name;
		std::wstring top;
		std::wstring middle;
		std::wstring bottom;
	};

	using PagesData = std::map<DWORD, PageData>;
}
