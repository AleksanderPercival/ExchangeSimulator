module;

#include <cstdint>

export module Order;

export enum class OrderType {
	Buy,
	Sell
};

export class Order {
private:
	uint64_t id;
	OrderType type;
	double price;
	double quantity;

public:
	Order(uint64_t id, OrderType type, double price, double quantity)
		: id(id), type(type), price(price), quantity(quantity) {}

	uint64_t getId() const { return id; }
	OrderType getType() const { return type; }
	double getPrice() const { return price; }
	double getQuantity() const { return quantity; }
};