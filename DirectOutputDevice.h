#pragma once

#include <Windows.h>
#include "DirectOutputImpl.h"
#include "types.h"
#include <optional>
#include <string>
#include <functional>
#include <utility>

namespace direct_output_proxy {
	using ButtonEventCallback = std::function<void(DWORD button, bool down, DWORD page)>;

	class DirectOutputDevice {
	public:
		DirectOutputDevice(CDirectOutput* direct_output, void* handle);

		void Init();

		// Adds a new page. Fails if the page already exists.
		void AddPage(DWORD page, const PageData& data, bool activate);

		// Updates an existing page. Fails if the page does not exist.
		void SetPage(DWORD page, const PageData& data);

		// Removes an existing page.
		void RemovePage(DWORD page);

		// Updates a line on a page.
		void SetLine(DWORD page, LineIndex line, const std::wstring& content);

		// Registers a callback which is called if there's a button event.
		void RegisterButtonCallback(ButtonEventCallback callback) {
			button_callback_ = std::move(callback);
		}
	private:
		HRESULT UpdatePage();

		static void __stdcall PageCallback(void* handle, DWORD page, bool activated, void* param) {
			DirectOutputDevice* device = (DirectOutputDevice*)param;
			device->HandlePageCallback(page, activated);
		}

		static void __stdcall ButtonCallback(void* handle, DWORD buttons, void* param) {
			DirectOutputDevice* device = (DirectOutputDevice*)param;
			device->HandleButtonCallback(buttons);
		}

		// Handles page callback (page getting activated or deactivated).
		void HandlePageCallback(const DWORD page, const bool activated);

		// Handles button callback (button pressed or released).
		// `buttons` contains the currently pressed buttons.
		void HandleButtonCallback(const DWORD buttons);

		CDirectOutput* direct_output_;
		void* handle_;
		DWORD buttons_ = 0;
		std::optional<DWORD> current_page_;

		PagesData pages_;
		ButtonEventCallback button_callback_;
	};
}
