#pragma once

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include "DeribitSubscription.hpp"

// For convenience and readability
using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

/**
 * @class DeribitAuth
 * @brief Main class for handling Deribit exchange authentication and trading operations
 * 
 * This class manages WebSocket connections, API authentication, order management,
 * and market data operations with the Deribit cryptocurrency exchange.
 */
class DeribitAuth {
public:
    /**
     * @brief Constructor initializes exchange client with API credentials
     * @param client_id The Deribit API client ID
     * @param client_secret The Deribit API client secret
     */
    DeribitAuth(const std::string& client_id, const std::string& client_secret);

    // Connection and Authentication Methods
    bool connect();                                  // Establishes WebSocket connection to Deribit
    bool authenticate();                             // Authenticates using provided credentials
    std::string getAccessToken() const { 
        return access_token; 
    }

    // Trading Operations
    /**
     * @brief Places a buy order on the exchange
     * @param instrument_name Trading instrument (e.g., "BTC-PERPETUAL")
     * @param amount Order size/quantity
     * @param type Order type (market/limit)
     * @param label Optional order identifier
     */
    bool placeBuyOrder(const std::string& instrument_name, double amount, 
                      const std::string& type = "market", 
                      const std::string& label = "");

    /**
     * @brief Cancels an existing order
     * @param order_id The ID of the order to cancel
     */
    bool cancelOrder(const std::string& order_id);

    /**
     * @brief Modifies parameters of an existing order
     * @param order_id The ID of the order to modify
     * @param amount New order amount
     * @param price New order price
     * @param advanced Optional advanced order parameters
     */
    bool editOrder(const std::string& order_id, double amount, 
                  double price, const std::string& advanced = "");

    // Market Data Operations
    bool getOrderBook(const std::string& instrument_name, int depth = 5);  // Retrieves order book data
    bool getPosition(const std::string& instrument_name);                   // Gets current position info
    bool getOpenOrders();                                                  // Lists all open orders

    // Subscription and Timing Management
    DeribitSubscription& getSubscriptionHandler() { 
        return subscription_handler; 
    }
    
    void setTradingLoopStart() {
        trading_loop_start = std::chrono::high_resolution_clock::now();
    }

private:
    // WebSocket client type definitions
    typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

    // Performance measurement points
    std::chrono::high_resolution_clock::time_point trading_loop_start;    // Start of trading loop
    std::chrono::high_resolution_clock::time_point order_start_time;      // Order placement timing

    // WebSocket Event Handlers
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_error(websocketpp::connection_hdl hdl);
    context_ptr on_tls_init();

    // Member variables
    client ws_client;                               // WebSocket client instance
    websocketpp::connection_hdl connection_hdl;     // Active connection handle
    std::string client_id;                         // Deribit API client ID
    std::string client_secret;                     // Deribit API client secret
    std::string access_token;                      // Current session access token
    std::string refresh_token;                     // Token for refreshing session
    bool connected;                                // WebSocket connection status
    bool authenticated;                            // API authentication status
    DeribitSubscription subscription_handler;      // Market data subscription manager
};
