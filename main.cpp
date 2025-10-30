#include "orderbook.hpp"

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "  Order Book Demo" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    OrderBook ob;
    
    // Build initial book
    std::cout << "\n--- Building Order Book ---" << std::endl;
    uint64_t bid1 = ob.add_limit(99.5, 100, true);
    std::cout << "Added Bid #" << bid1 << ": $99.5 x 100" << std::endl;
    
    uint64_t bid2 = ob.add_limit(99.0, 150, true);
    std::cout << "Added Bid #" << bid2 << ": $99.0 x 150" << std::endl;
    
    uint64_t ask1 = ob.add_limit(100.5, 80, false);
    std::cout << "Added Ask #" << ask1 << ": $100.5 x 80" << std::endl;
    
    uint64_t ask2 = ob.add_limit(101.0, 120, false);
    std::cout << "Added Ask #" << ask2 << ": $101.0 x 120" << std::endl;
    
    std::cout << "\nCurrent Book State:" << std::endl;
    ob.print_top();
    std::cout << "Total active orders: " << ob.total_orders() << std::endl;
    
    // Aggressive order that crosses
    std::cout << "\n--- Aggressive Buy Order ---" << std::endl;
    uint64_t bid3 = ob.add_limit(100.7, 100, true);
    std::cout << "Added Bid #" << bid3 << ": $100.7 x 100 (crosses!)" << std::endl;
    
    ob.print_trades();
    ob.print_top();
    std::cout << "Total active orders: " << ob.total_orders() << std::endl;
    
    // Market order
    std::cout << "\n--- Market Sell Order ---" << std::endl;
    uint64_t market_id = ob.add_market(50, false);
    std::cout << "Market Sell #" << market_id << ": 50 shares" << std::endl;
    
    ob.print_trades();
    ob.print_top();
    
    // Cancel order
    std::cout << "\n--- Cancel Order ---" << std::endl;
    bool cancelled = ob.cancel(bid2);
    std::cout << "Cancelled Order #" << bid2 << ": " << (cancelled ? "Success" : "Failed") << std::endl;
    std::cout << "Total active orders: " << ob.total_orders() << std::endl;
    
    // Performance test
    std::cout << "\n--- Performance Benchmark ---" << std::endl;
    ob.clear();
    ob.benchmark(100000);
    std::cout << "Final active orders: " << ob.total_orders() << std::endl;
    std::cout << "Total trades executed: " << ob.get_trades().size() << std::endl;
    
    return 0;
}