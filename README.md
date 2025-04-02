---

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

## Build & Run  
### Compilation Steps  
Use the following command to compile the code:  

```bash
g++ -std=c++11 -I/path/to/websocketpp -I/path/to/nlohmann_json DeribitAuth.cpp DeribitSubscription.cpp main.cpp -lssl -lcrypto -pthread -o deribit_auth
```  

### Running the Program  
After compilation, execute the program using:  

```bash
./deribit_auth
```  

## Deliverables  
- Complete source code with inline documentation  
- Video demo showcasing functionality and code review  

Check out the [video demo](https://www.awesomescreenshot.com/video/38301410?key=6827d686b70f168673853627b992633c) for a walkthrough!  

---
