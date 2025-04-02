# Deribit Trade Execution System

This is a high-performance trading system designed for the Deribit cryptocurrency exchange, implemented in C++ to achieve low-latency execution. It supports trading across spot, futures, and options markets, covering all Deribit-supported symbols. The system integrates with Deribit’s WebSocket API for real-time market data and order management.

## Key Features
- **Order Management**: Place, cancel, and modify orders with detailed feedback.
- **Market Data**: Fetch order books, view positions, and list open orders.
- **Real-Time Streaming**: Subscribe to live updates (e.g., order book changes, user trades) via WebSocket.
- **Performance Optimized**: Low-latency design with latency benchmarking (order placement, market data processing, end-to-end loop).
- **Robust Design**: Secure TLS connection, comprehensive error handling, and logging.

## Tech Stack
- **Language**: C++  
- **Libraries**: WebSocket++ (WebSocket client), nlohmann/json (JSON parsing)  
- **Dependencies**: ASIO for networking, OpenSSL for TLS  

## Deliverables
- Complete source code with inline documentation
- Video demo showcasing functionality and code review

Check out the [video demo](#) for a walkthrough, and feel free to explore the code or contribute!
