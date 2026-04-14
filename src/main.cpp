#include "httplib.h"
#include "nlohmann/json.hpp"
#include "controller.hpp"
#include <iostream>

using json = nlohmann::json;

int main() {
    PSController controller;
    if (!controller.connect()) {
        std::cout << "Пристрій не знайдено. Перевірте підключення." << std::endl;
    }

    httplib::Server svr;

    svr.Post("/control", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            auto d = json::parse(req.body);
            controller.update(d["r"], d["g"], d["b"], d["v1"], d["v2"]);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } catch (...) {
            res.status = 400;
        }
    });

    std::cout << "Сервер активний на http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}
