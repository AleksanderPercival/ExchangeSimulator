module;
#include <cstdint>

export module Order;

export enum class Type {
    Limit,
    Market
};

export enum class Side {
    Buy,
    Sell
};

export struct Order {
    uint64_t id;
    Type type;
    Side side;
    uint32_t price;
    uint32_t quantity; 
    uint64_t timestamp;
};