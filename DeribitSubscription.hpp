#pragma once

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class DeribitSubscription {
public:
    // Constructor takes references to websocket client and connection handle
    DeribitSubscription(
        websocketpp::client<websocketpp::config::asio_tls_client>& ws_client,
        websocketpp::connection_hdl& conn_hdl,
        bool& auth_status);

    // Subscription methods
    bool subscribePublic(const std::vector<std::string>& channels);
    bool subscribePrivate(const std::vector<std::string>& channels);
    bool unsubscribe(const std::vector<std::string>& channels);

    // Helper method to handle subscription responses
    void handleSubscriptionMessage(const json& message);

private:
    std::vector<std::string> active_subscriptions;
    bool handleSubscriptionResponse(const json& response);
    bool sendSubscriptionMessage(const json& msg);

    websocketpp::client<websocketpp::config::asio_tls_client>& ws_client;
    websocketpp::connection_hdl& connection_hdl;
    bool& authenticated;
};