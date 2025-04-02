# Deribit Trade Execution System Documentation

This document provides an overview of the Deribit Trade Execution System, a low-latency C++ application for trading on the Deribit exchange. It covers the code structure, key classes, functionality, and usage instructions.

---

## Code Structure

The project consists of three main source files:
- **`DeribitAuth.hpp` / `DeribitAuth.cpp`**: Handles WebSocket connection, authentication, and trading operations.
- **`DeribitSubscription.hpp` / `DeribitSubscription.cpp`**: Manages real-time market data subscriptions via WebSocket.
- **`main.cpp`**: Implements the command-line interface (CLI) and ties everything together.

### Dependencies
- **WebSocket++**: For WebSocket communication.
- **nlohmann/json**: For JSON parsing and serialization.
- **OpenSSL**: For TLS/SSL security.
- **ASIO**: For asynchronous I/O (included with WebSocket++).
- **pthread**: For threading support.

---

## Key Components

### 1. `DeribitAuth` Class
**File**: `DeribitAuth.hpp` / `DeribitAuth.cpp`  
**Purpose**: Manages connection to Deribit’s WebSocket API, authentication, and core trading operations.

#### Key Methods
- **`connect()`**: Establishes a WebSocket connection to `wss://test.deribit.com/ws/api/v2`.
  - Sets up TLS via `on_tls_init()` and runs the ASIO event loop in a detached thread.
- **`authenticate()`**: Sends a JSON-RPC authentication request using client credentials.
- **`placeBuyOrder(instrument_name, amount, type, label)`**: Places a market or limit buy order.
- **`cancelOrder(order_id)`**: Cancels an existing order by ID.
- **`editOrder(order_id, amount, price, advanced)`**: Modifies an order’s parameters.
- **`getOrderBook(instrument_name, depth)`**: Retrieves the order book for a given instrument.
- **`getPosition(instrument_name)`**: Fetches current position details.
- **`getOpenOrders()`**: Lists all open orders.

#### Event Handlers
- **`on_open()`**: Confirms connection establishment.
- **`on_message()`**: Parses incoming JSON responses (e.g., order confirmations, market data).
- **`on_close()` / `on_error()`**: Handles connection closure or errors.

#### Latency Tracking
- Uses `std::chrono` to measure order placement and trading loop latency, logged in `on_message()`.

---

### 2. `DeribitSubscription` Class
**File**: `DeribitSubscription.hpp` / `DeribitSubscription.cpp`  
**Purpose**: Manages WebSocket subscriptions for real-time market data.

#### Key Methods
- **`subscribePublic(channels)`**: Subscribes to public channels (e.g., `book.<instrument>`).
- **`subscribePrivate(channels)`**: Subscribes to private channels (e.g., `user.trades`), requires authentication.
- **`unsubscribe(channels)`**: Unsubscribes from specified channels.
- **`handleSubscriptionMessage(message)`**: Processes subscription updates (e.g., order book changes, trades).

#### Supported Channels
- **Public**: `announcements`, `trades.<kind>.<currency>`, `book.<instrument_name>`, etc.
- **Private**: `user.trades.<kind>.<currency>`, `block_rfq.maker.<currency>`, etc.

---

### 3. `main.cpp`
**Purpose**: Provides a CLI for interacting with the system.

#### Features
- **Commands**: `auth`, `buy`, `cancel`, `edit`, `orderbook`, `position`, `orders`, `subscribe`, `unsubscribe`, `help`, `exit`.
- **UI**: Color-coded output using ANSI escape codes for readability.
- **Supported Currencies**: BTC, ETH, AVAX, BNB, ADA, DOGE, PAXG, XRP, SOL.

#### Workflow
1. Initializes a `DeribitAuth` instance with user-provided credentials.
2. Processes commands in a loop, calling appropriate `DeribitAuth` or `DeribitSubscription` methods.
3. Displays responses and subscription updates in the console.

---

## Functionality

### Order Management
- **Place Order**: Sends a JSON-RPC `private/buy` request with instrument, amount, and type.
- **Cancel Order**: Issues a `private/cancel` request with an order ID.
- **Modify Order**: Uses `private/edit` to update amount, price, or advanced parameters.
- **View Orders**: Retrieves open orders via `private/get_open_orders`.

### Market Data
- **Order Book**: Fetches bids and asks with `public/get_order_book`.
- **Positions**: Gets position details (size, P/L) via `private/get_position`.

### Real-Time Streaming
- Subscribes to channels via `public/subscribe` or `private/subscribe`.
- Handles updates (e.g., trades, order book changes) in real time.

### Performance
- Measures latency for order placement and trading loops using `std::chrono::high_resolution_clock`.
- Logs results in microseconds (e.g., "Order Placement Latency: 250 µs").

---

## Usage

1. **Setup**: Follow the [README](#) to install dependencies and compile the code:
   ```bash
   g++ -std=c++11 -I/path/to/websocketpp -I/path/to/nlohmann_json DeribitAuth.cpp DeribitSubscription.cpp main.cpp -lssl -lcrypto -pthread -o deribit_auth