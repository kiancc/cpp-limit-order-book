#include "orderbook.hpp"
#include <cassert>
#include <iostream>

void test_basic_matching() {
    std::cout << "\n=== Test: Basic Matching ===" << std::endl;
    OrderBook ob;
    
    // Add resting ask at 100.5
    uint64_t ask_id = ob.add_limit(100.5, 50, false);
    assert(ob.total_orders() == 1);
    
    // Add bid that crosses - should match
    uint64_t bid_id = ob.add_limit(100.5, 30, true);
    
    // Should have 1 order left (partial fill of ask: 20 remaining)
    assert(ob.total_orders() == 1);
    
    // Should have 1 trade
    assert(ob.get_trades().size() == 1);
    const auto& trade = ob.get_trades()[0];
    assert(trade.price == 100.5);
    assert(trade.qty == 30);
    
    ob.print_trades();
    std::cout << "✓ Basic matching works\n";
}

void test_no_matching() {
    std::cout << "\n=== Test: No Matching ===" << std::endl;
    OrderBook ob;
    
    // Add bid and ask that don't cross
    ob.add_limit(100.0, 50, true);   // Bid at 100
    ob.add_limit(101.0, 50, false);  // Ask at 101
    
    assert(ob.total_orders() == 2);
    assert(ob.get_trades().size() == 0);
    
    ob.print_top();
    std::cout << "✓ No matching when spread exists\n";
}

void test_multiple_levels() {
    std::cout << "\n=== Test: Multiple Price Levels ===" << std::endl;
    OrderBook ob;
    
    // Build order book with multiple levels
    ob.add_limit(99.0, 100, true);   // Bid level 1
    ob.add_limit(99.5, 100, true);   // Bid level 2 (best bid)
    ob.add_limit(100.5, 100, false); // Ask level 1 (best ask)
    ob.add_limit(101.0, 100, false); // Ask level 2
    
    assert(ob.total_orders() == 4);
    ob.print_top();
    
    // Add aggressive bid that sweeps multiple levels
    ob.add_limit(101.0, 150, true);
    
    // Should match 100 @ 100.5 and 50 @ 101.0
    assert(ob.get_trades().size() == 2);
    assert(ob.total_orders() == 3); // 2 bids + 1 partial ask (50 remaining)
    
    ob.print_trades();
    std::cout << "✓ Multiple level matching works\n";
}

void test_fifo_priority() {
    std::cout << "\n=== Test: FIFO Priority ===" << std::endl;
    OrderBook ob;
    
    // Add multiple orders at same price level
    uint64_t ask1 = ob.add_limit(100.0, 50, false);
    uint64_t ask2 = ob.add_limit(100.0, 50, false);
    uint64_t ask3 = ob.add_limit(100.0, 50, false);
    
    assert(ob.total_orders() == 3);
    
    // Match with first order (FIFO)
    ob.add_limit(100.0, 50, true);
    
    assert(ob.total_orders() == 2);
    assert(ob.get_trades().size() == 1);
    
    // First trade should match the first order
    const auto& trade = ob.get_trades()[0];
    assert(trade.seller_id == ask1);
    
    std::cout << "✓ FIFO priority maintained\n";
}

void test_partial_fills() {
    std::cout << "\n=== Test: Partial Fills ===" << std::endl;
    OrderBook ob;
    
    // Large resting order
    ob.add_limit(100.0, 1000, false);
    
    // Multiple small orders that partially fill
    ob.add_limit(100.0, 100, true);
    ob.add_limit(100.0, 200, true);
    ob.add_limit(100.0, 300, true);
    
    assert(ob.get_trades().size() == 3);
    assert(ob.total_orders() == 1); // Resting ask should have 400 remaining
    
    ob.print_trades();
    std::cout << "✓ Partial fills work correctly\n";
}

void test_cancellation() {
    std::cout << "\n=== Test: Order Cancellation ===" << std::endl;
    OrderBook ob;
    
    // Add orders
    uint64_t bid1 = ob.add_limit(99.0, 100, true);
    uint64_t bid2 = ob.add_limit(99.5, 100, true);
    uint64_t ask1 = ob.add_limit(100.5, 100, false);
    
    assert(ob.total_orders() == 3);
    
    // Cancel middle order
    bool cancelled = ob.cancel(bid2);
    assert(cancelled);
    assert(ob.total_orders() == 2);
    
    // Try to cancel again (should fail)
    cancelled = ob.cancel(bid2);
    assert(!cancelled);
    
    // Cancel non-existent order
    cancelled = ob.cancel(999);
    assert(!cancelled);
    
    std::cout << "✓ O(1) cancellation works\n";
}

void test_market_orders() {
    std::cout << "\n=== Test: Market Orders ===" << std::endl;
    OrderBook ob;
    
    // Build book
    ob.add_limit(100.0, 100, false);
    ob.add_limit(100.5, 100, false);
    ob.add_limit(101.0, 100, false);
    
    // Market buy order - should sweep at any price
    ob.add_market(250, true);
    
    assert(ob.get_trades().size() == 3);
    assert(ob.total_orders() == 1); // 50 remaining at 101.0
    
    ob.print_trades();
    std::cout << "✓ Market orders execute immediately\n";
}

void test_edge_cases() {
    std::cout << "\n=== Test: Edge Cases ===" << std::endl;
    OrderBook ob;
    
    // Zero quantity
    uint64_t id = ob.add_limit(100.0, 0, true);
    assert(id == 0);
    assert(ob.total_orders() == 0);
    
    // Negative quantity
    id = ob.add_limit(100.0, -10, true);
    assert(id == 0);
    
    // Empty book operations
    ob.cancel(999);
    ob.print_top();
    
    std::cout << "✓ Edge cases handled\n";
}

void test_clear() {
    std::cout << "\n=== Test: Clear Book ===" << std::endl;
    OrderBook ob;
    
    ob.add_limit(100.0, 100, true);
    ob.add_limit(101.0, 100, false);
    ob.add_limit(100.5, 50, true); // Doesn't match (100.5 < 101.0)
    
    assert(ob.total_orders() == 3);
    assert(ob.get_trades().size() == 0);
    
    ob.clear();
    
    assert(ob.total_orders() == 0);
    assert(ob.get_trades().size() == 0);
    
    // Add orders after clear to verify it works
    ob.add_limit(100.0, 50, true);
    assert(ob.total_orders() == 1);
    
    std::cout << "✓ Clear works correctly\n";
}

void test_stress() {
    std::cout << "\n=== Test: Stress Test ===" << std::endl;
    OrderBook ob;
    
    const int N = 10000;
    
    // Add many orders
    for (int i = 0; i < N; ++i) {
        double price = 100.0 + (i % 100) * 0.01;
        ob.add_limit(price, 100, i % 2 == 0);
    }
    
    std::cout << "Added " << N << " orders, total in book: " << ob.total_orders() << std::endl;
    std::cout << "Total trades: " << ob.get_trades().size() << std::endl;
    
    // Cancel many orders
    int cancelled_count = 0;
    for (uint64_t i = 1; i <= N; i += 10) {
        if (ob.cancel(i)) cancelled_count++;
    }
    
    std::cout << "Cancelled " << cancelled_count << " orders" << std::endl;
    std::cout << "Remaining orders: " << ob.total_orders() << std::endl;
    
    std::cout << "✓ Stress test completed\n";
}

void test_benchmark() {
    std::cout << "\n=== Performance Benchmark ===" << std::endl;
    OrderBook ob;
    
    // Benchmark insertions
    ob.benchmark(100000);
    
    std::cout << "Active orders: " << ob.total_orders() << std::endl;
    std::cout << "Total trades: " << ob.get_trades().size() << std::endl;
}

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "  Limit Order Book Test Suite" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        test_basic_matching();
        test_no_matching();
        test_multiple_levels();
        test_fifo_priority();
        test_partial_fills();
        test_cancellation();
        test_market_orders();
        test_edge_cases();
        test_clear();
        test_stress();
        test_benchmark();
        
        std::cout << "\n=====================================" << std::endl;
        std::cout << "  ✓ All tests passed!" << std::endl;
        std::cout << "=====================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
