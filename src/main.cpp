/**
 * @file main.cpp
 * @brief IoT Smart Home Monitoring System — Bootstrap Entry Point
 *
 * main.cpp chỉ làm 3 việc:
 *   1. Setup signal handler
 *   2. Tạo Application
 *   3. initialize() → run() → shutdown()
 *
 * Toàn bộ logic nằm trong:
 *   - Application       (Facade — orchestrator)
 *   - SensorService     (sensor lifecycle + reading loop)
 *   - CommunicationService (protocol + backend send)
 *   - AutomationService (rules engine + device triggers)
 */

#include "app/Application.h"

#include <csignal>
#include <memory>

// ─── Application instance for signal handler access ──────────────────
static std::unique_ptr<iot::Application> g_app;

void signalHandler(int) {
    if (g_app) {
        g_app->requestStop();
    }
}

int main() {
    // Signal handling for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    g_app = std::make_unique<iot::Application>();

    if (!g_app->initialize("config/config.json")) {
        return 1;
    }

    g_app->run(5);  // 5 sensor reading cycles for demo
    g_app->shutdown();
    g_app.reset();

    return 0;
}
