#include "OrderView.h"
#include <iostream>
#include <iomanip>
#include <string>

Order OrderView::inputNewOrder(std::istream& in, std::ostream& out) {
    Order o;
    out << "\n--- 시료 주문 ---\n";

    out << "시료 ID: ";
    std::getline(in, o.sampleId);

    out << "고객명: ";
    std::getline(in, o.customerName);

    out << "주문 수량: ";
    std::string line;
    std::getline(in, line);
    try { o.quantity = std::stoi(line); } catch (...) { o.quantity = 0; }

    return o;
}

void OrderView::printOrderPlaced(const Order& o, std::ostream& out) {
    out << "주문이 접수되었습니다.\n"
        << "  주문번호: " << o.orderNo << "\n"
        << "  시료 ID: "  << o.sampleId << "\n"
        << "  고객명: "   << o.customerName << "\n"
        << "  수량: "     << o.quantity << "\n";
}

void OrderView::printOrderList(const std::vector<Order>& orders, std::ostream& out) {
    out << "\n"
        << std::left
        << std::setw(22) << "주문번호"
        << std::setw(10) << "시료 ID"
        << std::setw(15) << "고객명"
        << std::setw(8)  << "수량\n"
        << std::string(55, '-') << "\n";
    for (const auto& o : orders) {
        out << std::left
            << std::setw(22) << o.orderNo
            << std::setw(10) << o.sampleId
            << std::setw(15) << o.customerName
            << std::setw(8)  << o.quantity << "\n";
    }
}

std::string OrderView::selectOrderNo(std::istream& in, std::ostream& out) {
    out << "처리할 주문번호: ";
    std::string line;
    std::getline(in, line);
    return line;
}

bool OrderView::askApprove(std::istream& in, std::ostream& out) {
    out << "승인(1) / 거절(0): ";
    std::string line;
    std::getline(in, line);
    return line == "1";
}

void OrderView::printApproved(const Order& o, std::ostream& out) {
    std::string statusStr = (o.getStatus() == OrderStatus::PRODUCING) ? "PRODUCING (생산 중)" : "CONFIRMED (출고 대기)";
    out << "주문 승인 완료. 상태: " << statusStr << "\n";
}

void OrderView::printRejected(const Order& o, std::ostream& out) {
    out << "주문 거절 완료. [" << o.orderNo << "]\n";
}

void OrderView::printInvalidOrderNo(std::ostream& out) {
    out << "해당 주문번호를 찾을 수 없습니다.\n";
}

void OrderView::printNoReservedOrders(std::ostream& out) {
    out << "처리할 RESERVED 주문이 없습니다.\n";
}
