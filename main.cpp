// Standard includes and DeribitAuth header
#include "DeribitAuth.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>

// UI Color definitions
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"

// Test details:
// client-id: oROD-D2E
// client-secret: hxMWiy9BYaAjP5sfOhNuoOco1GdgS1yteVakioCRI_8

/**
 * System Configuration
 * Supported currencies for trading operations
 * Note: Make sure these match with Deribit's available instruments
 */
const std::vector<std::string> SUPPORTED_CURRENCIES = {
    "BTC", "ETH", "AVAX", "BNB", "ADA", 
    "DOGE", "PAXG", "XRP", "SOL"
};

/**
 * UI Helper Functions
 * These functions handle the user interface and command presentation
 */

// Enhanced welcome message with color and formatting
void printWelcomeMessage() {
    std::cout << BLUE << "\n╔════════════════════════════════════╗" << std::endl
              << "║  Deribit Trading Management System ║" << std::endl
              << "╚════════════════════════════════════╝" << RESET << std::endl
              << "\nAvailable Commands:" << std::endl
              << GREEN << std::setw(15) << std::left << "  auth" << RESET << " - Connect to your Deribit account\n"
              << GREEN << std::setw(15) << std::left << "  buy" << RESET << " - Place a new market buy order\n"
              << GREEN << std::setw(15) << std::left << "  cancel" << RESET << " - Cancel an existing order\n"
              << GREEN << std::setw(15) << std::left << "  edit" << RESET << " - Modify an existing order\n"
              << GREEN << std::setw(15) << std::left << "  orderbook" << RESET << " - View market orderbook\n"
              << GREEN << std::setw(15) << std::left << "  position" << RESET << " - Check your positions\n"
              << GREEN << std::setw(15) << std::left << "  orders" << RESET << " - List your open orders\n"
              << GREEN << std::setw(15) << std::left << "  subscribe" << RESET << " - Subscribe to market data\n"
              << GREEN << std::setw(15) << std::left << "  unsubscribe" << RESET << " - Unsubscribe from data\n"
              << GREEN << std::setw(15) << std::left << "  help" << RESET << " - Show this menu\n"
              << GREEN << std::setw(15) << std::left << "  exit" << RESET << " - Exit the program\n"
              << BLUE << "\n════════════════════════════════════\n" << RESET;
}

// Helper function to display all supported currencies
void printSupportedCurrencies() {
    std::cout << "Supported currencies: {";
    for (size_t i = 0; i < SUPPORTED_CURRENCIES.size(); ++i) {
        std::cout << "\"" << SUPPORTED_CURRENCIES[i] << "\"";
        if (i < SUPPORTED_CURRENCIES.size() - 1)
            std::cout << ", ";
    }
    std::cout << "}" << std::endl;
}

/**
 * Authentication Check
 * @param auth Pointer to DeribitAuth instance
 * @return true if authenticated, false otherwise
 */
bool checkAuth(DeribitAuth* auth) {
    if (auth == nullptr) {
        std::cout << RED << "Error: Please authenticate first using 'auth' command.\n" << RESET;
        return false;
    }
    return true;
}

int main() {
    // Initialize system state
    std::string client_id, client_secret;
    DeribitAuth* auth = nullptr;

    // Setup initial UI state
    printWelcomeMessage();
    printSupportedCurrencies();

    // Main Command Processing Loop
    while (true) {
        std::cout << YELLOW << "\n➤ " << RESET;
        std::string command;
        std::getline(std::cin, command);

        // Trim whitespace from command
        command.erase(0, command.find_first_not_of(" \t\n\r\f\v"));
        command.erase(command.find_last_not_of(" \t\n\r\f\v") + 1);

        // Handle empty commands
        if (command.empty()) {
            continue;
        }

        // Convert command to lowercase for case-insensitive comparison
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        /**
         * Command Handlers
         * Each section handles a specific trading or system command
         */
        
        // System Commands
        if (command == "help") {
            printWelcomeMessage();
            continue;
        }
        else if (command == "auth") {
            std::cout << BLUE << "\n=== Authentication ===" << RESET << std::endl;

            // Get credentials
            std::cout << "Enter client_id: ";
            std::getline(std::cin, client_id);
            std::cout << "Enter client_secret: ";
            std::getline(std::cin, client_secret);

            // Cleanup and initialize auth
            if (auth != nullptr) delete auth;
            auth = new DeribitAuth(client_id, client_secret);

            // Attempt connection and authentication
            std::cout << "Attempting to connect to Deribit..." << std::endl;

            if (!auth->connect()) {
                std::cerr << RED << "Failed to connect to Deribit." << RESET << std::endl;
                continue;
            }

            std::cout << "Attempting to authenticate..." << std::endl;
            if (auth->authenticate()) {
                std::cout << GREEN << "Authentication successful!" << RESET << std::endl;
                std::cout << "Access token: " << auth->getAccessToken() << std::endl;
            } else {
                std::cerr << RED << "Authentication failed." << RESET << std::endl;
            }
        }
        else if (command == "exit") {
            std::cout << GREEN << "\nThank you for using Deribit Trading Management System.\n"
                      << "Cleaning up and exiting...\n" << RESET;
            if (auth != nullptr) {
                delete auth;
            }
            break;
        }
        
        // Trading Commands
        else if (command == "buy") {
            std::cout << BLUE << "\n=== New Market Buy Order ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::string instrument;
            double amount;
            std::cout << "Enter instrument name (e.g., ETH-PERPETUAL): ";
            std::getline(std::cin, instrument);
            std::cout << "Enter amount: ";
            std::cin >> amount;
            std::cin.ignore(); // Clear newline
            auth->setTradingLoopStart();
            if (auth->placeBuyOrder(instrument, amount, "market")) {
                std::cout << GREEN << "Buy order request sent." << RESET << std::endl;
            }
        }
        else if (command == "cancel") {
            std::cout << BLUE << "\n=== Cancel Order ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::string order_id;
            std::cout << "Enter order ID to cancel: ";
            std::getline(std::cin, order_id);
            if (auth->cancelOrder(order_id)) {
                std::cout << GREEN << "Cancel order request sent." << RESET << std::endl;
            }
        }
        else if (command == "edit") {
            std::cout << BLUE << "\n=== Edit Order ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::string order_id;
            double amount, price;
            std::string advanced;

            std::cout << "Enter order ID to modify: ";
            std::getline(std::cin, order_id);

            std::cout << "Enter new amount: ";
            std::cin >> amount;

            std::cout << "Enter new price: ";
            std::cin >> price;
            std::cin.ignore(); // Clear newline

            std::cout << "Enter advanced parameter (or press enter to skip): ";
            std::getline(std::cin, advanced);

            if (auth->editOrder(order_id, amount, price, advanced)) {
                std::cout << GREEN << "Edit order request sent." << RESET << std::endl;
            }
        }
        
        // Market Data Commands
        else if (command == "orderbook") {
            std::cout << BLUE << "\n=== Orderbook Request ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::string instrument;
            int depth;
            std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
            std::getline(std::cin, instrument);
            std::cout << "Enter depth (1,5,10,20,50,100,1000,10000): ";
            std::cin >> depth;
            std::cin.ignore(); // Clear newline
            if (auth->getOrderBook(instrument, depth)) {
                std::cout << GREEN << "Get orderbook request sent." << RESET << std::endl;
            }
        }
        else if (command == "position") {
            std::cout << BLUE << "\n=== Position Request ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::string instrument;
            std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
            std::getline(std::cin, instrument);
            if (auth->getPosition(instrument)) {
                std::cout << GREEN << "Get position request sent." << RESET << std::endl;
            }
        }
        else if (command == "orders") {
            std::cout << BLUE << "\n=== Open Orders Request ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            if (auth->getOpenOrders()) {
                std::cout << GREEN << "Get open orders request sent." << RESET << std::endl;
            }
        }
        
        // Subscription Commands
        else if (command == "subscribe") {
            std::cout << BLUE << "\n=== Subscribe to Channels ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            // Get and validate subscription type
            std::string channel_type;
            std::cout << "Enter channel type (public/private): ";
            std::getline(std::cin, channel_type);

            std::vector<std::string> channels;
            std::string channel;

            // Display available channels based on type
            if (channel_type == "public") {
                std::cout << "Available public channels:" << std::endl;
                std::cout << "  - announcements" << std::endl;
                std::cout << "  - trades.<kind>.<currency>.<interval>" << std::endl;
                std::cout << "  - book.<instrument_name>.<interval>" << std::endl;
                std::cout << "  - instrument.state.any.any" << std::endl;
                std::cout << "  - deribit_price_index.<index_name>" << std::endl;
            } else if (channel_type == "private") {
                std::cout << "Available private channels:" << std::endl;
                std::cout << "  - user.changes.<kind>.<currency>" << std::endl;
                std::cout << "  - user.orders.<kind>.<currency>" << std::endl;
                std::cout << "  - user.trades.<kind>.<currency>" << std::endl;
                std::cout << "  - user.portfolio.<currency>" << std::endl;
                std::cout << "  - block_rfq.maker.btc" << std::endl;
                std::cout << "  - block_rfq.maker.eth" << std::endl;
                std::cout << "  - block_rfq.maker.usdc" << std::endl;
                std::cout << "  - block_rfq.taker.btc" << std::endl;
                std::cout << "  - block_rfq.taker.eth" << std::endl;
                std::cout << "  - block_rfq.taker.usdc" << std::endl;
                std::cout << "  - block_rfq.maker.quotes.<your-desired-trade>" << std::endl;
            } else {
                std::cout << RED << "Invalid channel type. Must be 'public' or 'private'." << RESET << std::endl;
                continue;
            }

            // Channel collection loop
            while (true) {
                std::cout << "> ";
                std::getline(std::cin, channel);
                if (channel == "done") break;
                channels.push_back(channel);
            }

            // Process subscription
            auto& subscription = auth->getSubscriptionHandler();
            if (channel_type == "public") {
                if (subscription.subscribePublic(channels)) {
                    std::cout << GREEN << "Public subscription request sent for channels: " << RESET;
                    for (const auto& ch : channels) std::cout << ch << " ";
                    std::cout << std::endl;
                }
            } else if (channel_type == "private") {
                if (subscription.subscribePrivate(channels)) {
                    std::cout << GREEN << "Private subscription request sent for channels: " << RESET;
                    for (const auto& ch : channels) std::cout << ch << " ";
                    std::cout << std::endl;
                }
            }
        }
        else if (command == "unsubscribe") {
            std::cout << BLUE << "\n=== Unsubscribe from Channels ===" << RESET << std::endl;
            if (!checkAuth(auth)) continue;

            std::vector<std::string> channels;
            std::string channel;
            std::cout << "Enter channel names to unsubscribe (or 'done' to finish):" << std::endl;
            while (true) {
                std::getline(std::cin, channel);
                if (channel == "done") break;
                channels.push_back(channel);
            }

            auto& subscription = auth->getSubscriptionHandler();
            if (subscription.unsubscribe(channels)) {
                std::cout << GREEN << "Unsubscribe request sent." << RESET << std::endl;
            }
        }
        
        // Invalid Command Handler
        else {
            std::cout << RED << "\nError: Unknown command '" << command << "'\n" << RESET
                      << "Type " << GREEN << "help" << RESET << " to see available commands.\n";
        }
    }

    return 0;
}