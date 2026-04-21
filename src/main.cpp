#include <iostream>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>

import Order;
import Asset;
import OrderBook;
import RiskManager;
import DataTerminal;
import Exchange;

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "[GLFW ERROR] " << error << ": " << description << '\n';
}

int main() {
    std::cout << "[SYSTEM] Booting Graphical Interface...\n";

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Exchange Simulator - Control Panel", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    DataTerminal terminal;
    terminal.initializeStorage();

    Exchange exchange;
    exchange.addAsset({ "BTC/USD", 1, 100 });
    exchange.addAsset({ "ETH/USD", 1, 10 }); 
    exchange.addAsset({ "AAPL", 1, 1 });  

    std::cout << "[SYSTEM] GUI Initialized successfully. Entering Main Loop.\n";

    std::cout << "[SYSTEM] GUI Initialized successfully. Entering Main Loop.\n";

    const char* marketSymbols[] = { "BTC/USD", "ETH/USD", "AAPL" };
    static int currentMarketIdx = 0;

    static uint64_t nextOrderId = 1000;
    static int orderSide = 0; // 0 = Buy, 1 = Sell
    static int orderType = 0; // 0 = Limit, 1 = Market
    static float quantity = 10.0f;
    static float price = 150.0f;

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. CONTROL PANEL
        ImGui::Begin("Trading Terminal - BTC/USD", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Combo("Market", &currentMarketIdx, marketSymbols, 3);
        ImGui::Separator();

        ImGui::Combo("Side", &orderSide, "Buy\0Sell\0");
        ImGui::Combo("Type", &orderType, "Limit\0Market\0");

        ImGui::InputFloat("Quantity", &quantity, 1.0f, 10.0f, "%.1f");

        if (orderType == 0) {
            ImGui::InputFloat("Price", &price, 0.5f, 1.0f, "%.2f");
        }

        ImGui::Separator();

        if (ImGui::Button("Submit Order", ImVec2(150, 40))) {
            Side side = (orderSide == 0) ? Side::Buy : Side::Sell;
            Type type = (orderType == 0) ? Type::Limit : Type::Market;

            uint32_t enginePrice = static_cast<uint32_t>(price * 100);
            uint32_t engineQty = static_cast<uint32_t>(quantity);

            auto now = std::chrono::high_resolution_clock::now();
            uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

            Order newOrder{ nextOrderId++, type, side, enginePrice, engineQty, timestamp };

            const char* activeSymbol = marketSymbols[currentMarketIdx];
            exchange.routeOrder(activeSymbol, newOrder);

            std::cout << "[GUI] Order submitted to" << activeSymbol << "core: "
                << (side == Side::Buy ? "BUY " : "SELL ")
                << engineQty << " units @ price level " << enginePrice << "\n";
        }
        ImGui::End();

        // 2. LIVE ORDER BOOK (Active)
        ImGui::Begin("Live Order Book - BTC/USD", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        ImGui::Columns(2, "orderBookColumns");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "BIDS (Buy)");
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "ASKS (Sell)");
        ImGui::NextColumn();
        ImGui::Separator();

        const char* activeSymbol = marketSymbols[currentMarketIdx];
        OrderBook* activeBook = exchange.getOrderBook(activeSymbol);

        if (activeBook) {
            auto bids = activeBook->getBidsSnapshot();
            for (const auto& b : bids) { ImGui::TextUnformatted(b.c_str()); }
            ImGui::NextColumn();

            auto asks = activeBook->getAsksSnapshot();
            for (const auto& a : asks) { ImGui::TextUnformatted(a.c_str()); }
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "SYSTEM ERROR: MARKET OFFLINE");
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::End();

        // 3. TRADE HISTORY (Active Report System)
        ImGui::Begin("Trade History", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        if (activeBook) {
            auto history = activeBook->getTradeHistory();
            if (history.empty()) {
                ImGui::TextDisabled("No trades executed yet...");
            }
            else {
                for (auto it = history.rbegin(); it != history.rend(); ++it) {
                    float displayPrice = it->price / 100.0f;
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[TRADE] %d units @ $%.2f", it->quantity, displayPrice);
                }
            }
        }
        else {
            ImGui::TextDisabled("Awaiting Matching Engine installation...");
        }
        ImGui::End();

        // 4. MARKET CHART (Active candle drawing system)
        ImGui::Begin(("Market Chart - " + std::string(activeSymbol)).c_str());

        if (activeBook) {
            auto chartData = activeBook->getChartData();

            if (ImPlot::BeginPlot("Price Action", ImVec2(-1, 300))) {
                ImPlot::SetupAxes("Time (Candles)", "Price (USD)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

                for (int i = 0; i < chartData.size(); ++i) {
                    ImGui::PushID(i);

                    const auto& c = chartData[i];
                    double x = i; //The X axis is the numbers of subsequent candles

                    float open = c.open / 100.0f;
                    float high = c.high / 100.0f;
                    float low = c.low / 100.0f;
                    float close = c.close / 100.0f;

                    bool isBull = close >= open;
                    ImVec4 color = isBull ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f);

                    double wick_x[2] = { x, x };
                    double wick_y[2] = { low, high };
                    ImPlot::SetNextLineStyle(color, 1.0f);
                    ImPlot::PlotLine("##Wick", wick_x, wick_y, 2);

                    double body_x[2] = { x, x };
                    double body_y[2] = { open, close };

                    if (open == close) {
                        body_x[0] -= 0.2; body_x[1] += 0.2;
                    }

                    ImPlot::SetNextLineStyle(color, 6.0f);
                    ImPlot::PlotLine("##Body", body_x, body_y, 2);

                    ImGui::PopID();
                }
                ImPlot::EndPlot();
            }
        }
        else {
            ImGui::TextDisabled("Chart data feed disconnected.");
        }
        ImGui::End();

        //RENDER
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.12f, 0.14f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    std::cout << "[SYSTEM] Shutting down GUI...\n";
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}