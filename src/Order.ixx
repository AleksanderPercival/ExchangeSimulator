module;

#include <string>
#include <cstdint>
#include <limits>

export module Order;

export enum class OrderSide {Buy,Sell};

export class Order {
protected:
	uint64_t id;
	OrderSide side;
	double quantity;

public:
	Order(uint64_t id, OrderSide side, double quantity)
		: id(id), side(side), quantity(quantity) {
	}

	virtual ~Order() = default;

	virtual std::string getOrderInfo() const = 0;

	virtual double getPrice() const = 0;

	uint64_t getId() const { return id; }
	OrderSide getSide() const { return side; }
	double getQuantity() const { return quantity; }

	void reduceQuantity(double amount) {
		quantity -= amount;
	}
};

export class LimitOrder : public Order {
private:
	double limitPrice;

public:
	LimitOrder(uint64_t id, OrderSide side, double quantity, double limitPrice)
		: Order(id, side, quantity), limitPrice(limitPrice) {
	}

	double getPrice() const override { return limitPrice; }

	std::string getOrderInfo() const override {
		return "Limit Order  [ID: " + std::to_string(id) + "] - Qty: " +
			std::to_string(quantity) + " @ Price: " + std::to_string(limitPrice);
	}
};

export class MarketOrder : public Order {
public:
	MarketOrder(uint64_t id, OrderSide side, double quantity)
		: Order(id, side, quantity) {
	}

	double getPrice() const override {
		return (side == OrderSide::Buy) ? std::numeric_limits<double>::max() : 0.0;
	}

	std::string getOrderInfo() const override {
		return "Market Order [ID: " + std::to_string(id) + "] - Qty: " +
			std::to_string(quantity) + " @ Price: ANY (Market Price)";
	}
};