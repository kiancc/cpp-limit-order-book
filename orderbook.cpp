#include "orderbook.hpp"

void OrderBook::match(Order& inc, bool is_bid) {
    if (is_bid) {
        // incoming bid matches against asks (ascending map)
        auto& book = asks;
        auto it = book.begin();
        while (inc.qty > 0 && it != book.end() && it->first <= inc.price) {
            auto& level = it->second;
            while (inc.qty > 0 && !level.empty()) {
                Order& resting = level.front();
                int trade_qty = std::min(inc.qty, resting.qty);
                
                // Record the trade
                trades.emplace_back(inc.id, resting.id, it->first, trade_qty);
                
                inc.qty -= trade_qty;
                resting.qty -= trade_qty;
                
                if (resting.qty == 0) {
                    order_index.erase(resting.id);
                    level.pop_front();  // O(1) with deque
                }
            }
            if (level.empty()) it = book.erase(it);
            else ++it;
        }
    } else {
        // incoming ask matches against bids (descending map)
        auto& book = bids;
        auto it = book.begin();
        while (inc.qty > 0 && it != book.end() && it->first >= inc.price) {
            auto& level = it->second;
            while (inc.qty > 0 && !level.empty()) {
                Order& resting = level.front();
                int trade_qty = std::min(inc.qty, resting.qty);
                
                // Record the trade
                trades.emplace_back(resting.id, inc.id, it->first, trade_qty);
                
                inc.qty -= trade_qty;
                resting.qty -= trade_qty;
                
                if (resting.qty == 0) {
                    order_index.erase(resting.id);
                    level.pop_front();  // O(1) with deque
                }
            }
            if (level.empty()) it = book.erase(it);
            else ++it;
        }
    }
}

uint64_t OrderBook::add_limit(double price, int qty, bool is_bid) {
    if (qty <= 0) return 0;
    Order inc(next_id++, price, qty);
    uint64_t order_id = inc.id;
    
    // Attempt to match first; if any quantity remains, insert into the book
    match(inc, is_bid);
    
    if (inc.qty > 0) {
        if (is_bid) {
            bids[inc.price].push_back(inc);
        } else {
            asks[inc.price].push_back(inc);
        }
        // Add to index for O(1) cancellation
        order_index[order_id] = {price, is_bid};
    }
    return order_id;
}

bool OrderBook::cancel(uint64_t id) {
    // O(1) lookup using order_index
    auto it = order_index.find(id);
    if (it == order_index.end()) return false;
    
    auto [price, is_bid] = it->second;
    
    if (is_bid) {
        auto level_it = bids.find(price);
        if (level_it != bids.end()) {
            auto& level = level_it->second;
            for (auto order_it = level.begin(); order_it != level.end(); ++order_it) {
                if (order_it->id == id) {
                    level.erase(order_it);
                    if (level.empty()) bids.erase(level_it);
                    order_index.erase(it);
                    return true;
                }
            }
        }
    } else {
        auto level_it = asks.find(price);
        if (level_it != asks.end()) {
            auto& level = level_it->second;
            for (auto order_it = level.begin(); order_it != level.end(); ++order_it) {
                if (order_it->id == id) {
                    level.erase(order_it);
                    if (level.empty()) asks.erase(level_it);
                    order_index.erase(it);
                    return true;
                }
            }
        }
    }
    order_index.erase(it);
    return false;
}

uint64_t OrderBook::add_market(int qty, bool is_bid) {
    if (qty <= 0) return 0;
    Order inc(next_id++, is_bid ? 1e9 : 0.0, qty);  // Extreme price to match any available
    match(inc, is_bid);
    return inc.id;
}

void OrderBook::print_top() const {
    if (!bids.empty()) {
        auto& top = bids.begin()->second;
        int total_qty = 0;
        for (const auto& order : top) total_qty += order.qty;
        std::cout << "Best Bid: " << bids.begin()->first << " x " << total_qty << std::endl;
    }
    if (!asks.empty()) {
        auto& top = asks.begin()->second;
        int total_qty = 0;
        for (const auto& order : top) total_qty += order.qty;
        std::cout << "Best Ask: " << asks.begin()->first << " x " << total_qty << std::endl;
    }
}

void OrderBook::print_trades() const {
    std::cout << "\n=== Trades ===" << std::endl;
    for (const auto& trade : trades) {
        std::cout << "Trade: Buyer #" << trade.buyer_id 
                  << " <- Seller #" << trade.seller_id
                  << " @ " << trade.price << " x " << trade.qty << std::endl;
    }
    std::cout << "Total trades: " << trades.size() << "\n" << std::endl;
}

void OrderBook::clear() {
    bids.clear();
    asks.clear();
    order_index.clear();
    trades.clear();
}

size_t OrderBook::total_orders() const {
    return order_index.size();
}

void OrderBook::benchmark(int n) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        add_limit(100.0 + (i % 10) * 0.1, 100, i % 2 == 0);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inserted " << n << " orders in " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 
              << " ms\n";
}