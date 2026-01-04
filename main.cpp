#include <crow/app.h>
#include <crow/http_request.h>
#include <crow/http_response.h>

#include <string>
#include <optional>

#include <Windows.h>
#include "DirectOutputProxy.h"
#include "DirectOutputDevice.h"
#include "types.h"
#include "utils.h"

std::optional<std::wstring> GetParam(const crow::request& req, const std::string& name) {
	const char* content_param = req.url_params.get(name);
	if (content_param == nullptr) return std::nullopt;
	std::string content(content_param);
	std::wstring wcontent(content.begin(), content.end());

	return wcontent;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	crow::SimpleApp app;
	direct_output_proxy::DirectOutputProxy proxy;
	if (!proxy.Init()) return 1;

	direct_output_proxy::DirectOutputDevice* device = proxy.GetDeviceByType(direct_output_proxy::DeviceType::kX52Pro);
	if (device == nullptr) return 2;

	device->AddPage(0, { .name = L"info", .top = L"info", }, true);
	device->AddPage(1, { .name = L"debug", .top = L"debug", }, false);

	device->RegisterButtonCallback([device](const DWORD button, const bool down, const DWORD page) {
		if (!down) return;
		device->SetLine(1, direct_output_proxy::kTopLine, L"Button: " + direct_output_proxy::ButtonToString(button));
	});

	CROW_ROUTE(app, "/addpage/<int>/<int>")([&proxy](const crow::request& req, const int page, const int activate) {
		direct_output_proxy::DirectOutputDevice* device = proxy.GetDeviceByType(direct_output_proxy::DeviceType::kX52Pro);
		if (device == nullptr) return crow::response(404, "no device");

		direct_output_proxy::PageData data;

		std::optional<std::wstring> name = GetParam(req, "name");
		if (name.has_value()) data.name = name.value();
		std::optional<std::wstring> top = GetParam(req, "top");
		if (top.has_value()) data.top = top.value();
		std::optional<std::wstring> middle = GetParam(req, "middle");
		if (middle.has_value()) data.middle = middle.value();
		std::optional<std::wstring> bottom = GetParam(req, "bottom");
		if (bottom.has_value()) data.bottom = bottom.value();

		device->AddPage(page, data, activate != 0);
		return crow::response(200, "ok");
	});

	CROW_ROUTE(app, "/delpage/<int>")([&proxy](const int page) {
		direct_output_proxy::DirectOutputDevice* device = proxy.GetDeviceByType(direct_output_proxy::DeviceType::kX52Pro);
		if (device == nullptr) return crow::response(404, "no device");

		device->RemovePage(page);
		return crow::response(200, "ok");
	});

	CROW_ROUTE(app, "/setline/<int>/<int>")([&proxy](const crow::request& req, const int page, const int line) {
		direct_output_proxy::DirectOutputDevice* device = proxy.GetDeviceByType(direct_output_proxy::DeviceType::kX52Pro);
		if (device == nullptr) return crow::response(404, "no device");

		if (line < 0 || line > 2) {
			return crow::response(416, "invalid argument: line");
		}

		std::optional<std::wstring> content = GetParam(req, "content");
		if (!content.has_value()) return crow::response(400, "missing param: content");

		device->SetLine(page, (direct_output_proxy::LineIndex)line, content.value());
		return crow::response(200, "ok");
	});

	CROW_ROUTE(app, "/")([&proxy]() {
		std::string resp = "DirectOutputProxy running\n";
		proxy.ApplyToDevices([&resp](direct_output_proxy::DirectOutputDevice& device) {
			std::optional<std::string> info = direct_output_proxy::WstrToStr(device.GetInfo());
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

	app.port(8080).run();

	if (!proxy.Shutdown()) return 1;
	return 0;
}
