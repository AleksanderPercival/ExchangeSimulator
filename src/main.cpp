#include <iostream>
#include <vector>
#include <memory>

import Order;

int main() {
    std::cout << "System initialized. Starting Exchange Simulator...\n";

    std::vector<std::unique_ptr<Order>> activeOrders;

    activeOrders.push_back(std::make_unique<LimitOrder>(1, OrderSide::Buy, 10.0, 150.50));
    activeOrders.push_back(std::make_unique<LimitOrder>(2, OrderSide::Sell, 5.0, 151.00));
    activeOrders.push_back(std::make_unique<MarketOrder>(3, OrderSide::Buy, 20.0));

    std::cout << "--- Active Orders ---\n";

    for (const auto& order : activeOrders) {
        std::cout << order->getOrderInfo() << "\n";
    }

    return 0;
}