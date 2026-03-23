#include <iostream>

import Order;

int main() {
    std::cout << "System initialized. Starting Exchange Simulator...\n";

    Order myFirstOrder(1, OrderType::Buy, 150.5, 10.0);

    std::cout << "Order created successfully:\n"
        << "ID: " << myFirstOrder.getId() << "\n"
        << "Price: " << myFirstOrder.getPrice() << "\n"
        << "Quantity: " << myFirstOrder.getQuantity() << "\n";

    return 0;
}