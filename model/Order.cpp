#include "Order.h"
#include <map>
#include <set>
#include <stdexcept>
#include <string>

std::string statusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASE:   return "RELEASE";
    case OrderStatus::REJECTED:  return "REJECTED";
    default:                     return "UNKNOWN";
    }
}

OrderStatus statusFromString(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    throw std::invalid_argument("Unknown order status: " + s);
}

void Order::transitionTo(OrderStatus next) {
    static const std::map<OrderStatus, std::set<OrderStatus>> allowed = {
        { OrderStatus::RESERVED,  { OrderStatus::CONFIRMED, OrderStatus::PRODUCING, OrderStatus::REJECTED } },
        { OrderStatus::PRODUCING, { OrderStatus::CONFIRMED } },
        { OrderStatus::CONFIRMED, { OrderStatus::RELEASE   } },
        { OrderStatus::RELEASE,   { } },
        { OrderStatus::REJECTED,  { } },
    };
    if (allowed.at(status_).count(next) == 0)
        throw std::logic_error(
            "illegal state transition: " + statusToString(status_) +
            " -> " + statusToString(next));
    status_ = next;
}

void Order::approve(bool hasEnoughStock) {
    transitionTo(hasEnoughStock ? OrderStatus::CONFIRMED : OrderStatus::PRODUCING);
}

void Order::reject() {
    transitionTo(OrderStatus::REJECTED);
}

void Order::completeProduction() {
    transitionTo(OrderStatus::CONFIRMED);
}

void Order::release() {
    transitionTo(OrderStatus::RELEASE);
}
