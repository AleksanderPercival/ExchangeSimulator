#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>

import Order;
import OrderBook;
import RiskManager;
import DataTerminal;

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
    OrderBook orderBook("BTC/USD");

    std::cout << "[SYSTEM] GUI Initialized successfully. Entering Main Loop.\n";

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

        ImGui::Begin("Trading Terminal - BTC/USD", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Combo("Side", &orderSide, "Buy\0Sell\0");
        ImGui::Combo("Type", &orderType, "Limit\0Market\0");

        ImGui::InputFloat("Quantity", &quantity, 1.0f, 10.0f, "%.1f");

        if (orderType == 0) {
            ImGui::InputFloat("Price", &price, 0.5f, 1.0f, "%.2f");
        }

        ImGui::Separator();

        if (ImGui::Button("Submit Order", ImVec2(150, 40))) {
            OrderSide side = (orderSide == 0) ? OrderSide::Buy : OrderSide::Sell;

            if (orderType == 0) {
                orderBook.addOrder(std::make_unique<LimitOrder>(nextOrderId++, side, quantity, price));
            }
            else {
                orderBook.addOrder(std::make_unique<MarketOrder>(nextOrderId++, side, quantity));
            }
            std::cout << "[GUI] Order submitted to core.\n";
        }
        ImGui::End();

        ImGui::Begin("Live Order Book - BTC/USD", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        ImGui::Columns(2, "orderBookColumns");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "BIDS (Buy)");
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "ASKS (Sell)");
        ImGui::NextColumn();
        ImGui::Separator();

        auto bids = orderBook.getBidsSnapshot();
        for (const auto& b : bids) {
            ImGui::TextUnformatted(b.c_str());
        }
        ImGui::NextColumn();

        auto asks = orderBook.getAsksSnapshot();
        for (const auto& a : asks) {
            ImGui::TextUnformatted(a.c_str());
        }

        ImGui::Columns(1);
        ImGui::End();

        ImGui::Begin("Trade History", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

        auto history = orderBook.getTradeHistorySnapshot();
        if (history.empty()) {
            ImGui::TextDisabled("No trades executed yet...");
        }
        else {
            for (auto it = history.rbegin(); it != history.rend(); ++it) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", it->c_str());
            }
        }
        ImGui::End();

        ImGui::Begin("Market Chart - BTC/USD");

        auto chartData = orderBook.getChartDataSnapshot();

        if (ImPlot::BeginPlot("Price Action", ImVec2(-1, 300))) {
            ImPlot::SetupAxes("Time (Candles)", "Price (USD)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

            for (int i = 0; i < chartData.size(); ++i) {
                ImGui::PushID(i);

                const auto& c = chartData[i];
                double x = i;
                bool isBull = c.close >= c.open; 

                ImVec4 color = isBull ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f);

                double wick_x[2] = { x, x };
                double wick_y[2] = { c.low, c.high };
                ImPlot::SetNextLineStyle(color, 1.0f);
                ImPlot::PlotLine("##Wick", wick_x, wick_y, 2);

                double body_x[2] = { x, x };
                double body_y[2] = { c.open, c.close };

                if (c.open == c.close) {
                    body_x[0] -= 0.2; body_x[1] += 0.2;
                }

                ImPlot::SetNextLineStyle(color, 6.0f);
                ImPlot::PlotLine("##Body", body_x, body_y, 2);

                ImGui::PopID();
            }
            ImPlot::EndPlot();
        }
        ImGui::End();

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