#pragma once
#include "../model/Order.h"
#include <iosfwd>
#include <vector>
#include <string>

class OrderView {
public:
    static Order       inputNewOrder(std::istream& in, std::ostream& out);
    static void        printOrderPlaced(const Order& o, std::ostream& out);
    static void        printReservedList(const std::vector<Order>& orders, std::ostream& out);
    static std::string selectOrderNo(std::istream& in, std::ostream& out);
    static bool        askApprove(std::istream& in, std::ostream& out);
    static void        printApproved(const Order& o, std::ostream& out);
    static void        printRejected(const Order& o, std::ostream& out);
    static void        printInvalidOrderNo(std::ostream& out);
    static void        printNoReservedOrders(std::ostream& out);
};
