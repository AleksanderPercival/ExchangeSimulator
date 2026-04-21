module;
#include <cstdint>

export module MarketData;

export struct Trade {
    uint64_t timestamp;
    uint32_t price;
    uint32_t quantity;
};

export struct Candle {
    uint32_t open;
    uint32_t high;
    uint32_t low;
    uint32_t close;
    uint32_t volume;

    Candle(uint32_t price, uint32_t qty)
        : open(price), high(price), low(price), close(price), volume(qty) {
    }

    void update(uint32_t price, uint32_t qty) {
        if (price > high) high = price;
        if (price < low) low = price;
        close = price;
        volume += qty;
    }
};