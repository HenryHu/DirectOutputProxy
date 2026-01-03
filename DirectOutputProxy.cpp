#include <iostream>
#include <Windows.h>
#include <map>
#include <optional>
#include "DirectOutput.h"
#include "DirectOutputImpl.h"
#include "Utils.h"

namespace direct_output_proxy {
    struct PageData {
        std::wstring name;
        std::wstring top;
        std::wstring middle;
        std::wstring bottom;
    };

    using PagesData = std::map<DWORD, PageData>;

    struct DeviceContext {
        DWORD buttons;
        std::optional<DWORD> current_page;

        PagesData pages;
    };

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

	class DirectOutputProxy {
	public:
        bool Init() {
			HRESULT status = direct_output_.Initialize(L"DirectOutputProxy");
			if (FAILED(status)) {
				if (status == E_NOTIMPL) {
					std::cerr << "Failed to initialize: DLL failed to load, maybe missing from Registry; check HKLM\\SOFTWARE\\Saitek\\DirectOutput\\DirectOutput_Saitek";
				}
				else {
					std::cerr << "Failed to initialize: " << std::hex << status << std::endl;
				}
                return false;
			}

			direct_output_.Enumerate(&DeviceCallback, this);
			direct_output_.RegisterDeviceCallback(&DeviceChangeCallback, this);
            return true;
        }

        bool Shutdown() {
			HRESULT status = direct_output_.Deinitialize();
			if (FAILED(status)) {
				std::cerr << "Failed to deinitialize: " << status << std::endl;
				return false;
			}
            return true;
        }

		static void __stdcall PageCallback(void* device, DWORD page, bool activated, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandlePageCallback(device, page, activated);
		}

		void HandlePageCallback(void* device, DWORD page, bool activated) {
			DeviceContext& device_context = devices_[device];
			std::cout << "device: " << device << " page: " << page << " active : " << activated << std::endl;
			if (!activated) {
				if (device_context.current_page == page) {
					device_context.current_page.reset();
				}
				return;
			}
			device_context.current_page = page;

			auto it = device_context.pages.find(page);
			if (it != device_context.pages.end()) {
				CHECK_ERROR("SetString Top", direct_output_.SetString(device, page, 0, it->second.top.length(), it->second.top.c_str()));
				CHECK_ERROR("SetString Middle", direct_output_.SetString(device, page, 1, it->second.middle.length(), it->second.middle.c_str()));
				CHECK_ERROR("SetString Bottom", direct_output_.SetString(device, page, 2, it->second.bottom.length(), it->second.bottom.c_str()));
			}
		}

		static void __stdcall ButtonCallback(void* device, DWORD buttons, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandleButtonCallback(device, buttons);
		}

		void HandleButtonCallback(void* device, DWORD buttons) {
			DeviceContext& device_context = devices_[device];
			std::cout << "device: " << device << " buttons: " << buttons << std::endl;

			for (const DWORD button : kButtons) {
				if ((buttons & button) && !(device_context.buttons & button)) {
					std::cout << "Button " << ButtonToString(button) << " down on page " << (device_context.current_page.has_value() ? device_context.current_page.value() : -1) << std::endl;
				}
				else if (!(buttons & button) && (device_context.buttons & button)) {
					std::cout << "Button " << ButtonToString(button) << " up on page " << (device_context.current_page.has_value() ? device_context.current_page.value() : -1) << std::endl;
				}
			}
			device_context.buttons = buttons;
		}

		static void __stdcall DeviceCallback(void* device, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandleDeviceCallback(device);
		}

		void HandleDeviceCallback(void* device) {
			DeviceContext& device_context = devices_[device];
			std::cout << "device: " << device << std::endl;

			GUID dev_type;
			CHECK_ERROR("GetDeviceType", direct_output_.GetDeviceType(device, &dev_type));
			std::cerr << "Detected: " << DevTypeToString(dev_type) << std::endl;

			CHECK_ERROR("RegisterPageCallback", direct_output_.RegisterPageCallback(device, &PageCallback, this));

			InitPages(device_context.pages);
			for (const auto& [page, data] : device_context.pages) {
				CHECK_ERROR("AddPage", direct_output_.AddPage(device, page, data.name.c_str(), page == 0 ? FLAG_SET_AS_ACTIVE : 0));
			}
			HandlePageCallback(device, 0, true);

			CHECK_ERROR("RegisterButtonCallback", direct_output_.RegisterSoftButtonCallback(device, &ButtonCallback, this));
		}

		static void __stdcall DeviceChangeCallback(void* device, bool added, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandleDeviceChangeCallback(device, added);
		}

		void HandleDeviceChangeCallback(void* device, bool added) {
			std::cout << "device: " << device << " " << (added ? "added" : "removed") << std::endl;

			if (added) {
				HandleDeviceCallback(device);
			} else {
				devices_.erase(device);
			}
		}

	private:
		CDirectOutput direct_output_;
        std::map<void*, DeviceContext> devices_;
	};

}

int main()
{
	direct_output_proxy::DirectOutputProxy proxy;
	if (!proxy.Init()) return 1;
	std::cin.get();
	if (!proxy.Shutdown()) return 1;
	return 0;
}