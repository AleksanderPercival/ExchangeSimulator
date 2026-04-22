module;
#include <string>
#include <unordered_map>
#include <deque>
#include <iostream>
#include <cstdint>
#include <algorithm>

export module Portfolio;

export struct PositionLot {
    uint32_t quantity;
    uint32_t purchasePrice;
};

export class Account {
private:
    int64_t cashBalance;
    int64_t realizedPnL;

    std::unordered_map<std::string, std::deque<PositionLot>> positions;

public:
    explicit Account(int64_t initialCash) : cashBalance(initialCash), realizedPnL(0) {}

    int64_t getCashBalance() const { return cashBalance; }
    int64_t getRealizedPnL() const { return realizedPnL; }

    uint32_t getTotalPosition(const std::string& symbol) {
        uint32_t total = 0;
        auto it = positions.find(symbol);
        if (it != positions.end()) {
            for (const auto& lot : it->second) {
                total += lot.quantity;
            }
        }
        return total;
    }

    void processTrade(const std::string& symbol, bool isBuy, uint32_t price, uint32_t qty) {
        if (isBuy) {
            int64_t cost = static_cast<int64_t>(price) * qty;
            cashBalance -= cost;
            positions[symbol].push_back({ qty, price });
        }
        else {
            int64_t revenue = static_cast<int64_t>(price) * qty;
            cashBalance += revenue;

            uint32_t remainingToSell = qty;
            auto& lots = positions[symbol];

            while (remainingToSell > 0 && !lots.empty()) {
                auto& oldestLot = lots.front();

                uint32_t soldFromLot = std::min(remainingToSell, oldestLot.quantity);

                int64_t pnl = static_cast<int64_t>(soldFromLot) * (static_cast<int64_t>(price) - oldestLot.purchasePrice);
                realizedPnL += pnl;

                oldestLot.quantity -= soldFromLot;
                remainingToSell -= soldFromLot;

                if (oldestLot.quantity == 0) {
                    lots.pop_front();
                }
            }

            if (remainingToSell > 0) {
                std::cout << "[ACCOUNT] WARNING: Sold " << remainingToSell
                    << " more units of " << symbol << " than owned. (Short Position)\n";
            }
        }
    }
};