#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <future>

import Order;
import OrderBook;
import RiskManager;

void simulateClient(OrderBook& book, uint64_t startId, OrderSide side, int numOrders) {
    for (int i = 0; i < numOrders; ++i) {
        book.addOrder(std::make_unique<LimitOrder>(startId + i, side, 10.0, 100.0 + i));
    }
}

int main() {
    std::cout << "System initialized. Starting Exchange Simulator...\n";

    OrderBook orderBook;
    RiskManager riskManager;

    std::future<bool> scanResult = riskManager.performDeepScanAsync();

    std::cout << "[SYSTEM] Client threads activation...\n";

    {
        std::jthread broker1(simulateClient, std::ref(orderBook), 10000, OrderSide::Buy, 5000);
        std::jthread broker2(simulateClient, std::ref(orderBook), 20000, OrderSide::Sell, 5000);
    }

    std::cout << "[SYSTEM] All threads finished routing orders.\n";

    std::cout << "[SYSTEM] Waiting for RiskManager clearance...\n";
    bool isMarketSafe = scanResult.get();

    if (isMarketSafe) {
        std::cout << "[SYSTEM] Market is safe. Proceeding with operations.\n\n";
    }

    std::promise<std::string> supervisorPromise;
    std::future<std::string> supervisorFuture = supervisorPromise.get_future();

    std::jthread authThread(&RiskManager::awaitManualOverride, &riskManager, std::move(supervisorPromise));

    std::string authCode = supervisorFuture.get();
    std::cout << "[SYSTEM] Received code from future: " << authCode << "\n";

    return 0;
}