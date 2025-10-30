#pragma once
#include <map>
#include <deque>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <iostream>

struct Order {
    uint64_t id;
    double price;
    int qty;
    std::chrono::nanoseconds ts;
    Order(uint64_t i, double p, int q) 
        : id(i), price(p), qty(q), 
          ts(std::chrono::high_resolution_clock::now().time_since_epoch()) {}
};

struct Trade {
    uint64_t buyer_id, seller_id;
    double price;
    int qty;
    std::chrono::nanoseconds ts;
    Trade(uint64_t b, uint64_t s, double p, int q)
        : buyer_id(b), seller_id(s), price(p), qty(q),
          ts(std::chrono::high_resolution_clock::now().time_since_epoch()) {}
};

class OrderBook {
    std::map<double, std::deque<Order>, std::greater<>> bids;  // price → [orders], descending
    std::map<double, std::deque<Order>> asks;                  // price → [orders], ascending
    std::unordered_map<uint64_t, std::pair<double, bool>> order_index;  // id → (price, is_bid)
    std::vector<Trade> trades;
    uint64_t next_id = 1;

    void match(Order& incoming, bool is_bid);

public:
    uint64_t add_limit(double price, int qty, bool is_bid);
    uint64_t add_market(int qty, bool is_bid);
    bool cancel(uint64_t id);
    void print_top() const;
    void print_trades() const;
    void clear();
    size_t total_orders() const;
    const std::vector<Trade>& get_trades() const { return trades; }
    void benchmark(int n_orders);
};