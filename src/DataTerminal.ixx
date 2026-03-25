module;

#include <iostream>
#include <string>
#include <filesystem>
#include <regex>
#include <fstream>

export module DataTerminal;

export class DataTerminal {
private:
    std::filesystem::path logDirectory;
    std::string logFileName;

public:
    DataTerminal() : logDirectory("exchange_logs"), logFileName("system_events.log") {}

    bool initializeStorage() {
        std::cout << "[DATA_TERMINAL] Verifying storage integrity...\n";

        if (!std::filesystem::exists(logDirectory)) {
            std::cout << "[DATA_TERMINAL] Log directory not found. Creating '" << logDirectory.string() << "'...\n";
            std::filesystem::create_directories(logDirectory);
        }

        std::filesystem::path fullPath = logDirectory / logFileName;

        std::ofstream logFile(fullPath, std::ios::app);
        if (logFile.is_open()) {
            logFile << "--- SYSTEM BOOT ---\n";
            logFile.close();
            std::cout << "[DATA_TERMINAL] Storage ready at: " << std::filesystem::absolute(fullPath).string() << "\n";
            return true;
        }

        std::cout << "[DATA_TERMINAL] ERROR: Failed to access storage.\n";
        return false;
    }

    void executeAdminCommand(const std::string& commandStr) {
        std::cout << "[DATA_TERMINAL] Received command: '" << commandStr << "'\n";

        std::regex cancelPattern(R"(^CANCEL\s+(\d+)$)");
        std::smatch match; 

        if (std::regex_match(commandStr, match, cancelPattern)) {
            std::string orderIdStr = match[1].str();
            std::cout << "[DATA_TERMINAL] SUCCESS: Command parsed. Executing cancellation for Order ID: " << orderIdStr << "\n";

            // Deleting order from OrderBook logic
        }
        else {
            std::cout << "[DATA_TERMINAL] ERROR: Invalid command format. Access denied.\n";
        }
    }
};