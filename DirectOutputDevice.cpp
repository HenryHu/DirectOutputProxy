#include "DirectOutputDevice.h"

#include <Windows.h>
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

	HRESULT DirectOutputDevice::Init() {
		GUID dev_type;
		CHECK_RETURN("GetDeviceType", direct_output_->GetDeviceType(handle_, &dev_type));
		type_ = DeviceTypeGuidToDeviceType(dev_type);
		DebugW() << "Detected: " << DevTypeToString(type_) << std::endl;

		for (const auto& [page, data] : pages_) {
			CHECK_RETURN("AddPage", direct_output_->AddPage(handle_, page, data.name.c_str(), page == 0 ? FLAG_SET_AS_ACTIVE : 0));
		}
		HandlePageCallback(0, true);

		CHECK_RETURN("RegisterPageCallback", direct_output_->RegisterPageCallback(handle_, &PageCallback, this));
		CHECK_RETURN("RegisterButtonCallback", direct_output_->RegisterSoftButtonCallback(handle_, &ButtonCallback, this));
		return S_OK;
	}

	HRESULT DirectOutputDevice::UpdatePage() {
		if (!current_page_.has_value()) return S_OK;
		DWORD page = current_page_.value();

		auto it = pages_.find(page);
		if (it == pages_.end()) return S_OK;
		PageData& data = it->second;

		CHECK_RETURN("SetString Top", direct_output_->SetString(handle_, page, kTopLine,
			static_cast<DWORD>(data.top.length()), data.top.c_str()));
		CHECK_RETURN("SetString Middle", direct_output_->SetString(handle_, page, kMiddleLine,
			static_cast<DWORD>(data.middle.length()), data.middle.c_str()));
		CHECK_RETURN("SetString Bottom", direct_output_->SetString(handle_, page, kBottomLine,
			static_cast<DWORD>(data.bottom.length()), data.bottom.c_str()));

		return S_OK;
	}

	void DirectOutputDevice::HandlePageCallback(const DWORD page, const bool activated) {
		Debug() << "device: " << handle_ << " page: " << page << " active : " << activated << std::endl;
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
		Debug() << "device: " << handle_ << " buttons: " << buttons << std::endl;

		DWORD page = current_page_.has_value() ? current_page_.value() : -1;

		for (const DWORD button : kButtons) {
			if ((buttons & button) && !(buttons_ & button)) {
				DebugW() << "Button " << ButtonToString(button) << " down on page " << page << std::endl;
				if (button_callback_) {
					button_callback_(button, /*down=*/true, page);
				}
			} else if (!(buttons & button) && (buttons_ & button)) {
				DebugW() << "Button " << ButtonToString(button) << " up on page " << page << std::endl;
				if (button_callback_) {
					button_callback_(button, /*down=*/false, page);
				}
			}
		}
		buttons_ = buttons;
	}

	HRESULT DirectOutputDevice::AddPage(const DWORD page, const PageData& data, const bool activate) {
		if (pages_.contains(page)) return -ERROR_ALREADY_EXISTS;
		pages_[page] = data;
		CHECK_RETURN("AddPage", direct_output_->AddPage(handle_, page, data.name.c_str(), activate ? FLAG_SET_AS_ACTIVE : 0));
		if (activate) current_page_ = page;
		return UpdatePage();
	}

	HRESULT DirectOutputDevice::SetPage(const DWORD page, const PageData& data) {
		if (!pages_.contains(page)) return -ERROR_NOT_FOUND;
		pages_[page] = data;
		return UpdatePage();
	}

	HRESULT DirectOutputDevice::RemovePage(const DWORD page) {
		if (!pages_.contains(page)) return -ERROR_NOT_FOUND;
		pages_.erase(page);
		CHECK_RETURN("RemovePage", direct_output_->RemovePage(handle_, page));
		return S_OK;
	}

	HRESULT DirectOutputDevice::SetLine(const DWORD page, const LineIndex line, const std::wstring& content) {
		auto it = pages_.find(page);
		if (it == pages_.end()) return -ERROR_NOT_FOUND;

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
		return UpdatePage();
	}

	std::wstring DirectOutputDevice::GetInfo() {
		std::wstring info = L"device type: " + DevTypeToString(type_);
		info += L"\npages: " + std::to_wstring(pages_.size());
		for (const auto& [page, data] : pages_) {
			info += L"\npage " + std::to_wstring(page) + L": '" + data.top + L"', '" + data.middle + L"', '" + data.bottom + L"'";
			if (page == current_page_) {
				info += L" [current]";
			}
		}
		if (current_page_.has_value()) {
			info += L"\nCurrent page: " + std::to_wstring(current_page_.value());
		} else {
			info += L"\nCurrent page: mode";
		}
		return info;
	}
}
