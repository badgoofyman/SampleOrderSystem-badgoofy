#pragma once
#include "../model/Order.h"
#include "../model/Sample.h"
#include <iosfwd>
#include <vector>
#include <string>
#include <ctime>

class OrderView {
public:
    // --- 주문 접수 ([2]) ---
    static std::string inputOrderSampleId(std::istream& in, std::ostream& out);
    static std::string inputCustomerName(std::istream& in, std::ostream& out);
    static int         inputQuantity(std::istream& in, std::ostream& out);
    static void        printInvalidSampleId(std::ostream& out);
    static void        printInvalidCustomerName(std::ostream& out);
    static void        printInvalidQuantity(std::ostream& out);
    static void        printOrderConfirm(const Sample& s, const std::string& customer,
                                         int qty, std::ostream& out);
    static bool        askYesNo(std::istream& in, std::ostream& out);
    static void        printOrderCancelled(std::ostream& out);
    static void        printOrderPlaced(const Order& o, std::ostream& out);

    // --- 주문 승인/거절 ([3]) ---
    static void printOrderListNumbered(const std::vector<Order>& orders,
                                       std::ostream& out);
    static int  selectOrderIndex(int count, std::istream& in, std::ostream& out);
    static void printStockCheckSufficient(const Sample& s, int qty,
                                          std::ostream& out);
    static void printStockCheckInsufficient(const Sample& s, int qty, int available,
                                             int shortage, int prodQty, int totalMin,
                                             std::ostream& out);
    static void printApprovalResult(const Order& o, std::ostream& out);
    static void printRejectionResult(const Order& o, std::ostream& out);
    static void printNoReservedOrders(std::ostream& out);

    // --- 출고 처리 ([6]) ---
    static void printOrderList(const std::vector<Order>& orders, std::ostream& out);
    static void printReleaseResult(const Order& o, time_t releasedAt,
                                   std::ostream& out);
    static void printNoConfirmedOrders(std::ostream& out);
};
