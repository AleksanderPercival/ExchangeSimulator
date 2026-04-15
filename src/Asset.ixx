module;

#include <string>
#include <cstdint>

export module Asset;

export struct Asset {
    std::string symbol;
    uint32_t tickSize;
    uint32_t lotSize;
};