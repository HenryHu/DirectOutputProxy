#include <iostream>
#include <map>
#include <ios>
#include <functional>
#include <utility>

#include <Windows.h>

#include "DirectOutputImpl.h"
#include "DirectOutputDevice.h"
#include "utils.h"
#include "types.h"

namespace direct_output_proxy {
	using DeviceCallback = std::function<void(DirectOutputDevice& device)>;

	class DirectOutputProxy {
	public:
		bool Init() {
			HRESULT status = direct_output_.Initialize(L"DirectOutputProxy");
			if (FAILED(status)) {
				if (status == E_NOTIMPL) {
					std::cerr << "Failed to initialize: DLL failed to load, maybe missing from Registry; check HKLM\\SOFTWARE\\Saitek\\DirectOutput\\DirectOutput_Saitek";
				} else {
					std::cerr << "Failed to initialize: " << std::hex << status << std::endl;
				}
				return false;
			}

			CHECK_ERROR("Enumerate", direct_output_.Enumerate(&EnumerateCallback, this));
			CHECK_ERROR("RegisterDeviceCallback", direct_output_.RegisterDeviceCallback(&RawDeviceCallback, this));
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

		DirectOutputDevice* GetDeviceByType(const DeviceType dev_type) {
			for (auto& [handle, device] : devices_) {
				if (device.GetType() == dev_type) return &device;
			}
			return nullptr;
		}

		void ApplyToDevices(DeviceCallback callback) {
			for (auto& [handle, device] : devices_) {
				callback(device);
			}
		}

		void RegisterNewDeviceCallback(DeviceCallback callback) {
			new_device_cb_ = std::move(callback);
		}

		void RegisterDeviceGoneCallback(DeviceCallback callback) {
			device_gone_cb_ = std::move(callback);
		}
	private:
		static void __stdcall EnumerateCallback(void* device, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandleNewDevice(device);
		}

		void HandleNewDevice(void* handle) {
			std::cout << "device: " << handle << std::endl;

			devices_.insert(std::make_pair(handle, DirectOutputDevice(&direct_output_, handle)));

			DirectOutputDevice& device = devices_.at(handle);
			device.Init();
			if (new_device_cb_) new_device_cb_(device);
		}

		static void __stdcall RawDeviceCallback(void* device, bool added, void* param) {
			DirectOutputProxy* proxy = (DirectOutputProxy*)param;
			proxy->HandleDeviceCallback(device, added);
		}

		void HandleDeviceCallback(void* device, bool added) {
			std::cout << "device: " << device << " " << (added ? "added" : "removed") << std::endl;

			if (added) {
				HandleNewDevice(device);
			} else {
				if (device_gone_cb_ && devices_.contains(device)) device_gone_cb_(devices_.at(device));
				devices_.erase(device);
			}
		}

		CDirectOutput direct_output_;
		std::map<void*, DirectOutputDevice> devices_;

		DeviceCallback new_device_cb_, device_gone_cb_;
	};
}
