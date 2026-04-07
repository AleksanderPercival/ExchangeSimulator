module;

#include <algorithm>

export module MarketData;

export struct Candle {
    double open; 
    double high;
    double low;
    double close; 
    double volume;

    Candle(double price, double quantity)
        : open(price), high(price), low(price), close(price), volume(quantity) {
    }

    void update(double price, double quantity) {
        high = std::max(high, price);
        low = std::min(low, price);
        close = price;
        volume += quantity;
    }
};