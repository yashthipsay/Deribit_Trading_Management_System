#include "DeribitAuth.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

// Constructor: initialize client credentials and connection flags.
DeribitAuth::DeribitAuth(const std::string& client_id, const std::string& client_secret)
: client_id(client_id), 
client_secret(client_secret), 
connected(false), 
authenticated(false),
subscription_handler(ws_client, connection_hdl, authenticated) {
std::cout << "DeribitAuth object created with client_id: " << client_id << std::endl;
}

// TLS initialization callback
websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>
DeribitAuth::on_tls_init() {
    std::cout << "Initializing TLS..." << std::endl;
    auto ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(
        websocketpp::lib::asio::ssl::context::sslv23);
    try {
        ctx->set_options(websocketpp::lib::asio::ssl::context::default_workarounds |
                         websocketpp::lib::asio::ssl::context::no_sslv2 |
                         websocketpp::lib::asio::ssl::context::no_sslv3 |
                         websocketpp::lib::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cerr << "Error in TLS initialization: " << e.what() << std::endl;
    }
    return ctx;
}

// Connect to Deribit's test WebSocket endpoint.
bool DeribitAuth::connect() {
    std::cout << "Connecting to Deribit WebSocket..." << std::endl;
    std::string uri = "wss://test.deribit.com/ws/api/v2";

    // Disable logging for clarity (enable if you need to debug)
    ws_client.clear_access_channels(websocketpp::log::alevel::all);
    ws_client.clear_error_channels(websocketpp::log::elevel::all);

    // Initialize ASIO and set handlers.
    ws_client.init_asio();
    ws_client.set_tls_init_handler(std::bind(&DeribitAuth::on_tls_init, this));
    ws_client.set_open_handler(std::bind(&DeribitAuth::on_open, this, _1));
    ws_client.set_message_handler(std::bind(&DeribitAuth::on_message, this, _1, _2));
    ws_client.set_close_handler(std::bind(&DeribitAuth::on_close, this, _1));
    ws_client.set_fail_handler(std::bind(&DeribitAuth::on_error, this, _1));

    // Create connection.
    websocketpp::lib::error_code ec;
    auto con = ws_client.get_connection(uri, ec);
    if (ec) {
        std::cerr << "Connection creation failed: " << ec.message() << std::endl;
        return false;
    }
    connection_hdl = con->get_handle();
    ws_client.connect(con);

    // Run the ASIO event loop in a background thread.
    std::thread([this]() { ws_client.run(); }).detach();

    // Wait until the connection is established (with a timeout).
    int timeout = 10;
    while (!connected && timeout--) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!connected) {
        std::cerr << "Failed to connect within timeout period." << std::endl;
    } else {
        std::cout << "Connection established successfully." << std::endl;
    }
    return connected;
}

// Handler for when the connection is opened.
void DeribitAuth::on_open(websocketpp::connection_hdl hdl) {
    connected = true;
    std::cout << "Connected to Deribit WebSocket." << std::endl;
}



// Message handler: parse authentication response.
void DeribitAuth::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    try {
        auto j = json::parse(msg->get_payload());
        // Check if the response contains the "result" field.
        if (j.contains("result")) {
            auto result = j["result"];
            if (result.contains("access_token")) {
                access_token = result["access_token"];
                refresh_token = result["refresh_token"];
                authenticated = true;
                std::cout << "Authenticated successfully." << std::endl;
                std::cout << "Access Token: " << access_token << std::endl;
                
            }
            else if (result.contains("order") && j["id"] == 5275) {
                auto order = result["order"];
                auto order_end_time = std::chrono::high_resolution_clock::now();
                auto order_latency = std::chrono::duration_cast<std::chrono::microseconds>(
                    order_end_time - order_start_time).count();
                auto loop_latency = std::chrono::duration_cast<std::chrono::microseconds>(
                    order_end_time - trading_loop_start).count();
                std::cout << "Order Placement Latency: " << order_latency << " microseconds" << std::endl;
                std::cout << "Order placed successfully:" << std::endl;
                std::cout << "Order ID: " << order["order_id"] << std::endl;
                std::cout << "Amount: " << order["amount"] << std::endl;
                std::cout << "Average Price: " << order["average_price"] << std::endl;
                std::cout << "State: " << order["order_state"] << std::endl;
            }
            else if (result.contains("order_id")) {
                std::cout << "Order cancelled successfully:" << std::endl;
                std::cout << "Order ID: " << result["order_id"] << std::endl;
                std::cout << "State: " << result["order_state"] << std::endl;
            }
            else if (result.contains("order")) {
                auto order = result["order"];
                std::cout << "Order updated successfully:" << std::endl;
                std::cout << "Order ID: " << order["order_id"] << std::endl;
                std::cout << "Amount: " << order["amount"] << std::endl;
                std::cout << "Price: " << order["price"] << std::endl;
                if (order.contains("advanced")) {
                    std::cout << "Advanced: " << order["advanced"] << std::endl;
                }
                std::cout << "State: " << order["order_state"] << std::endl;
            }
            else if (j["id"] == 1953) {  // Get open orders response
                auto result = j["result"];
                if (result.empty()) {
                    std::cout << "No open orders found." << std::endl;
                    return;
                }
                
                std::cout << "\n=== Open Orders ===" << std::endl;
                for (const auto& order : result) {
                    std::cout << "Order ID: " << order["order_id"] << std::endl;
                    std::cout << "Instrument: " << order["instrument_name"] << std::endl;
                    std::cout << "Type: " << order["order_type"] << std::endl;
                    std::cout << "Direction: " << order["direction"] << std::endl;
                    std::cout << "Amount: " << order["amount"] << std::endl;
                    std::cout << "Filled Amount: " << order["filled_amount"] << std::endl;
                    std::cout << "Price: " << order["price"] << std::endl;
                    std::cout << "State: " << order["order_state"] << std::endl;
                    std::cout << "Time in Force: " << order["time_in_force"] << std::endl;
                    std::cout << "Created: " << order["creation_timestamp"] << std::endl;
                    std::cout << "Last Update: " << order["last_update_timestamp"] << std::endl;
                    
                    if (order.contains("label") && !order["label"].empty()) {
                        std::cout << "Label: " << order["label"] << std::endl;
                    }
                    
                    std::cout << "Reduce Only: " << (order["reduce_only"].get<bool>() ? "Yes" : "No") << std::endl;
                    std::cout << "Post Only: " << (order["post_only"].get<bool>() ? "Yes" : "No") << std::endl;
                    
                    if (order.contains("trigger_price")) {
                        std::cout << "Trigger Price: " << order["trigger_price"] << std::endl;
                    }
                    
                    std::cout << "---------------------" << std::endl;
                }
                std::cout << "===================" << std::endl;
            }
            else if (result.contains("instrument_name") && result.contains("bids") && result.contains("asks")) {
                std::cout << "Order book for " << result["instrument_name"].get<std::string>() << ":" << std::endl;
                std::cout << "Mark Price: " << result["mark_price"].get<double>() << std::endl;
                std::cout << "Last Price: " << result["last_price"].get<double>() << std::endl;
                
                std::cout << "\nBids:" << std::endl;
                for (const auto& bid : result["bids"]) {
                    std::cout << "Price: " << bid[0].get<double>() 
                              << ", Amount: " << bid[1].get<double>() << std::endl;
                }
                
                std::cout << "\nAsks:" << std::endl;
                for (const auto& ask : result["asks"]) {
                    std::cout << "Price: " << ask[0].get<double>() 
                              << ", Amount: " << ask[1].get<double>() << std::endl;
                }
            }
           // Handle position response
         else if (result.contains("instrument_name") && result.contains("size")) {
               std::cout << "\nPosition Details for " << result["instrument_name"].get<std::string>() << ":" << std::endl;
               std::cout << "Size: " << result["size"].get<double>() << std::endl;
               std::cout << "Direction: " << result["direction"].get<std::string>() << std::endl;
               std::cout << "Average Price: " << result["average_price"].get<double>() << std::endl;
               std::cout << "Floating P/L: " << result["floating_profit_loss"].get<double>() << std::endl;
               std::cout << "Realized P/L: " << result["realized_profit_loss"].get<double>() << std::endl;
               std::cout << "Total P/L: " << result["total_profit_loss"].get<double>() << std::endl;
               std::cout << "Leverage: " << result["leverage"].get<int>() << std::endl;
               
               if (result["estimated_liquidation_price"].get<double>() > 0) {
                   std::cout << "Est. Liquidation Price: " << result["estimated_liquidation_price"].get<double>() << std::endl;
               }
           }
             // Handle subscription messages
        // Route subscription messages to subscription handler
       else if ((j.contains("method") && j["method"] == "subscription") ||
            (j.contains("result") && j["id"] == 42)) {  // ID 42 is used for subscriptions
            subscription_handler.handleSubscriptionMessage(j);
            return;
        }
        } else if (j.contains("error")) {
            std::cerr << "Authentication error: " << j["error"].dump() << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
    }
}

// Handler for connection close events.
void DeribitAuth::on_close(websocketpp::connection_hdl hdl) {
    connected = false;
    std::cout << "Connection closed." << std::endl;
}

// Handler for connection failures.
void DeribitAuth::on_error(websocketpp::connection_hdl hdl) {
    std::cerr << "Connection error encountered." << std::endl;
}

// Send an authentication request using client_credentials.
bool DeribitAuth::authenticate() {
    if (!connected) {
        std::cerr << "Not connected to Deribit server." << std::endl;
        return false;
    }

    // Create JSON-RPC authentication message.
    json j;
    j["jsonrpc"] = "2.0";
    j["id"] = 1; // Request identifier.
    j["method"] = "public/auth";
    j["params"] = {
        {"grant_type", "client_credentials"},
        {"client_id", client_id},
        {"client_secret", client_secret}
    };

    std::string payload = j.dump();
    websocketpp::lib::error_code ec;
    ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "Error sending authentication request: " << ec.message() << std::endl;
        return false;
    }

    // Wait for authentication to complete (with a timeout).
    int waitTime = 10;
    while (!authenticated && waitTime--) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!authenticated) {
        std::cerr << "Authentication timed out." << std::endl;
    }
    return authenticated;
}


bool DeribitAuth::placeBuyOrder(const std::string& instrument_name, double amount,
    const std::string& type, const std::string& label) {
if (!authenticated) {
std::cerr << "Not authenticated. Please authenticate first." << std::endl;
return false;
}

    // Record order start time
    order_start_time = std::chrono::high_resolution_clock::now();

// Create JSON-RPC buy order message
json j;
j["jsonrpc"] = "2.0";
j["id"] = 5275;  // Request identifier
j["method"] = "private/buy";
j["params"] = {
{"instrument_name", instrument_name},
{"amount", amount},
{"type", type}
};

// Add label if provided
if (!label.empty()) {
j["params"]["label"] = label;
}

std::string payload = j.dump();
std::cout << "Sending buy order: " << payload << std::endl;

websocketpp::lib::error_code ec;
ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);

if (ec) {
std::cerr << "Error sending buy order request: " << ec.message() << std::endl;
return false;
}

return true;
}

bool DeribitAuth::editOrder(const std::string& order_id, double amount, 
    double price, const std::string& advanced) {
        if (!authenticated) {
            std::cerr << "Not authenticated. Please authenticate first." << std::endl;
            return false;
        }

        // Create JSON-RPC edit order message
        json j;
        j["jsonrpc"] = "2.0";
        j["id"] = 3725;  // Request identifier
        j["method"] = "private/edit";
        j["params"] = {
            {"order_id", order_id},
            {"amount", amount},
            {"price", price}
        };

        // Add advanced parameter if provided
        if (!advanced.empty()) {
            j["params"]["advanced"] = advanced;
        }

        std::string payload = j.dump();
        std::cout << "Sending edit order request: " << payload << std::endl;

        websocketpp::lib::error_code ec;
        ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);

        if (ec) {
            std::cerr << "Error sending edit order request: " << ec.message() << std::endl;
            return false;
        }
    
        return true;
    }

    bool DeribitAuth::cancelOrder(const std::string& order_id) {
        if (!authenticated) {
            std::cerr << "Not authenticated. Please authenticate first." << std::endl;
            return false;
        }
    
        // Create JSON-RPC cancel order message
        json j;
        j["jsonrpc"] = "2.0";
        j["id"] = 4214;  // Request identifier
        j["method"] = "private/cancel";
        j["params"] = {
            {"order_id", order_id}
        };
    
        std::string payload = j.dump();
        std::cout << "Sending cancel order request: " << payload << std::endl;
    
        websocketpp::lib::error_code ec;
        ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);
    
        if (ec) {
            std::cerr << "Error sending cancel order request: " << ec.message() << std::endl;
            return false;
        }
    
        return true;
    }

    bool DeribitAuth::getOrderBook(const std::string& instrument_name, int depth) {
        // Create JSON-RPC get orderbook message
        json j;
        j["jsonrpc"] = "2.0";
        j["id"] = 8772;  // Request identifier
        j["method"] = "public/get_order_book";
        j["params"] = {
            {"instrument_name", instrument_name},
            {"depth", depth}
        };
    
        std::string payload = j.dump();
        std::cout << "Sending get orderbook request: " << payload << std::endl;
    
        websocketpp::lib::error_code ec;
        ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);
    
        if (ec) {
            std::cerr << "Error sending get orderbook request: " << ec.message() << std::endl;
            return false;
        }
    
        return true;
    }

    bool DeribitAuth::getPosition(const std::string& instrument_name) {
        if (!authenticated) {
            std::cerr << "Not authenticated. Please authenticate first." << std::endl;
            return false;
        }
    
        // Create JSON-RPC get position message
        json j;
        j["jsonrpc"] = "2.0";
        j["id"] = 404;  // Request identifier
        j["method"] = "private/get_position";
        j["params"] = {
            {"instrument_name", instrument_name}
        };
    
        std::string payload = j.dump();
        std::cout << "Sending get position request: " << payload << std::endl;
    
        websocketpp::lib::error_code ec;
        ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);
    
        if (ec) {
            std::cerr << "Error sending get position request: " << ec.message() << std::endl;
            return false;
        }
    
        return true;
    }

    bool DeribitAuth::getOpenOrders() {
        if (!authenticated) {
            std::cerr << "Not authenticated. Please authenticate first." << std::endl;
            return false;
        }
    
        // Create JSON-RPC get open orders message
        json j;
        j["jsonrpc"] = "2.0";
        j["id"] = 1953;  // Request identifier
        j["method"] = "private/get_open_orders";
        j["params"] = json::object();  // Empty params object
    
        std::string payload = j.dump();
        std::cout << "Sending get open orders request: " << payload << std::endl;
    
        websocketpp::lib::error_code ec;
        ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);
    
        if (ec) {
            std::cerr << "Error sending get open orders request: " << ec.message() << std::endl;
            return false;
        }
    
        return true;
    }



