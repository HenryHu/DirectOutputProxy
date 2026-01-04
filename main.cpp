#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>

#include <string>
#include <optional>

#include <Windows.h>
#include <shellapi.h>
#include "DirectOutputProxy.h"
#include "DirectOutputDevice.h"
#include "types.h"
#include "utils.h"

namespace {
	std::optional<std::wstring> GetParam(const crow::request& req, const std::string& name) {
		const char* content_param = req.url_params.get(name);
		if (content_param == nullptr) return std::nullopt;
		std::string content(content_param);
		return std::wstring(content.begin(), content.end());
	}
}

namespace direct_output_proxy {
	bool InitProxy(DirectOutputProxy& proxy) {
		proxy.RegisterNewDeviceCallback([](DirectOutputDevice& device) {
			if (device.GetType() != DeviceType::kX52Pro) return;
			device.AddPage(0, { .name = L"info", .top = L"info", }, true);
			device.AddPage(1, { .name = L"debug", .top = L"debug", }, false);
			device.RegisterButtonCallback([&device](const DWORD button, const bool down, const DWORD page) {
				if (!down) return;
				device.SetLine(1, kMiddleLine, L"Button: " + ButtonToString(button));
			});
		});
		return proxy.Init();
	}

	void SetupApp(crow::SimpleApp& app, DirectOutputProxy& proxy) {
		CROW_ROUTE(app, "/addpage/<int>/<int>")([&proxy](const crow::request& req, const int page, const int activate) {
			DirectOutputDevice* device = proxy.GetDeviceByType(DeviceType::kX52Pro);
			if (device == nullptr) return crow::response(404, "no device");

			PageData data;

			std::optional<std::wstring> name = GetParam(req, "name");
			if (name.has_value()) data.name = name.value();
			std::optional<std::wstring> top = GetParam(req, "top");
			if (top.has_value()) data.top = top.value();
			std::optional<std::wstring> middle = GetParam(req, "middle");
			if (middle.has_value()) data.middle = middle.value();
			std::optional<std::wstring> bottom = GetParam(req, "bottom");
			if (bottom.has_value()) data.bottom = bottom.value();

			HRESULT result = device->AddPage(page, data, activate != 0);
			if (FAILED(result)) {
				return crow::response(ConvertHresultToHttpCode(result), "error: " + ResultToString(result));
			}
			return crow::response(200, "ok");
		});

		CROW_ROUTE(app, "/delpage/<int>")([&proxy](const int page) {
			DirectOutputDevice* device = proxy.GetDeviceByType(DeviceType::kX52Pro);
			if (device == nullptr) return crow::response(404, "no device");

			HRESULT result = device->RemovePage(page);
			if (FAILED(result)) {
				return crow::response(ConvertHresultToHttpCode(result), "error: " + ResultToString(result));
			}
			return crow::response(200, "ok");
		});

		CROW_ROUTE(app, "/setline/<int>/<int>")([&proxy](const crow::request& req, const int page, const int line) {
			DirectOutputDevice* device = proxy.GetDeviceByType(DeviceType::kX52Pro);
			if (device == nullptr) return crow::response(404, "no device");

			if (line < 0 || line > 2) {
				return crow::response(416, "invalid argument: line");
			}

			std::optional<std::wstring> content = GetParam(req, "content");
			if (!content.has_value()) return crow::response(400, "missing param: content");

			HRESULT result = device->SetLine(page, (LineIndex)line, content.value());
			if (FAILED(result)) {
				return crow::response(ConvertHresultToHttpCode(result), "error: " + ResultToString(result));
			}
			return crow::response(200, "ok");
		});

		CROW_ROUTE(app, "/")([&proxy]() {
			std::string resp = "DirectOutputProxy running\n";
			proxy.ApplyToDevices([&resp](DirectOutputDevice& device) {
				std::optional<std::string> info = WstrToStr(device.GetInfo());
				if (info.has_value()) {
					resp += info.value();
				}
			});

			return crow::response(200, resp);
		});

		CROW_ROUTE(app, "/exit")([&app]() {
			app.stop();
			return "ok";
		});
	}
}

int main() {
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	crow::SimpleApp app;
	direct_output_proxy::DirectOutputProxy proxy;
	if (!direct_output_proxy::InitProxy(proxy)) return 1;
	direct_output_proxy::SetupApp(app, proxy);

	int port = 8080;
	if (argc > 1) {
		port = std::stoi(argv[1]);
	}
	app.port(port).run();

	if (!proxy.Shutdown()) return 1;
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd) {
	main();
}
