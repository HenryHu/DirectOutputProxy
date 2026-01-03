#include "DirectOutputDevice.h"

#include <Windows.h>
#include <iostream>
#include <ostream>

#include "DirectOutputImpl.h"
#include "types.h"
#include "utils.h"
#include <DirectOutput.h>

namespace direct_output_proxy {
	namespace {
		void InitPages(PagesData& pages) {
			pages[0] = {
				.top = L"top1",
				.middle = L"middle1",
				.bottom = L"bottom1",
			};
			pages[1] = {
				.top = L"top2",
				.middle = L"middle2",
				.bottom = L"bottom2",
			};
		}
	}

	DirectOutputDevice::DirectOutputDevice(CDirectOutput* direct_output, void* handle)
		: direct_output_(direct_output), handle_(handle) {
		InitPages(pages_);
	}

	void DirectOutputDevice::Init() {
		GUID dev_type;
		CHECK_ERROR("GetDeviceType", direct_output_->GetDeviceType(handle_, &dev_type));
		std::cerr << "Detected: " << DevTypeToString(dev_type) << std::endl;

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

		CHECK_ERROR("SetString Top", direct_output_->SetString(handle_, page, 0, data.top.length(), data.top.c_str()));
		CHECK_ERROR("SetString Middle", direct_output_->SetString(handle_, page, 1, data.middle.length(), data.middle.c_str()));
		CHECK_ERROR("SetString Bottom", direct_output_->SetString(handle_, page, 2, data.bottom.length(), data.bottom.c_str()));

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

		for (const DWORD button : kButtons) {
			if ((buttons & button) && !(buttons_ & button)) {
				std::cout << "Button " << ButtonToString(button) << " down on page " << (current_page_.has_value() ? current_page_.value() : -1) << std::endl;
			} else if (!(buttons & button) && (buttons_ & button)) {
				std::cout << "Button " << ButtonToString(button) << " up on page " << (current_page_.has_value() ? current_page_.value() : -1) << std::endl;
			}
		}
		buttons_ = buttons;
	}
}
