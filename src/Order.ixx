module;
#include <cstdint>

export module Order;

export enum class Side {
    Buy,
    Sell
};

export struct Order {
    uint64_t id;
    Side side;
    uint32_t price;
    uint32_t quantity; 
    uint64_t timestamp;
};