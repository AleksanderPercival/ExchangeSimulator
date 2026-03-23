#include <iostream>
#include <vector>
#include <memory>

import Order;
import OrderBook;

int main() {
    std::cout << "System initialized. Starting Exchange Simulator...\n";

    OrderBook orderBook;

    orderBook.addOrder(std::make_unique<LimitOrder>(1, OrderSide::Buy, 10.0, 150.50));
    orderBook.addOrder(std::make_unique<LimitOrder>(2, OrderSide::Sell, 5.0, 151.00));
    orderBook.addOrder(std::make_unique<MarketOrder>(3, OrderSide::Buy, 5000.0));
    orderBook.addOrder(std::make_unique<LimitOrder>(4, OrderSide::Sell, 15.0, 152.00));

    orderBook.displayBook();

    orderBook.displayWhaleOrders(100.0);

    return 0;
}