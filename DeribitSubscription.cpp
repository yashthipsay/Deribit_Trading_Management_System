#include "DeribitSubscription.hpp"
#include <iostream>

DeribitSubscription::DeribitSubscription(
    websocketpp::client<websocketpp::config::asio_tls_client>& ws_client,
    websocketpp::connection_hdl& conn_hdl,
    bool& auth_status)
    : ws_client(ws_client), connection_hdl(conn_hdl), authenticated(auth_status) {
}

bool DeribitSubscription::subscribePublic(const std::vector<std::string>& channels) {
    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "public/subscribe";
    j["id"] = 42;
    j["params"] = {
        {"channels", channels}
    };

    std::cout << "Subscribing to channels: ";
    for (const auto& channel : channels) {
        std::cout << channel << " ";
    }
    std::cout << std::endl;

    return sendSubscriptionMessage(j);
}

bool DeribitSubscription::subscribePrivate(const std::vector<std::string>& channels) {
    if (!authenticated) {
        std::cerr << "Not authenticated. Please authenticate first." << std::endl;
        return false;
    }

    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "private/subscribe";
    j["id"] = 42;
    j["params"] = {
        {"channels", channels}
    };

    return sendSubscriptionMessage(j);
}

bool DeribitSubscription::unsubscribe(const std::vector<std::string>& channels) {
    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "public/unsubscribe";
    j["id"] = 42; // Since we are using a single channel, we can use the same ID 
    j["params"] = {
        {"channels", channels}
    };

    return sendSubscriptionMessage(j);
}

bool DeribitSubscription::sendSubscriptionMessage(const json& j) {
    std::string payload = j.dump();
    std::cout << "Sending subscription request: " << payload << std::endl;

    websocketpp::lib::error_code ec;
    ws_client.send(connection_hdl, payload, websocketpp::frame::opcode::text, ec);

    if (ec) {
        std::cerr << "Error sending subscription request: " << ec.message() << std::endl;
        return false;
    }

    return true;
}

void DeribitSubscription::handleSubscriptionMessage(const json& message) {
    try {
        std::cout << "Raw message: " << message.dump(2) << std::endl;
        // Check if this is a subscription confirmation
        if (message.contains("result")) {
            if (message["result"].is_null()) {
                std::cerr << "Subscription failed or returned null result" << std::endl;
                return;
            }
            
            std::cout << "Subscription confirmed for channels: ";
            if (message["result"].is_array()) {
                for (const auto& channel : message["result"]) {
                    std::cout << channel << " ";
                    active_subscriptions.push_back(channel);
                }
            } else {
                std::cout << message["result"].dump();
            }
            std::cout << std::endl;
            return;
        }

        // Handle subscription updates
        if (message.contains("method") && message["method"] == "subscription") {
            auto params = message["params"];
            std::string channel = params["channel"].get<std::string>();
            std::cout << "\nReceived update for channel: " << channel << std::endl;
            
            if (channel == "announcements") {
                auto data = params["data"];
                std::cout << "=== Announcement ===" << std::endl;
                std::cout << "Title: " << data["title"].get<std::string>() << std::endl;
                std::cout << "Body: " << data["body"].get<std::string>() << std::endl;
                std::cout << "Important: " << (data["important"].get<bool>() ? "Yes" : "No") << std::endl;
                std::cout << "Action: " << data["action"].get<std::string>() << std::endl;
                std::cout << "Publication Time: " << data["publication_timestamp"].get<long>() << std::endl;
                std::cout << "=================" << std::endl;
            }
            else if (channel.find("block_rfq.maker.") != std::string::npos) {
                auto data = params["data"];
                std::cout << "=== Block RFQ ===" << std::endl;
                std::cout << "Block RFQ ID: " << data["block_rfq_id"] << std::endl;
                std::cout << "State: " << data["state"] << std::endl;
                std::cout << "Role: " << data["role"] << std::endl;
                std::cout << "Amount: " << data["amount"] << std::endl;
                std::cout << "Taker Rating: " << data["taker_rating"] << std::endl;
                std::cout << "Creation Time: " << data["creation_timestamp"] << std::endl;
                std::cout << "Expiration Time: " << data["expiration_timestamp"] << std::endl;
                std::cout << "Combo ID: " << data["combo_id"] << std::endl;

                if (data.contains("legs")) {
                    std::cout << "\nLegs:" << std::endl;
                    for (const auto& leg : data["legs"]) {
                        std::cout << "  Instrument: " << leg["instrument_name"] << std::endl;
                        std::cout << "  Direction: " << leg["direction"] << std::endl;
                        std::cout << "  Ratio: " << leg["ratio"] << std::endl;
                    }
                }
                std::cout << "=====================" << std::endl;
            }
            else if (channel.find("block_rfq.maker.quotes") != std::string::npos) {
                auto data = params["data"];
                for (const auto& quote : data) {
                    std::cout << "=== Block RFQ Quote ===" << std::endl;
                    std::cout << "Block RFQ Quote ID: " << quote["block_rfq_quote_id"] << std::endl;
                    std::cout << "Block RFQ ID: " << quote["block_rfq_id"] << std::endl;
                    std::cout << "Direction: " << quote["direction"] << std::endl;
                    std::cout << "Amount: " << quote["amount"] << std::endl;
                    std::cout << "Price: " << quote["price"] << std::endl;
                    std::cout << "State: " << quote["quote_state"] << std::endl;
                    
                    if (quote.contains("legs")) {
                        std::cout << "\nLegs:" << std::endl;
                        for (const auto& leg : quote["legs"]) {
                            std::cout << "  Instrument: " << leg["instrument_name"] << std::endl;
                            std::cout << "  Direction: " << leg["direction"] << std::endl;
                            std::cout << "  Amount: " << leg["ratio"] << std::endl;
                            std::cout << "  Price: " << leg["price"] << std::endl;
                        }
                    }
                    
                    std::cout << "Creation Time: " << quote["creation_timestamp"] << std::endl;
                    std::cout << "Last Update: " << quote["last_update_timestamp"] << std::endl;
                    
                    if (quote.contains("label")) {
                        std::cout << "Label: " << quote["label"] << std::endl;
                    }
                    
                    std::cout << "Filled Amount: " << quote["filled_amount"] << std::endl;
                    std::cout << "Is Replaced: " << quote["replaced"].get<bool>() << std::endl;
                    std::cout << "=====================" << std::endl;
                }
            }
            else if (channel.find("user.trades.") != std::string::npos) {
                auto data = params["data"];
                for (const auto& trade : data) {
                    std::cout << "\n=== User Trade ===" << std::endl;
                    std::cout << "Trade ID: " << trade["trade_id"] << std::endl;
                    std::cout << "Instrument: " << trade["instrument_name"] << std::endl;
                    std::cout << "Direction: " << trade["direction"] << std::endl;
                    std::cout << "Amount: " << trade["amount"] << std::endl;
                    std::cout << "Price: " << trade["price"] << std::endl;
                    std::cout << "Mark Price: " << trade["mark_price"] << std::endl;
                    std::cout << "Index Price: " << trade["index_price"] << std::endl;
                    std::cout << "State: " << trade["state"] << std::endl;
                    std::cout << "Order Type: " << trade["order_type"] << std::endl;
                    std::cout << "Fee: " << trade["fee"] << " " << trade["fee_currency"] << std::endl;
                    std::cout << "Timestamp: " << trade["timestamp"] << std::endl;
                    std::cout << "===================" << std::endl;
                }
            }
                        // Add handler for trades channel
        else if (channel.find("trades.") != std::string::npos) {
            auto data = params["data"];
            for (const auto& trade : data) {
                std::cout << "\n=== Trade ===" << std::endl;
                std::cout << "Instrument: " << trade["instrument_name"] << std::endl;
                std::cout << "Trade ID: " << trade["trade_id"] << std::endl;
                std::cout << "Trade Sequence: " << trade["trade_seq"] << std::endl;
                std::cout << "Direction: " << trade["direction"] << std::endl;
                std::cout << "Amount: " << trade["amount"] << std::endl;
                std::cout << "Price: " << trade["price"] << std::endl;
                 std::cout << "Mark Price: " << trade["mark_price"] << std::endl;
                std::cout << "Index Price: " << trade["index_price"] << std::endl;
                std::cout << "Timestamp: " << trade["timestamp"] << std::endl;
                
                // Optional IV field for options
                if (trade.contains("iv")) {
                    std::cout << "Implied Volatility: " << trade["iv"] << "%" << std::endl;
                }
                
                std::cout << "Tick Direction: " << trade["tick_direction"] << std::endl;
                std::cout << "==================" << std::endl;
            }
        }
        else if (channel.find("block_rfq.taker.") != std::string::npos) {
            auto data = params["data"];
            std::cout << "\n=== Block RFQ Taker Update ===" << std::endl;
            std::cout << "Block RFQ ID: " << data["block_rfq_id"] << std::endl;
            std::cout << "State: " << data["state"] << std::endl;
            std::cout << "Role: " << data["role"] << std::endl;
            std::cout << "Amount: " << data["amount"] << std::endl;
            std::cout << "Min Trade Amount: " << data["min_trade_amount"] << std::endl;
            std::cout << "Taker Rating: " << data["taker_rating"] << std::endl;
            std::cout << "Creation Time: " << data["creation_timestamp"] << std::endl;
            std::cout << "Expiration Time: " << data["expiration_timestamp"] << std::endl;
            
            if (data.contains("makers") && !data["makers"].empty()) {
                std::cout << "\nMakers:" << std::endl;
                for (const auto& maker : data["makers"]) {
                    std::cout << "  " << maker << std::endl;
                }
            }

            if (data.contains("legs") && !data["legs"].empty()) {
                std::cout << "\nLegs:" << std::endl;
                for (const auto& leg : data["legs"]) {
                    std::cout << "  Instrument: " << leg["instrument_name"] << std::endl;
                    std::cout << "  Direction: " << leg["direction"] << std::endl;
                    std::cout << "  Ratio: " << leg["ratio"] << std::endl;
                }
            }

            // Handle bids if present
            if (data.contains("bids") && !data["bids"].empty()) {
                std::cout << "\nBids:" << std::endl;
                for (const auto& bid : data["bids"]) {
                    std::cout << "  Price: " << bid["price"] << std::endl;
                    std::cout << "  Amount: " << bid["amount"] << std::endl;
                    std::cout << "  Execution: " << bid["execution_instruction"] << std::endl;
                    std::cout << "  Last Update: " << bid["last_update_timestamp"] << std::endl;
                    if (bid.contains("makers")) {
                        std::cout << "  Makers: ";
                        for (const auto& maker : bid["makers"]) {
                            std::cout << maker << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }

            // Handle asks if present
            if (data.contains("asks") && !data["asks"].empty()) {
                std::cout << "\nAsks:" << std::endl;
                for (const auto& ask : data["asks"]) {
                    std::cout << "  Price: " << ask["price"] << std::endl;
                    std::cout << "  Amount: " << ask["amount"] << std::endl;
                    std::cout << "  Execution: " << ask["execution_instruction"] << std::endl;
                    std::cout << "  Last Update: " << ask["last_update_timestamp"] << std::endl;
                    if (ask.contains("makers")) {
                        std::cout << "  Makers: ";
                        for (const auto& maker : ask["makers"]) {
                            std::cout << maker << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }

            if (data.contains("label")) {
                std::cout << "\nLabel: " << data["label"] << std::endl;
            }
            
            std::cout << "Disclosed: " << (data["disclosed"].get<bool>() ? "Yes" : "No") << std::endl;
            
            if (data.contains("combo_id") && !data["combo_id"].is_null()) {
                std::cout << "Combo ID: " << data["combo_id"] << std::endl;
            }
            
            std::cout << "=============================" << std::endl;
        }

        else if (channel.find("block_rfq.taker.") != std::string::npos) {
            auto data = params["data"];
            std::cout << "\n=== Block RFQ Taker Update ===" << std::endl;
            std::cout << "Block RFQ ID: " << data["block_rfq_id"] << std::endl;
            std::cout << "State: " << data["state"] << std::endl;
            std::cout << "Role: " << data["role"] << std::endl;
            std::cout << "Amount: " << data["amount"] << std::endl;
            std::cout << "Min Trade Amount: " << data["min_trade_amount"] << std::endl;
            std::cout << "Taker Rating: " << data["taker_rating"] << std::endl;
            std::cout << "Creation Time: " << data["creation_timestamp"] << std::endl;
            std::cout << "Expiration Time: " << data["expiration_timestamp"] << std::endl;
            
            // Handle makers array
            if (data.contains("makers") && !data["makers"].empty()) {
                std::cout << "\nMakers:" << std::endl;
                for (const auto& maker : data["makers"]) {
                    std::cout << "  " << maker << std::endl;
                }
            }
        
            // Handle legs array
            if (data.contains("legs") && !data["legs"].empty()) {
                std::cout << "\nLegs:" << std::endl;
                for (const auto& leg : data["legs"]) {
                    std::cout << "  Instrument: " << leg["instrument_name"] << std::endl;
                    std::cout << "  Direction: " << leg["direction"] << std::endl;
                    std::cout << "  Ratio: " << leg["ratio"] << std::endl;
                }
            }
        
            // Handle bids array
            if (data.contains("bids") && !data["bids"].empty()) {
                std::cout << "\nBids:" << std::endl;
                for (const auto& bid : data["bids"]) {
                    std::cout << "  Price: " << bid["price"] << std::endl;
                    std::cout << "  Amount: " << bid["amount"] << std::endl;
                    std::cout << "  Execution: " << bid["execution_instruction"] << std::endl;
                    std::cout << "  Last Update: " << bid["last_update_timestamp"] << std::endl;
                    if (bid.contains("makers")) {
                        std::cout << "  Makers: ";
                        for (const auto& maker : bid["makers"]) {
                            std::cout << maker << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        
            // Handle asks array (if present)
            if (data.contains("asks") && !data["asks"].empty()) {
                std::cout << "\nAsks:" << std::endl;
                for (const auto& ask : data["asks"]) {
                    std::cout << "  Price: " << ask["price"] << std::endl;
                    std::cout << "  Amount: " << ask["amount"] << std::endl;
                    std::cout << "  Execution: " << ask["execution_instruction"] << std::endl;
                    std::cout << "  Last Update: " << ask["last_update_timestamp"] << std::endl;
                    if (ask.contains("makers")) {
                        std::cout << "  Makers: ";
                        for (const auto& maker : ask["makers"]) {
                            std::cout << maker << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        
            std::cout << "Disclosed: " << (data["disclosed"].get<bool>() ? "Yes" : "No") << std::endl;
            
            if (data.contains("label")) {
                std::cout << "\nLabel: " << data["label"] << std::endl;
            }
            
            if (data.contains("combo_id") && !data["combo_id"].is_null()) {
                std::cout << "Combo ID: " << data["combo_id"] << std::endl;
            }
            
            std::cout << "=============================" << std::endl;
        }

        else if (channel.find("book.") != std::string::npos) {
            auto data = params["data"];
            std::cout << "\n=== Order Book Update for " << data["instrument_name"] << " ===" << std::endl;
            std::cout << "Type: " << data["type"] << std::endl;
            std::cout << "Timestamp: " << data["timestamp"] << std::endl;
            std::cout << "Change ID: " << data["change_id"] << std::endl;
            
            if (data.contains("prev_change_id")) {
                std::cout << "Previous Change ID: " << data["prev_change_id"] << std::endl;
            }

            // Print bids
            if (data.contains("bids") && !data["bids"].empty()) {
                std::cout << "\nBids:" << std::endl;
                for (const auto& bid : data["bids"]) {
                    std::cout << "  Action: " << std::setw(6) << std::left << bid[0] 
                            << " Price: " << std::setw(10) << bid[1] 
                            << " Amount: " << bid[2] << std::endl;
                }
            }

            // Print asks
            if (data.contains("asks") && !data["asks"].empty()) {
                std::cout << "\nAsks:" << std::endl;
                for (const auto& ask : data["asks"]) {
                    std::cout << "  Action: " << std::setw(6) << std::left << ask[0] 
                            << " Price: " << std::setw(10) << ask[1] 
                            << " Amount: " << ask[2] << std::endl;
                }
            }
            
            std::cout << "=============================" << std::endl;
        } 

        else if (channel.find("instrument.state.") != std::string::npos) {
            auto data = params["data"];
            std::cout << "\n=== Instrument State Update ===" << std::endl;
            std::cout << "Instrument: " << data["instrument_name"] << std::endl;
            std::cout << "State: " << data["state"] << std::endl;
            std::cout << "Timestamp: " << data["timestamp"] << std::endl;
            std::cout << "=============================" << std::endl;
        }
        else if (channel.find("deribit_price_index.") != std::string::npos) {
            auto data = params["data"];
            std::cout << "\n=== Deribit Price Index Update ===" << std::endl;
            std::cout << "Index Name: " << data["index_name"] << std::endl;
            std::cout << "Price: " << std::fixed << std::setprecision(2) 
                     << data["price"] << " USD" << std::endl;
            std::cout << "Timestamp: " << data["timestamp"] << std::endl;
            std::cout << "=============================" << std::endl;
        }



        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling subscription message: " << e.what() << std::endl;
    }
}