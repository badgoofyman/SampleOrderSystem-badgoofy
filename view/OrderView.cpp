#include "OrderView.h"
#include "../util/ConsoleColor.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>

// ──────────────────────────────────────────────────────
// 주문 접수 ([2])
// ──────────────────────────────────────────────────────

std::string OrderView::inputOrderSampleId(std::istream& in, std::ostream& out) {
    out << "  시료 ID: ";
    std::string line;
    std::getline(in, line);
    return line;
}

std::string OrderView::inputCustomerName(std::istream& in, std::ostream& out) {
    out << "  고객명: ";
    std::string line;
    std::getline(in, line);
    return line;
}

int OrderView::inputQuantity(std::istream& in, std::ostream& out) {
    out << "  주문 수량: ";
    std::string line;
    std::getline(in, line);
    try { return std::stoi(line); } catch (...) { return 0; }
}

void OrderView::printInvalidSampleId(std::ostream& out) {
    ConsoleColor::println(out, "  ※ 해당 시료 ID가 없습니다. 다시 입력해주세요.", ConsoleColor::YELLOW);
}

void OrderView::printOrderConfirm(const Sample& s, const std::string& customer,
                                  int qty, std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out, "  ┌──────────────────────────────────────┐", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  │           주문 확인                  │", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  └──────────────────────────────────────┘", ConsoleColor::BLUE);
    out << "  시료:   [" << s.id << "] " << s.name << "\n";
    out << "  고객명: " << customer << "\n";
    out << "  수량:   ";
    ConsoleColor::print(out, std::to_string(qty), ConsoleColor::YELLOW);
    out << " ea\n";
    out << "\n  ";
    ConsoleColor::print(out, "[Y]", ConsoleColor::GREEN);
    out << " 예약 접수   ";
    ConsoleColor::print(out, "[N]", ConsoleColor::RED);
    out << " 취소\n  선택: ";
}

bool OrderView::askYesNo(std::istream& in, std::ostream& out) {
    std::string line;
    std::getline(in, line);
    return (line == "Y" || line == "y");
}

void OrderView::printOrderCancelled(std::ostream& out) {
    ConsoleColor::println(out, "  주문이 취소되었습니다.", ConsoleColor::YELLOW);
}

void OrderView::printOrderPlaced(const Order& o, std::ostream& out) {
    ConsoleColor::print(out, "\n  ✔ 주문 접수 완료!\n", ConsoleColor::GREEN);
    out << "  주문번호: " << o.orderNo << "\n"
        << "  시료 ID:  " << o.sampleId << "\n"
        << "  고객명:   " << o.customerName << "\n"
        << "  수량:     " << o.quantity << " ea\n"
        << "  상태:     ";
    ConsoleColor::println(out, "RESERVED", ConsoleColor::BLUE);
}

// ──────────────────────────────────────────────────────
// 주문 승인/거절 ([3])
// ──────────────────────────────────────────────────────

void OrderView::printOrderListNumbered(const std::vector<Order>& orders, std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out,
        "  ┌────┬──────────────────────┬──────────┬───────────────┬──────┐",
        ConsoleColor::BLUE);
    ConsoleColor::print(out,
        "  │ No │ 주문번호              │ 시료 ID  │ 고객명        │ 수량 │",
        ConsoleColor::BLUE);
    out << "\n";
    ConsoleColor::println(out,
        "  ├────┼──────────────────────┼──────────┼───────────────┼──────┤",
        ConsoleColor::BLUE);
    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        out << "  │ "
            << std::right << std::setw(2) << (i + 1) << " │ "
            << std::left  << std::setw(21) << o.orderNo << "│ "
            << std::setw(9)  << o.sampleId << "│ "
            << std::setw(14) << o.customerName << "│ "
            << std::setw(4)  << o.quantity << " │\n";
    }
    ConsoleColor::println(out,
        "  └────┴──────────────────────┴──────────┴───────────────┴──────┘",
        ConsoleColor::BLUE);
}

int OrderView::selectOrderIndex(int count, std::istream& in, std::ostream& out) {
    while (true) {
        out << "  처리할 번호 (1~" << count << "): ";
        std::string line;
        if (!std::getline(in, line)) return -1;
        try {
            int idx = std::stoi(line);
            if (idx >= 1 && idx <= count) return idx - 1;
        } catch (...) {}
        ConsoleColor::println(out, "  ※ 잘못된 번호입니다.", ConsoleColor::YELLOW);
    }
}

void OrderView::printStockCheckSufficient(const Sample& s, int qty, std::ostream& out) {
    out << "\n  재고 확인: [" << s.id << "] 재고 ";
    ConsoleColor::print(out, std::to_string(s.stock), ConsoleColor::GREEN);
    out << "개 ≥ 주문 " << qty << "개  → ";
    ConsoleColor::println(out, "재고 충분", ConsoleColor::GREEN);
    out << "\n  ";
    ConsoleColor::print(out, "[Y]", ConsoleColor::GREEN);
    out << " 승인   ";
    ConsoleColor::print(out, "[N]", ConsoleColor::RED);
    out << " 거절\n  선택: ";
}

void OrderView::printStockCheckInsufficient(const Sample& s, int qty, int available,
                                             int shortage, int prodQty, int totalMin,
                                             std::ostream& out) {
    out << "\n  재고 확인: [" << s.id << "] 가용 재고 ";
    ConsoleColor::print(out, std::to_string(available), ConsoleColor::YELLOW);
    out << "개 < 주문 " << qty << "개  → ";
    ConsoleColor::println(out, "재고 부족 → 생산 필요", ConsoleColor::YELLOW);
    out << "  부족분:   " << shortage << "개\n"
        << "  실 생산량: ";
    ConsoleColor::print(out, std::to_string(prodQty), ConsoleColor::YELLOW);
    out << "개\n"
        << "  예상 시간: ";
    ConsoleColor::print(out, std::to_string(totalMin), ConsoleColor::YELLOW);
    out << " min\n\n  ";
    ConsoleColor::print(out, "[Y]", ConsoleColor::GREEN);
    out << " 승인(생산 예약)   ";
    ConsoleColor::print(out, "[N]", ConsoleColor::RED);
    out << " 거절\n  선택: ";
}

void OrderView::printApprovalResult(const Order& o, std::ostream& out) {
    out << "\n  ";
    if (o.getStatus() == OrderStatus::CONFIRMED) {
        ConsoleColor::print(out, "✔ 승인 완료 → ", ConsoleColor::GREEN);
        ConsoleColor::println(out, "CONFIRMED (출고 대기)", ConsoleColor::GREEN);
    } else {
        ConsoleColor::print(out, "✔ 승인 완료 → ", ConsoleColor::ORANGE);
        ConsoleColor::println(out, "PRODUCING (생산 중)", ConsoleColor::ORANGE);
    }
    out << "  주문번호: " << o.orderNo << "\n";
}

void OrderView::printRejectionResult(const Order& o, std::ostream& out) {
    out << "\n  ";
    ConsoleColor::print(out, "✘ 주문 거절: ", ConsoleColor::RED);
    out << o.orderNo << "\n";
}

void OrderView::printNoReservedOrders(std::ostream& out) {
    ConsoleColor::println(out, "  처리할 RESERVED 주문이 없습니다.", ConsoleColor::YELLOW);
}

// ──────────────────────────────────────────────────────
// 출고 처리 ([6])
// ──────────────────────────────────────────────────────

void OrderView::printOrderList(const std::vector<Order>& orders, std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out,
        "  ┌────┬──────────────────────┬──────────┬───────────────┬──────┐",
        ConsoleColor::BLUE);
    ConsoleColor::print(out,
        "  │ No │ 주문번호              │ 시료 ID  │ 고객명        │ 수량 │",
        ConsoleColor::BLUE);
    out << "\n";
    ConsoleColor::println(out,
        "  ├────┼──────────────────────┼──────────┼───────────────┼──────┤",
        ConsoleColor::BLUE);
    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        out << "  │ "
            << std::right << std::setw(2) << (i + 1) << " │ "
            << std::left  << std::setw(21) << o.orderNo << "│ "
            << std::setw(9)  << o.sampleId << "│ "
            << std::setw(14) << o.customerName << "│ "
            << std::setw(4)  << o.quantity << " │\n";
    }
    ConsoleColor::println(out,
        "  └────┴──────────────────────┴──────────┴───────────────┴──────┘",
        ConsoleColor::BLUE);
}

void OrderView::printReleaseResult(const Order& o, time_t releasedAt, std::ostream& out) {
    char buf[32] = {};
    struct tm tm_info;
#if defined(_WIN32)
    localtime_s(&tm_info, &releasedAt);
#else
    localtime_r(&releasedAt, &tm_info);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);

    out << "\n  ";
    ConsoleColor::print(out, "✔ 출고 처리 완료!\n", ConsoleColor::GREEN);
    out << "  주문번호: " << o.orderNo << "\n"
        << "  고객명:   " << o.customerName << "\n"
        << "  수량:     " << o.quantity << " ea\n"
        << "  출고 시각: " << buf << "\n"
        << "  상태:     ";
    ConsoleColor::println(out, "RELEASE", ConsoleColor::PURPLE);
}

void OrderView::printNoConfirmedOrders(std::ostream& out) {
    ConsoleColor::println(out, "  출고 대기 중인 주문이 없습니다.", ConsoleColor::YELLOW);
}
