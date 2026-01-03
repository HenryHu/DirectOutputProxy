#include <iostream>
#include <Windows.h>
#include <map>
#include <optional>
#include "DirectOutput.h"
#include "DirectOutputImpl.h"

void CHECK_ERROR(const std::string& message, HRESULT result) {
    if (SUCCEEDED(result)) return;

	std::cerr << "Failed: " << message << ' ';
	switch (result) {
	case E_PAGENOTACTIVE:
		std::cerr << "Page not active";
		break;
	case E_INVALIDARG:
		std::cerr << "Invalid argument";
		break;
	case E_OUTOFMEMORY:
		std::cerr << "Out of memory";
		break;
	case E_HANDLE:
		std::cerr << "Invalid handle";
		break;
	default:
		std::cerr << std::hex << result;
	}
	std::cerr << std::endl;
}

std::string DevTypeToString(const GUID& dev_type) {
    if (dev_type == DeviceType_X52Pro) {
        return "X52 Pro";
    }
    if (dev_type == DeviceType_Fip) {
        return "Flight Instrument Panel";
    }
    return "Unknown";
}

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

struct Context {
    CDirectOutput* direct_output;

    std::map<void*, DeviceContext> devices;
};

void __stdcall HandlePageCallback(void* device, DWORD page, bool activated, void* param) {
    Context* context = (Context*)param;
    DeviceContext& device_context = context->devices[device];
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
        CHECK_ERROR("SetString Top", context->direct_output->SetString(device, page, 0, it->second.top.length(), it->second.top.c_str()));
        CHECK_ERROR("SetString Middle", context->direct_output->SetString(device, page, 1, it->second.middle.length(), it->second.middle.c_str()));
        CHECK_ERROR("SetString Bottom", context->direct_output->SetString(device, page, 2, it->second.bottom.length(), it->second.bottom.c_str()));
    }
}

const DWORD kButtons[] = {
    SoftButton_Select,
    SoftButton_Up,
    SoftButton_Down,
};

std::string ButtonToString(const DWORD button) {
    switch (button) {
    case SoftButton_Select:
        return "Select";
    case SoftButton_Up:
        return "Up";
    case SoftButton_Down:
        return "Down";
    default:
        return "Button" + button;
    }
}

void __stdcall HandleButtonCallback(void* device, DWORD buttons, void* param) {
    Context* context = (Context*)param;
    DeviceContext& device_context = context->devices[device];
    std::cout << "device: " << device << " buttons: " << buttons << std::endl;

    for (const DWORD button : kButtons) {
        if ((buttons & button) && !(device_context.buttons & button)) {
            std::cout << "Button " << ButtonToString(button) << " down on page " << (device_context.current_page.has_value() ? device_context.current_page.value() : -1) << std::endl;
        } else if (!(buttons & button) && (device_context.buttons & button)) {
            std::cout << "Button " << ButtonToString(button) << " up on page "  << (device_context.current_page.has_value() ? device_context.current_page.value() : -1)<< std::endl;
        }
    }
    device_context.buttons = buttons;
}

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

void __stdcall HandleDeviceCallback(void* device, void* param) {
    Context* context = (Context*)param;
    DeviceContext& device_context = context->devices[device];
    std::cout << "device: " << device << std::endl;

    GUID dev_type;
    CHECK_ERROR("GetDeviceType", context->direct_output->GetDeviceType(device, &dev_type));
    std::cerr << "Detected: " << DevTypeToString(dev_type) << std::endl;

    CHECK_ERROR("RegisterPageCallback", context->direct_output->RegisterPageCallback(device, &HandlePageCallback, param));

    InitPages(device_context.pages);
    for (const auto& [page, data] : device_context.pages) {
		CHECK_ERROR("AddPage", context->direct_output->AddPage(device, page, data.name.c_str(), page == 0 ? FLAG_SET_AS_ACTIVE : 0));
    }
    HandlePageCallback(device, 0, true, param);

    CHECK_ERROR("RegisterButtonCallback", context->direct_output->RegisterSoftButtonCallback(device, &HandleButtonCallback, param));
}

void __stdcall HandleDeviceChangeCallback(void* device, bool added, void* param) {
    Context* context = (Context*)param;
    std::cout << "device: " << device << " " << (added ? "added" : "removed") << std::endl;

    if (added) {
        HandleDeviceCallback(device, param);
    }
    else {
        context->devices.erase(device);
    }
}

int main()
{
    CDirectOutput direct_output;
    HRESULT status = direct_output.Initialize(L"DirectOutputProxy");
    if (FAILED(status)) {
        if (status == E_NOTIMPL) {
            std::cerr << "Failed to initialize: DLL failed to load, maybe missing from Registry; check HKLM\\SOFTWARE\\Saitek\\DirectOutput\\DirectOutput_Saitek";
		} else {
			std::cerr << "Failed to initialize: " << std::hex << status << std::endl;
		}
        return 1;
    }

    Context context{ .direct_output = &direct_output };
    direct_output.Enumerate(&HandleDeviceCallback, &context);
    direct_output.RegisterDeviceCallback(&HandleDeviceChangeCallback, &context);

    std::cin.get();
    status = direct_output.Deinitialize();
    if (FAILED(status)) {
        std::cerr << "Failed to deinitialize: " << status << std::endl;
        return 1;
    }
    return 0;
}