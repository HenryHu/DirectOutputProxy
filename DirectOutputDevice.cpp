#include "DirectOutputDevice.h"

#include <Windows.h>
#include <iostream>
#include <ostream>

#include "DirectOutputImpl.h"
#include "types.h"
#include "utils.h"
#include <DirectOutput.h>
#include <string>

namespace direct_output_proxy {
	DirectOutputDevice::DirectOutputDevice(CDirectOutput* direct_output, void* handle)
		: direct_output_(direct_output), handle_(handle) {
	}

	void DirectOutputDevice::Init() {
		GUID dev_type;
		CHECK_ERROR("GetDeviceType", direct_output_->GetDeviceType(handle_, &dev_type));
		std::wcerr << "Detected: " << DevTypeToString(dev_type) << std::endl;

		for (const auto& [page, data] : pages_) {
			CHECK_ERROR("AddPage", direct_output_->AddPage(handle_, page, data.name.c_str(), page == 0 ? FLAG_SET_AS_ACTIVE : 0));
		}
		HandlePageCallback(0, true);

		CHECK_ERROR("RegisterPageCallback", direct_output_->RegisterPageCallback(handle_, &PageCallback, this));
		CHECK_ERROR("RegisterButtonCallback", direct_output_->RegisterSoftButtonCallback(handle_, &ButtonCallback, this));
	}

	HRESULT DirectOutputDevice::UpdatePage() {
		if (!current_page_.has_value()) return S_OK;
		DWORD page = current_page_.value();

		auto it = pages_.find(page);
		if (it == pages_.end()) return S_OK;
		PageData& data = it->second;

		CHECK_ERROR("SetString Top", direct_output_->SetString(handle_, page, kTopLine,
			static_cast<DWORD>(data.top.length()), data.top.c_str()));
		CHECK_ERROR("SetString Middle", direct_output_->SetString(handle_, page, kMiddleLine,
			static_cast<DWORD>(data.middle.length()), data.middle.c_str()));
		CHECK_ERROR("SetString Bottom", direct_output_->SetString(handle_, page, kBottomLine,
			static_cast<DWORD>(data.bottom.length()), data.bottom.c_str()));

		return S_OK;
	}

	void DirectOutputDevice::HandlePageCallback(const DWORD page, const bool activated) {
		std::cout << "device: " << handle_ << " page: " << page << " active : " << activated << std::endl;
		if (!activated) {
			if (current_page_ == page) {
				current_page_.reset();
			}
		} else {
			current_page_ = page;
			UpdatePage();
		}
	}

	void DirectOutputDevice::HandleButtonCallback(const DWORD buttons) {
		std::cout << "device: " << handle_ << " buttons: " << buttons << std::endl;

		DWORD page = current_page_.has_value() ? current_page_.value() : -1;

		for (const DWORD button : kButtons) {
			if ((buttons & button) && !(buttons_ & button)) {
				std::wcout << "Button " << ButtonToString(button) << " down on page " << page << std::endl;
				if (button_callback_) {
					button_callback_(button, /*down=*/true, page);
				}
			} else if (!(buttons & button) && (buttons_ & button)) {
				std::wcout << "Button " << ButtonToString(button) << " up on page " << page << std::endl;
				if (button_callback_) {
					button_callback_(button, /*down=*/false, page);
				}
			}
		}
		buttons_ = buttons;
	}

	void DirectOutputDevice::AddPage(const DWORD page, const PageData& data, const bool activate) {
		if (pages_.contains(page)) return;
		pages_[page] = data;
		CHECK_ERROR("AddPage", direct_output_->AddPage(handle_, page, data.name.c_str(), activate ? FLAG_SET_AS_ACTIVE : 0));
		if (activate) current_page_ = page;
		UpdatePage();
	}

	void DirectOutputDevice::SetPage(const DWORD page, const PageData& data) {
		if (!pages_.contains(page)) {
			std::cerr << "setting unknown page " << page;
			return;
		}
		pages_[page] = data;
		UpdatePage();
	}

	void DirectOutputDevice::RemovePage(const DWORD page) {
		if (!pages_.contains(page)) return;
		pages_.erase(page);
		CHECK_ERROR("RemovePage", direct_output_->RemovePage(handle_, page));
	}

	void DirectOutputDevice::SetLine(const DWORD page, const LineIndex line, const std::wstring& content) {
		auto it = pages_.find(page);
		if (it == pages_.end()) return;

		switch (line) {
		case kTopLine:
			it->second.top = content;
			break;
		case kMiddleLine:
			it->second.middle = content;
			break;
		case kBottomLine:
			it->second.bottom = content;
			break;
		}
		UpdatePage();
	}
}
